#include <fstream>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <bits/stdc++.h>
#include <thread>
#include <dirent.h>
using namespace std;

void print_sorted(string* s, int n)
{
    string temp;
    for(int i=0;i<n;i++)
    {
        for(int j=i+1;j<n;j++)
        {
            if(s[i]>s[j])
            {
                temp=s[i];
                s[i]=s[j];
                s[j]=temp;
            }
        }
    }
    for(int i=0;i<n;i++)
    {
        cout<<s[i]<<endl;
    }
    return;
}

int id, port, uniq_id, n_nb, n_f;

void server_thread(string* output1)
{
    int n_nb_connected = 0;
    int rec_sock; // socket on which we receive
    struct sockaddr_in myadr; //My information
    if ((rec_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("Could not get a socket to receive on");
        exit(EXIT_FAILURE);
    }
    int yes=1;
    // lose the pesky "Address already in use" error message
    if (setsockopt(rec_sock,SOL_SOCKET,SO_REUSEADDR | SO_REUSEPORT,&yes,sizeof(int)) == -1) {
        perror("setsockopt");
        exit(1);
    }
    myadr.sin_family = AF_INET;
    myadr.sin_port = htons(port);
    myadr.sin_addr.s_addr = INADDR_ANY; // set it to my IP address
    bzero(&(myadr.sin_zero),8); // remaining in the struct are set to zero
    if (bind(rec_sock, (struct sockaddr *)&myadr, sizeof(struct sockaddr)) == -1)
    {
        perror("Could not bind");
        exit(EXIT_FAILURE);
    }
    if (listen(rec_sock, 10) == -1)
    {
        perror("Could not listen");
        exit(EXIT_FAILURE);
    }
    char c[100] = {0}; // for buffering
    fd_set cs, rs; // current sockets and ready sockets
    struct sockaddr_in adr;
    int len_of_adr = sizeof(adr);
    int string_size;
    FD_ZERO(&cs);
    FD_SET(rec_sock,&cs);
    int i = 0;
    while(true)
    {
        i++;
        rs = cs;
        if (select(FD_SETSIZE,&rs,NULL,NULL,NULL) < 0)
        {
            perror("An error occurred during receiving");
            exit(EXIT_FAILURE);
        }

        for (int j=0; j<FD_SETSIZE; j++)
        {
            if (FD_ISSET(j,&rs))
            {
                if (j == rec_sock)
                {
                    int client_sock;
                    //cout<<"Hello3"<<endl;
                    if ((client_sock = accept(rec_sock, (struct sockaddr *)&adr, (socklen_t *)&len_of_adr)) < 0)
                    {
                        perror("Could not accept");
                        exit(EXIT_FAILURE);
                    }
                    //cout<<"Hello4"<<endl;
                    FD_SET(client_sock, &cs);
                }
                else
                {
                    string_size = recv(j,c,sizeof(c),0);
                    output1[n_nb_connected] = c;
                    n_nb_connected++;
                    FD_CLR(j,&cs);
                }
            }
        }
        if (n_nb_connected == n_nb) break;
    }
    return;
}

void client_thread(int* nb_port)
{
    string msg = "Connected to "+to_string(id)+" with unique-ID "+to_string(uniq_id)+" on port "+to_string(port);
    int send_sock;
    struct sockaddr_in adr;
    int string_size;
    const char *char_array = msg.c_str();
    // int a;
    // cin>>a;
    for (int i=0; i<n_nb; i++)
    {
        if ((send_sock = socket(AF_INET,SOCK_STREAM,0)) == -1)
        {
            perror("Could not get a socket to send on");
            exit(EXIT_FAILURE);
        }

        adr.sin_family = AF_INET;
        adr.sin_port = htons(nb_port[i]);
        adr.sin_addr.s_addr = INADDR_ANY; // set it to my IP address
        bzero(&(adr.sin_zero),8); // remaining in the struct are set to zero
        int x = connect(send_sock, (struct sockaddr *)&adr, sizeof(struct sockaddr));
        while (x == -1)
        {
            x = connect(send_sock, (struct sockaddr *)&adr, sizeof(struct sockaddr));
        }
        if ((string_size = send(send_sock, char_array, msg.size(), 0)) == -1)
        {
            perror("Could not send");
            exit(EXIT_FAILURE);
        }
        close(send_sock);
    }
    return;
}

int main(int argc, char** argv)
{
    // incase of wrong no of arguments
    if (argc != 3)
    {
        if (argc > 3) cout<<"Only ";
        cout<<"2 command-line arguments required - config file path and directory path"<<endl;
        return 0;
    }
    // save the paths in arguments to string variables
    string config_file, dir_path;
    config_file = argv[1];
    dir_path = argv[2];
    
    DIR *dir;
    struct dirent *diread;
    int n_files = 0;
    if ((dir = opendir(&dir_path[0])) != nullptr)
    {
        string temp;
        while ((diread = readdir(dir)) != nullptr)
        {
            temp = diread->d_name;
            if (temp[0] != '.' && temp.compare("Downloaded") != 0)
            {
                n_files++;
            }
        }
        closedir(dir);
    }
    else
    {
        perror("opendir");
        return EXIT_FAILURE;
    }
    string* files = new string[n_files];
    int i=0;
    if ((dir = opendir(&dir_path[0])) != nullptr)
    {
        string temp;
        while ((diread = readdir(dir)) != nullptr)
        {
            temp = diread->d_name;
            if (temp[0] != '.' && temp.compare("Downloaded") != 0)
            {
                files[i] = diread->d_name;
                i++;
            }
        }
        closedir(dir);
    }
    else
    {
        perror("opendir");
        return EXIT_FAILURE;
    }

    string line; // to store each line recursively
    int l = 0; // line number
    ifstream configfile(config_file);
    while (getline(configfile,line))
    {
        l++;
        if (l == 1)
        {
            int i = 0, j = 0;
            for (int y=0;y<line.size();y++)
            {
                if (line[y] == ' ')
                {
                    if (i == 0) i = y;
                    else j = y;
                }
            }
            id = stoi(line.substr(0,i));
            port = stoi(line.substr(i+1,j-i-1));
            uniq_id = stoi(line.substr(j+1,line.size()-j-1));
        }
        else if (l == 2) n_nb = stoi(line);
        else if (l == 4) n_f = stoi(line);
    }
    configfile.close();
    if(n_nb<1)
    {
        print_sorted(files,n_files);
        return 0;
    }
    int nb_id[n_nb], nb_port[n_nb];
    string* f = new string[n_f];
    l = 0;
    ifstream configfile1(config_file);
    while (getline(configfile1,line))
    {
        l++;
        if (l == 3)
        {
            int s[2*n_nb]; s[0] = -1; int x = 1;
            for (int y=0;y<line.size();y++)
            {
                if (line[y] == ' ')
                {
                    s[x] = y; x++;
                }
            }
            int i = -1, j;
            for (int y=0;y<n_nb;y++)
            {
                nb_id[y] = stoi(line.substr(s[2*y]+1,s[2*y+1]-s[2*y]-1));
                nb_port[y] = stoi(line.substr(s[2*y+1]+1,s[2*y+2]-s[2*y+1]-1));
            }
        }
        else if (l > 4 && l < 5+n_f)
        {
            f[l-5] = string(line).substr(0,line.size()-1);
            if(line[line.size()-1] != '\r')f[l-5]=f[l-5]+line[line.size()-1];
        }
    }
    configfile1.close();
    print_sorted(files,n_files);
////////////////////////////////////// Reading input from text file is done //////////////
    string output1[n_nb];
    std::thread client(client_thread,&nb_port[0]);
    std::thread server(server_thread,&output1[0]);
    client.join();
    server.join();
    print_sorted(output1,n_nb);
    return 0;
}
