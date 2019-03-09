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
    char file_name[1000];//文件解析出来的名称
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
    struct file_imformation myfile_information;//文件信息
    enum STATUS{HTTP=0, HTTPS, HOST_WRONG};
    STATUS status;
public:
    Baseclient(int thread_num, char *addr) : thread_number(thread_num), address(addr){
        sockfd = -1;
        port = 80;
        fqdn = NULL;
        status = HTTP;
        memset(http_request, 0, 100);
        bzero(&server,sizeof(server));
       bzero(&host,sizeof(host));
       bzero(&myfile_information,sizeof(myfile_information));
        cout << address << "   " << thread_number << endl;
    };
    ~Baseclient();
    STATUS parse_address();//解析下载地址
    void parse_httphead();//解析HTTP响应头
    void thread_download();//多线程下载
    void mysocket();
};

Baseclient :: ~Baseclient()
{
    delete [] myfile_information.file_path;

}
/*解析用户输入的下载地址*/
Baseclient :: STATUS Baseclient :: parse_address()
{
    //int len = strlen(address);
    char *get;
    /*判断下载地址的状态*/
    if(strstr(address,"https") != NULL)
    {
        //status = HTTPS;
        return HTTPS;
    }
    if(strstr(address, "http") != NULL)
    {
        status = HTTP;
    }
    
    /*获取FQDN*/
    get = address + 7;
    fqdn = get;//获取FQDN的起始位置
    get = strstr(get, "/");//解析出FQDN地址
    *get++ = '\0';
    cout << "fqdn: " << fqdn << endl;
    cout << "get: " << get << endl;
    
    /*获取文件的绝对路径*/
    int len = strlen(get)+2;   
    myfile_information.file_path = new char(len);
    sprintf(myfile_information.file_path, "/%s",get);
    myfile_information.file_path[len-1] = '\0';
    cout << myfile_information.file_path << endl;
    len = strlen(myfile_information.file_path);
    
    /*获取文件原来的名称*/
    int i = len;
    for(i = len-1; i>=0; i--)
    {
        if(myfile_information.file_path[i] == '/')
        {
            get = myfile_information.file_path + i + 1;
            break;
        }
    }
    len = strlen(get);
    strcpy(myfile_information.file_name,get);
    myfile_information.file_name[strlen(get)] = '\0';
    return HTTPS;
}
void Baseclient :: parse_httphead()
{

}
void Baseclient :: thread_download()
{

}
void Baseclient :: mysocket()
{
    parse_address();
    host = gethostbyname(fqdn);
   /* if(host == NULL)
    {
        status = HOST_WRONG;
    }*/
    server.sin_family = AF_INET;
   // server.sin_addr.s_addr = *(int *)host->h_addr_list[0]; 
    //server.sin_port = htons(port);
    /*创建套接字*/
    //sockfd = socket(AF_INET, SOCK_STREAM, 0);
    //assert(sockfd>=0);
    /*创建连接*/
   // int ret = connect(sockfd, (struct sockaddr*)&server, sizeof(server));
   // assert(ret != -1);

    cout << "成功连接服务器!\n";

    /*填充HTTP请求头*/
  /*  sprintf(http_request,"GET / %s HTTP/1.1\r\nHost: %s\r\nConnection: Close\r\n\r\n ",address, fqdn);
   */
    /*分析收到的HTTP响应头*/
    /*parse_httphead();
*/
    /*根据线程数量进行下载文件*/
  //  thread_download();
    
}

int main()
{
    int thread_number = 8;
    char address[100]="http//:st3.cdn.yestone.com/thumbs/2350753/image/15898/158987994/api_tumb_450.jpg";
    Baseclient myclient(thread_number, address);
   myclient.mysocket(); 
}
