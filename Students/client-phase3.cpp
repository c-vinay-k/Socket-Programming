#include <fstream>
#include <string>
#include <sys/stat.h>
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
#include <iostream>
#include <filesystem>
#include <vector>
#include <dirent.h>
#include <openssl/md5.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
using namespace std;

int id, port, uniq_id, n_nb, n_f, n_files;
int rec_sock; // socket on which we receive
string dir_path;
int sendall(int s, char *buf, int *len)
{
    int total = 0;
    // how many bytes we’ve sent
    int bytesleft = *len; // how many we have left to send
    int n;
    while (total < *len)
    {
        n = send(s, buf + total, bytesleft, 0);
        if (n == -1)
        {
            break;
        }
        total += n;
        bytesleft -= n;
    }
    *len = total;            // return number actually sent here
    return n == -1 ? -1 : 0; // return -1 on failure, 0 on success
}

void server_thread1(string *f, int *a)
{
    int n_nb_connected = 0;
    struct sockaddr_in myadr; // My information
    if ((rec_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("Could not get a socket to receive on");
        exit(EXIT_FAILURE);
    }
    myadr.sin_family = AF_INET;
    myadr.sin_port = htons(port);
    myadr.sin_addr.s_addr = INADDR_ANY; // set it to my IP address
    bzero(&(myadr.sin_zero), 8);        // remaining in the struct are set to zero
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

        for (int j = 0; j < FD_SETSIZE; j++)
        {
            if (FD_ISSET(j, &rs))
            {
                if (j == rec_sock)
                {
                    int client_sock;
                    // cout<<"Hello3"<<endl;
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
                    n_nb_connected++;
                    char c[2000] = {}; // for buffering
                    string_size = recv(j, c, sizeof(c), 0);
                    // cout<<c<<endl;
                    char *ptr;
                    int client_id, client_uniq_id;
                    int l1 = 0;
                    ptr = strtok(c, "\n");
                    string w = ptr;
                    cout<<w<<endl;
                    ptr = strtok(NULL, " ");
                    while (ptr != NULL)
                    {
                        // cout << ptr  << endl;
                        string w = ptr;
                        if (l1 == 0)
                        {
                            client_id = stoi(w);
                        }
                        else if (l1 == 1)
                            client_uniq_id = stoi(w);
                        else
                        {
                            for (int p = 0; p < n_f; p++)
                            {
                                if (w == f[p])
                                {
                                    if (a[2 * p + 1] == 0 || a[2 * p + 1] > client_id)
                                    {
                                        a[2 * p] = client_id;
                                        a[2 * p + 1] = client_uniq_id;
                                    }
                                    // cout<<"Found "<< w <<" at "<<l<<" with MD5 0 at depth 1"<<endl;
                                }
                            }
                        }
                        ptr = strtok(NULL, " ");

                        l1++;
                    }
                    FD_CLR(j, &cs);
                }
            }
        }
        if (n_nb_connected == n_nb)
            break;
    }
    return;
}

