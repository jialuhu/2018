/*************************************************************************
	> File Name: download.cpp
	> Author: 
	> Mail: 
	> Created Time: 三  3/ 6 20:05:00 2019
 ************************************************************************/

#include<iostream>
#include<unistd.h>
#include<fcntl.h>
#include<pthread.h>
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include<sys/stat.h>
#include<sys/time.h>
#include<arpa/inet.h>
#include<netdb.h>
#include<assert.h>
using namespace std;
struct file_imformation{
    char *file_path;//文件的绝对路径
    char *file_name;//文件解析出来的名称
    char *file_length;//文件的大小字节数目
};
class Baseclient{
private:
    int sockfd;//套接字
    int port;//端口号
    int thread_number;//开辟的线程数量
    char *address;//下载地址参数
    char *fqdn;//FQDN解析
    char http_request[100];//http请求头填写
    struct sockaddr_in server;//服务器套接字地址
    struct hostent *host;//通过解析下载地址，获取IP地址
    struct file_imformation *myfile_information;//文件信息
public:
    Baseclient(int thread_num, char *addr) : thread_number(thread_num), address(addr){
        sockfd = -1;
        port = 80;
        fqdn = NULL;
        memset(http_request, 0, 100);
        bzero(&server,sizeof(server));
        bzero(&host,sizeof(host));
        bzero(&myfile_information,sizeof(myfile_information));
        cout << address << "   " << thread_number << endl;
    };
    ~Baseclient();
    void parse_address();
    void mysocket();
};

Baseclient :: ~Baseclient()
{

}

void Baseclient :: mysocket()
{
    parse_address();
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = *(int *)host->h_addr_list[0]; 
    server.sin_port = htons(port);

    /*创建套接字*/
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    assert(sockfd>=0);
    /*创建连接*/
    int ret = connect(sockfd, (struct sockaddr*)&server, sizeof(server));
    assert(ret != -1);

    cout << "成功连接服务器!\n";
    
    /*填充HTTP请求头*/
    sprintf(http_request,"GET / %s HTTP/1.1\r\nHost: %s\r\nConnection: Close\r\n\r\n ",address, fqdn);
    

    
}

int main()
{
    int thread_number = 8;
    char address[100]="http//:st3.cdn.yestone.com/thumbs/2350753/image/15898/158987994/api_tumb_450.jpg";
    Baseclient myclient(thread_number, address);
    
}
