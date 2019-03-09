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
#include<stdlib.h>
using namespace std;
struct file_imformation{
    char *file_path;//文件的绝对路径
    char file_name[1000];//文件解析出来的名称
    long int file_length;//文件的大小字节数目
};
class Baseclient{
private:
    int sockfd;//套接字
    int port;//端口号
    int thread_number;//开辟的线程数量
    char *address;//下载地址参数
    char *fqdn;//FQDN解析
    char http_request[100];//http请求头填写
    char http_respond[1000];//http响应头接收
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
 //       cout << address << "   " << thread_number << endl;
    };
    ~Baseclient();
    STATUS parse_address();//解析下载地址
    void parse_httphead();//解析HTTP响应头
    void thread_download();//多线程下载
    void mysocket();
};

Baseclient :: ~Baseclient()
{
    close(sockfd);
    delete [] myfile_information.file_path;


}
/*解析用户输入的下载地址*/
Baseclient :: STATUS Baseclient :: parse_address()
{
    char *get;
    /*判断下载地址的状态*/
    if(strstr(address,"https") != NULL)
    {
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
    //cout << "fqdn: " << fqdn << endl;
    host = gethostbyname(fqdn); //通过名字获取hostIP地址
    
    /*获取文件的绝对路径*/
    int len = strlen(get)+2;   
    myfile_information.file_path = new char(len);
    sprintf(myfile_information.file_path, "/%s",get);
    myfile_information.file_path[len-1] = '\0';
  //  cout << myfile_information.file_path << endl;
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

/*发送HTTP请求头，接受HTTP响应头，对头部内容进行解析*/
void Baseclient :: parse_httphead()
{
    cout << "发送HTTP请求头：\n";
    cout << http_request << endl;
    cout << "接收HTTP响应头:\n";
    int ret = write(sockfd, http_request, strlen(http_request));
    if(ret <= 0)
    {
        cout << "wrong http_request\n";
        exit(0);
    }
    int k = 0;
    char ch[1];
    /*解析出HTTP响应头部分*/
    while(read(sockfd, ch, 1) != 0)
    {
        http_respond[k] = ch[0];
        if(k>4 && http_respond[k]=='\n' && http_respond[k-1]=='\r' && http_respond[k-2]=='\n' && http_respond[k-3]=='\r')
        {
            break;
        }
        k++;
    }
    int len = strlen(http_respond);
    http_respond[len] = '\0';
    cout << http_respond<< endl;
    
    /*解析出content-length:字段*/
    char *length;
    length = strstr(http_respond,"Content-Length:");
    if(length == NULL)
    {
        length = strstr(http_respond,"Content-length:");
        if(length == NULL)
        {
            length = strstr(http_respond, "content-Length:");
            if(length == NULL)
            {
                length = strstr(http_respond,"content-length:");
                if(length == NULL)
                {
                    cout << "NO Content-Length\n";
                    exit(0);
                }
            }
        }
    }
    char *get = strstr(length,"\r");
    *get = '\0';
    length = length + 16;;
    cout << length << endl;
    long int t = atol(length);
    myfile_information.file_length = t;
    cout << t << endl;
    int fd = open(myfile_information.file_name, O_CREAT | O_WRONLY, S_IRWXG | S_IRWXO | S_IRWXU);
    assert(fd >= 0);
    int index = 0;
    int mycount = 0;
    char buffer[1000];
    while(mycount<myfile_information.file_length && (index = read(sockfd,buffer,1000))>0)
    {
        mycount = mycount + index; 
        write(fd, buffer, index);
    }
    cout << "file_length::" << mycount<<endl;
    close(fd);
}
void Baseclient :: thread_download()
{

}
void Baseclient :: mysocket()
{
    parse_address();
    if(host == NULL)
    {
        status = HOST_WRONG;
    }
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
    sprintf(http_request,"GET %s HTTP/1.1\r\nHost: %s\r\nConnection: Close\r\n\r\n ",myfile_information.file_path ,fqdn);
    cout << "http_request:\n" << http_request << endl;
    /*分析收到的HTTP响应头*/
    parse_httphead();

    /*根据线程数量进行下载文件*/
  //  thread_download();
    
}

int main()
{
    int thread_number = 8;
    char address[100]="http://pic1.sc.chinaz.com/files/pic/pic9/201903/zzpic16937.jpg";
    Baseclient myclient(thread_number, address);
   myclient.mysocket(); 
}
