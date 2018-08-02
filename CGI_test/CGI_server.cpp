/*************************************************************************
	> File Name: CGI.cpp
	> Author:jialuhu 
	> Mail: 
	> Created Time: Thu 02 Aug 2018 04:05:21 PM CST
 ************************************************************************/

/*#include<iostream>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<assert.h>
#include<unistd.h>
#include<errno.h>
#include<string.h>
#include<stdio.h>
using namespace std;
const char *ip = "127.0.01";
const int port = 4507;*/
#include"CGI.h"
class server
{
private:
    int sock;
public:
    server();
    void create_socket();
};

server::server()
{
    int sock = 0;
}

void server::create_socket()
{
    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &address.sin_addr);
    address.sin_port = htons(port);
    
    sock = socket(PF_INET, SOCK_STREAM, 0);
    assert(sock >= 0);

    int ret = bind(sock,(struct sockaddr*)&address, sizeof(address));
    assert(ret != -1);

    ret = listen(sock,5);
    assert(ret != -1);

    struct sockaddr_in client;
    bzero(&client, sizeof(client));
    socklen_t client_addrlength = sizeof(client);
    int connfd = accept(sock, (struct sockaddr*)&client, &client_addrlength);
    if(connfd<0)
    {
        cout << "errno is :" << errno << endl; 
    }
    else{
        close(1);
        dup2(connfd,1);
        printf("abc hujialu\n");
        close(connfd);
    }
    close(sock);
}

int main()
{
    server CGI;
    CGI.create_socket();
    return 0;
}
