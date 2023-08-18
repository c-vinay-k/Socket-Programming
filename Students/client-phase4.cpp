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
#include<stdio.h>
#include <dirent.h>
#include <ctime>
using namespace std;

int id, port, uniq_id, n_nb, n_f,n_nbi;

struct sockaddr_in myadr;
int rec_sock;

void server_thread1(int* nb_port)
{ // socket on which we receive //My information
    int n_nb_connected=0;
    if ((rec_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("Could not get a socket to receive on");
        exit(EXIT_FAILURE);
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
    char c[2000] = {0}; // for buffering
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
                    n_nb_connected++;
                    string_size = recv(j,c,sizeof(c),0);
                    //cout<<string_size<<endl;
                    //cout<<c<<endl;
                    char *ptr;
                    ptr = strtok(c, "\n");
                    string k = ptr;
                    cout<<k<<endl;
                    ptr = strtok(NULL, " ");
                    k = ptr;
                    if(k!= "0")continue;
                    ptr = strtok(NULL, " ");
                    while (ptr != NULL)
                    {
                        bool flag = false;
                        //cout << ptr  << endl;
                        //string w1 = ptr;
                        int w;
                        sscanf(ptr, "%d", &w);
                        for(int p = 0;p<n_nb;p++)
                        {
                            if(w == nb_port[p] || w == port){
                                flag = true;
                                break;
                            }
                            
                        }
                        ptr = strtok (NULL, " ");
                        if(flag)continue;
                        nb_port[n_nb] = w;
                        n_nb++;
                        //bool flag = false;
                        //for(int p=0;p<n_f;p++){
                        //    flag = false;
                        //    for(int q=0;q<g;q++){
                        //        if(f[p]==a[q]){
                        //            cout<<"Found "<< f[p] <<" at "<<l<<" with MD5 0 at depth 1"<<endl;
                        //            flag = true;
                        //            break;
                        //        }
                        //    }
                        //    //if(flag)continue;
                        //    //cout<<"Found "<< f[p] <<" at 0 with MD5 0 at depth 0"<<endl;
                        //}
                    }
                    //for(int i=0;i<n_nb;i++){
                    //    cout<<nb_port[i]<<endl;
                    //}
                    FD_CLR(j,&cs);
                    //close(rec_sock);
                }
            }
        }
        if (n_nb_connected==n_nbi) break;
    }
    return;
}

void client_thread1(int* nb_port)
{
    string msg = "Connected to "+to_string(id)+" with unique-ID "+to_string(uniq_id)+" on port "+to_string(port)+"\n";
    msg = msg + "0";
    for(int i=0;i<n_nbi;i++){
        msg = msg +" "+to_string(nb_port[i]);
    }
    int send_sock;
    struct sockaddr_in adr,adr1;
    int string_size;
    const char *char_array = msg.c_str();
    for (int i=0; i<n_nbi; i++)
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
//cout<<"Hello"<<endl;
        int x = connect(send_sock, (struct sockaddr *)&adr, sizeof(struct sockaddr));
        while (x == -1)
        {
            x = connect(send_sock, (struct sockaddr *)&adr, sizeof(struct sockaddr));
        }
//cout<<"Hello1"<<endl;
        if ((string_size = send(send_sock, char_array, msg.size(), 0)) == -1)
        {
            perror("Could not send");
            exit(EXIT_FAILURE);
        }
        close(send_sock);

    }
    return;
}

