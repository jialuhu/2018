/*************************************************************************
	> File Name: CGI_client.cpp
	> Author:jialuhu 
	> Mail: 
	> Created Time: Thu 02 Aug 2018 04:35:04 PM CST
 ************************************************************************/

/*#include<iostream>
#include<sys/socket.h>
#include<errno.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<assert.h>
#include<netinet/in.h>
#include<string.h>
const char* ip = "127.0.0.1";
const int port = 4507;
using namespace std;*/
#include"CGI.h"
class client{
private:
    int sock;
public:
    client();
    void create_client();
};

client::client()
{
    int sock = 0;
}

void client::create_client()
{
    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family=AF_INET;
    inet_pton(AF_INET, ip, &address.sin_addr);
    address.sin_port = htons(port);
    
    sock = socket(PF_INET, SOCK_STREAM, 0);
    assert(sock >= 0);

    if((connect(sock,(struct sockaddr*)&address, sizeof(address))) != -1)
    {
        char buf[30];
        cout << "connect successful!\n";
        if(recv(sock,buf,sizeof(buf),0) > 0)
        {
            cout << buf << endl;
        }
        else{
            cout << "the errno:" << errno << endl;
        }
    }
    else{
        cout << "the errno:" << errno <<endl;
    }
    close(sock);
}

int main()
{
    client CGI;
    CGI.create_client();
    return 0;
}