void client_thread1(int *nb_port, string *files, int n_files)
{
    string msg = "Connected to "+to_string(id)+" with unique-ID "+to_string(uniq_id)+" on port "+to_string(port)+"\n";    
    msg = msg + to_string(id) + " " + to_string(uniq_id);
    for (int y = 0; y < n_files; y++)
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

void server_thread2(string *files, int *nb_id)
{
    int n_nb_connected = 0;
    // int rec_sock;             // socket on which we receive
    // struct sockaddr_in myadr; // My information
    // if ((rec_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    //{
    //     perror("Could not get a socket to receive on");
    //     exit(EXIT_FAILURE);
    // }
    // myadr.sin_family = AF_INET;
    // myadr.sin_port = htons(port);
    // myadr.sin_addr.s_addr = INADDR_ANY; // set it to my IP address
    // bzero(&(myadr.sin_zero), 8);        // remaining in the struct are set to zero
    // if (bind(rec_sock, (struct sockaddr *)&myadr, sizeof(struct sockaddr)) == -1)
    //{
    //     perror("Could not bind");
    //     exit(EXIT_FAILURE);
    // }
    // if (listen(rec_sock, 10) == -1)
    //{
    //     perror("Could not listen");
    //     exit(EXIT_FAILURE);
    // }
    fd_set cs, rs; // current sockets and ready sockets
    struct sockaddr_in adr;
    int len_of_adr = sizeof(adr);
    int string_size;
    FD_ZERO(&cs);
    FD_SET(rec_sock, &cs);
    int i = 0;
    while (true)
    {
        rs = cs;
        if (select(FD_SETSIZE, &rs, NULL, NULL, NULL) < 0)
        {
            perror("An error occurred during receiving");
            exit(EXIT_FAILURE);
        }

        for (int j = 0; j < FD_SETSIZE; j++)
        {
            if (FD_ISSET(j, &rs))
            {
                if (j == rec_sock)
                {
                    int client_sock;
                    // cout<<"Hello3"<<endl;
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
                    char c[10] = {}; // for buffering
                    string_size = recv(j, c, sizeof(c), 0);
                    string k = c;
                    cout<<k<<endl;
                    if (k != "x"){
                        // cout << ptr  << endl;
                        ifstream in_file(dir_path+ k, ios::binary);
                        in_file.seekg(0, ios::end);
                        int file_size = in_file.tellg();
                        in_file.seekg(0, ios::beg);
                        cout<<k<<" is filename and filesize is "<<file_size<<" "<<14<<endl;
                        //w = w + " " + to_string(file_size)+" 12";
                        string s = to_string(file_size);
                        int d = s.size();
                        for(int i=0;i<10-d;i++){
                            s = s + " ";
                        }
                        send(j, s.c_str(), 10, 0);
                        // FILE *fr;
                        // const char *o = (dir_path + k).c_str();
                        // fr = fopen(o, "rb");
                        char sendbuff[file_size];
                        in_file.read(sendbuff, file_size);
                        sendall(j, sendbuff, &file_size);
                    }
                    else{
                        n_nb_connected++;
                        cout<<n_nb_connected<<" neighbours done out of neighbours "<<n_nb<<endl;
                    }
                    FD_CLR(j, &cs);
                }
            }
        }
        if (n_nb_connected == n_nb)
            break;
    }
    cout<<"Secver thread ended"<<endl;
    return;
}

void client_thread2(string filename, int nport)
{
    int send_sock;
    struct sockaddr_in adr;
    int string_size;
        //cout<<"Start"<<endl;
        //cout<<nb_id[i]<<" "<<msg<<endl;
        //cout<<"Stop"<<endl;
        const char *char_array = filename.c_str();
        if ((send_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        {
            perror("Could not get a socket to send on");
            exit(EXIT_FAILURE);
        }

        adr.sin_family = AF_INET;
        adr.sin_port = htons(nport);
        adr.sin_addr.s_addr = INADDR_ANY; // set it to my IP address
        bzero(&(adr.sin_zero), 8);        // remaining in the struct are set to zero
                                          // cout<<"Hello"<<endl;
        int x = connect(send_sock, (struct sockaddr *)&adr, sizeof(struct sockaddr));
        while (x == -1)
        {
            x = connect(send_sock, (struct sockaddr *)&adr, sizeof(struct sockaddr));
        }
        // cout<<"Hello1"<<endl;
        if ((string_size = send(send_sock, char_array, filename.size(), 0)) == -1)
        {
            perror("Could not send");
            exit(EXIT_FAILURE);
        }
        // char recvbuff[2048];
        //
        // FILE *fw;
        // const char* o = (msg.substr(2,msg.length()-2)).c_str();
        // fw = fopen(o,"w");
        // while(1){
        // int count = recv(send_sock, recvbuff, 2048,0);
        //
        // if(!strcmp(recvbuff,"xyz"))
        // break;
        //
        // fwrite(recvbuff,2048, 1, fw);
        // memset(recvbuff,0,2048);
        //}
        // fclose(fw);
        if (filename != "x")
        {
            char c[10];
            int sz = recv(send_sock, c, sizeof(c), 0);
            cout<<sz<<" Received message: "<<c<<endl;
            int index=0;
            string filesize1 = "";
            while (true)
            {
                if (c[index] == ' ') break;
                filesize1 = filesize1 + c[index];
                index++;
            }
            cout<<filesize1<<" is size of "<<filename<<endl;
            int filesize = stoi(filesize1), recdsize = 0;
            char filedata[filesize];
            int x = 0;
            while (filesize > recdsize)
            {
                x= recv(send_sock, filedata + recdsize, filesize - recdsize, 0);
                recdsize = recdsize + x;
                if(x == 0)break;
                //cout<<"receiving\n";
            }
            mkdir((dir_path+"Downloaded").c_str(),0777);
            ofstream file(dir_path+"Downloaded/" + filename);
            if (file)
            {
                file.write(&filedata[0], filesize);
            }
            cout<<"Downloaded "<<filename<<endl;
        }
        close(send_sock);
    return;
}
void clientmaintainer(int *nb_port, int *nb_id, string *f, int *a)
{
    for (int i=0;i<n_f;i++)
    {
        if (a[2*i] == 0) continue;
        for (int j = 0; j < n_nb; j++)
        {
            if (a[2*i] == nb_id[j])
            {
                client_thread2(f[i],nb_port[j]);
            }
        }
    }
    for (int j=0;j<n_nb;j++)
    {
        client_thread2("x",nb_port[j]);
    }
    cout<<"Client thread over"<<endl;
    return;
}

void print_md5_sum(unsigned char* md) {
    int i;
    for(i=0; i <MD5_DIGEST_LENGTH; i++) {
            printf("%02x",md[i]);
    }
    return;
}

// Get the size of the file by its file descriptor
unsigned long get_size_by_fd(int fd) {
    struct stat statbuf;
    if(fstat(fd, &statbuf) < 0) exit(-1);
    return statbuf.st_size;
}

void md5(string filename){
    int file_descript;
    unsigned long file_size;
    void* file_buffer;
    const char* filepath = filename.c_str();
    unsigned char result[MD5_DIGEST_LENGTH];
    file_descript = open(filepath, O_RDONLY);
    if(file_descript < 0){
    exit(-1);
    }
    file_size = get_size_by_fd(file_descript);
    file_buffer = mmap(0, file_size, PROT_READ, MAP_SHARED, file_descript, 0);
    MD5((unsigned char*) file_buffer, file_size, result);
    munmap(file_buffer, file_size); 
    print_md5_sum(result);
    return;
}

int main(int argc, char **argv)
{
    // incase of wrong no of arguments
    if (argc != 3)
    {
        if (argc > 3)
            cout << "Only ";
        cout << "2 command-line arguments required - config file path and directory path" << endl;
        return 0;
    }
    // save the paths in arguments to string variables
    string config_file;
    config_file = argv[1];
    dir_path = argv[2];
    cout<<dir_path<<endl;
    DIR *dir;
    struct dirent *diread;
    n_files = 0;
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
    string *files = new string[n_files];
    int i = 0;
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
    int l = 0;   // line number
    ifstream configfile(config_file);
    while (getline(configfile, line))
    {
        l++;
        if (l == 1)
        {
            int i = 0, j = 0;
            for (int y = 0; y < line.size(); y++)
            {
                if (line[y] == ' ')
                {
                    if (i == 0)
                        i = y;
                    else
                        j = y;
                }
            }
            id = stoi(line.substr(0, i));
            port = stoi(line.substr(i + 1, j - i - 1));
            uniq_id = stoi(line.substr(j + 1, line.size() - j - 1));
        }
        else if (l == 2)
            n_nb = stoi(line);
        else if (l == 4)
            n_f = stoi(line);
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
    int nb_id[n_nb], nb_port[n_nb];
    string *f = new string[n_f];
    l = 0;
    ifstream configfile1(config_file);
    while (getline(configfile1, line))
    {
        l++;
        if (l == 3)
        {
            int s[2 * n_nb];
            s[0] = -1;
            int x = 1;
            for (int y = 0; y < line.size(); y++)
            {
                if (line[y] == ' ')
                {
                    s[x] = y;
                    x++;
                }
            }
            int i = -1, j;
            for (int y = 0; y < n_nb; y++)
            {
                nb_id[y] = stoi(line.substr(s[2 * y] + 1, s[2 * y + 1] - s[2 * y] - 1));
                nb_port[y] = stoi(line.substr(s[2 * y + 1] + 1, s[2 * y + 2] - s[2 * y + 1] - 1));
            }
        }
        else if (l > 4 && l < 5 + n_f)
        {
            f[l - 5] = string(line).substr(0, line.size() - 1);
            if (line[line.size() - 1] != '\r')
                f[l - 5] = f[l - 5] + line[line.size() - 1];
        }
    }
    configfile1.close();
    for(int i=0;i<n_files;i++){
        cout<<files[i]<<endl;
    }
    ////////////////////////////////////// Reading input from text file is done //////////////
    int a[n_f * 2] = {0};
    std::thread server1(server_thread1, &f[0], &a[0]);
    std::thread client1(client_thread1, &nb_port[0], &files[0], n_files);
    server1.join();
    client1.join();
    //for(int i=0;i<n_f;i++){
    //    cout<<a[2*i]<<" " <<f[i]<<endl;
    //}
    //cout<<"STEP 1 DONE";
    std::thread server2(server_thread2, &files[0], &nb_id[0]);
    std::thread client2(clientmaintainer, &nb_port[0], &nb_id[0], &f[0], &a[0]);
    client2.join();
    server2.join();
    cout<<"FINISHED\n";
    for(int i=0;i<n_f;i++){
        if(a[2*i+1]<1){
            cout<<"Found "<<f[i]<<" at 0 with MD5 0 at depth 0"<<endl;
        }
        else{
            cout<<"Found "<<f[i]<<" at "<<a[2*i+1]<<" with MD5 ";
            md5(dir_path+"Downloaded/"+f[i]);
            cout<<" at depth 1"<<endl;
        }
    }
    return 0;
}