void client_thread2(int *nb_port, string* files, int n_files)
{
    string msg = to_string(id)+" " +to_string(uniq_id)+" ";
    for (int y=0; y<n_files; y++)
        msg = msg + " " + files[y];
    int send_sock;
    struct sockaddr_in adr;
    int string_size;
    const char *char_array = msg.c_str();
    for (int i = 0; i < n_nb; i++)
    {
        if ((send_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        {
            perror("Could not get a socket to send on");
            exit(EXIT_FAILURE);
        }

        adr.sin_family = AF_INET;
        adr.sin_port = htons(nb_port[i]);
        adr.sin_addr.s_addr = INADDR_ANY; // set it to my IP address
        bzero(&(adr.sin_zero), 8);        // remaining in the struct are set to zero
                                          // cout<<"Hello"<<endl;
        int x = connect(send_sock, (struct sockaddr *)&adr, sizeof(struct sockaddr));
        while (x == -1)
        {
            x = connect(send_sock, (struct sockaddr *)&adr, sizeof(struct sockaddr));
        }
        // cout<<"Hello1"<<endl;
        if ((string_size = send(send_sock, char_array, msg.size(), 0)) == -1)
        {
            perror("Could not send");
            exit(EXIT_FAILURE);
        }
        close(send_sock);
    }
    return;
}

void server_thread2(string *f,int* nb_port,int* files_found, int* a, int* nb_id)
{
    //cout<<"MUMBAI"<<endl;
    //int rec_sock;             // socket on which we receive
    //struct sockaddr_in myadr; // My information
    //if ((rec_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    //{
    //    perror("Could not get a socket to receive on");
    //    exit(EXIT_FAILURE);
    //}
    //myadr.sin_family = AF_INET;
    //myadr.sin_port = htons(port);
    //myadr.sin_addr.s_addr = INADDR_ANY; // set it to my IP address
    //bzero(&(myadr.sin_zero), 8);        // remaining in the struct are set to zero
    //if (bind(rec_sock, (struct sockaddr *)&myadr, sizeof(struct sockaddr)) == -1)
    //{
    //    perror("Could not bind");
    //    exit(EXIT_FAILURE);
    //}
    //if (listen(rec_sock, 10) == -1)
    //{
    //    perror("Could not listen");
    //    exit(EXIT_FAILURE);
    //}
    int n_nb_connected = 0;
    fd_set cs, rs; // current sockets and ready sockets
    struct sockaddr_in adr;
    int len_of_adr = sizeof(adr);
    int string_size;
    FD_ZERO(&cs);
    FD_SET(rec_sock, &cs);
    int i = 0;
    while (true)
    {
        i++;
        rs = cs;
        if (select(FD_SETSIZE, &rs, NULL, NULL, NULL) < 0)
        {
            perror("An error occurred during receiving");
            exit(EXIT_FAILURE);
        }
//
        for (int j = 0; j < FD_SETSIZE; j++)
        {
            if (FD_ISSET(j, &rs))
            {
                if (j == rec_sock)
                {
                    int client_sock;
                    if ((client_sock = accept(rec_sock, (struct sockaddr *)&adr, (socklen_t *)&len_of_adr)) < 0)
                    {
                        perror("Could not accept");
                        exit(EXIT_FAILURE);
                    }
                    // cout<<"Hello4"<<endl;
                    FD_SET(client_sock, &cs);
                }
                else
                {
                    char c[2000] = {}; // for buffering
                    string_size = recv(j, c, sizeof(c), 0);
                    n_nb_connected++;
                    char *ptr;
                    string port1;
                    int l1=0,client_id,client_uniq_id,depth=2;
                    ptr = strtok(c, " ");
                    while (ptr != NULL)
                    {
                        //cout << ptr  << endl;
                        string w = ptr;
                        //cout<<w<<endl;
                        if(l1==1){
                            client_uniq_id = stoi(w);
                            //cout<<l<<endl;
                        }
                        else if(l1==0){
                            client_id = stoi(w);
                            for(int h=0;h<n_nbi;h++){
                                if(client_id == nb_id[h]){
                                    depth = 1;
                                    break;
                                }
                            }
                        }
                        else{
                        for(int p = 0;p<n_f;p++)
                        {
                            if(w == f[p]){
                                //cout<<"Found "<< w <<" at "<<l<<" with MD5 0 at depth "<<depth<<endl;
                                if(a[2*p+1]==0 || a[2*p+1]>client_uniq_id){
                                        a[2*p]=client_id;
                                        a[2*p+1] = client_uniq_id;
                                        files_found[p]=depth;
                                }
                            }
                        }
                        }
                        //bool flag = false;
                        //for(int p=0;p<n_f;p++){
                        //    flag = false;
                        //    for(int q=0;q<g;q++){
                        //        if(f[p]==a[q]){
                        //            cout<<"Found "<< f[p] <<" at "<<l<<" with MD5 0 at depth 1"<<endl;
                        //            flag = true;
                        //            break;
                        //        }
                        //    }
                        //    //if(flag)continue;
                        //    //cout<<"Found "<< f[p] <<" at 0 with MD5 0 at depth 0"<<endl;
                        //}
                        ptr = strtok (NULL, " ");
                        
                        l1++;
                    }
                    FD_CLR(j,&cs);
                }
            }
        }
        if (n_nb_connected==n_nb)
            break;
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
        while ((diread = readdir(dir)) != nullptr)
        {
            if (diread->d_name[0] != '.')
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
        while ((diread = readdir(dir)) != nullptr)
        {
            if (diread->d_name[0] != '.')
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
        else if (l == 2) {
            n_nb = stoi(line);
            n_nbi = n_nb;
        }
        else if (l == 4) n_f = stoi(line);
    }
    configfile.close();

    if(n_nb<1){
        if(n_f<1)return 0;
        ifstream configfile1(config_file);
        l = 0;
        while (getline(configfile1, line)){
            l++;
            if(l>4 && l < 5 + n_f){
                string s = string(line).substr(0,line.size()-1);
                if(line[line.size()-1] != '\r')s=s+line[line.size()-1];
                cout<<"Found "<<s<<" at 0 with MD5 0 at depth 0"<<endl;
            }
        }
        return 0;
    }
    int nb_id[n_nb], nb_port[100];
    int files_found[n_nb]={0};
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
    for(int i=0;i<n_files;i++){
        cout<<files[i]<<endl;
    }
////////////////////////////////////// Reading input from text file is done //////////////

    std::thread server1(server_thread1,&nb_port[0]);
    //cout<<"Hi"<<endl;
    std::thread client1(client_thread1,&nb_port[0]);
    //server.join();
    client1.join();
    server1.join();
    int a[n_f*2]={0};
    std::thread client2(client_thread2,&nb_port[0],&files[0],n_files);
    std::thread server2(server_thread2,&f[0],&nb_port[0],&files_found[0],&a[0],&nb_id[0]);
    client2.join();
    server2.join();
    for (int i=0; i<n_f;i++)
    {
            cout<<"Found "<<f[i]<<" at "<<a[2*i+1]<<" with MD5 0 at depth "<<files_found[i]<<endl;
    }
    return 0;
}
