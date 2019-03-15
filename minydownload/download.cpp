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
    char file_name_td[1000];//建立.*td文件，判断是否为断点下载
    long int file_length;//文件的大小字节数目
};
struct thread_package{
    pthread_t pid;//线程号
    int sockfd;//sockfd
    int start;//文件下载起始位置
    int end;//文件下载结束位置
    char file_name[1000];//文件名称
    int read_ret;
    int write_ret;
};

class Baseclient{
private:
    int sockfd;//套接字
    int port;//端口号
    int thread_number;//开辟的线程数量
    char *address;//下载地址参数
    char *fqdn;//FQDN解析
    char http_request[1000];//http请求头填写
    char http_respond[1000];//http响应头接收
    struct sockaddr_in server;//服务器套接字地址
    struct hostent *host;//通过解析下载地址，获取IP地址
    struct thread_package Thread_package;//线程包
    struct file_imformation myfile_information;//文件信息
    enum STATUS{HTTP=0, HTTPS, HOST_WRONG};
    STATUS status;
public:
    Baseclient(int thread_num, char *addr) : thread_number(thread_num), address(addr){
        sockfd = -1;
        port = 80;//默认端口为80
        fqdn = NULL;
        status = HTTP;
        memset(http_request, 0, 1000);
        bzero(&server,sizeof(server));
        bzero(&Thread_package,sizeof(Thread_package));
       bzero(&host,sizeof(host));
       bzero(&myfile_information,sizeof(myfile_information));
    }
    ~Baseclient();
    STATUS parse_address();//解析下载地址
    void parse_httphead();//解析HTTP响应头
    void thread_download();//多线程下载
    void mysocket();
private:
    static void *work(void *arg);
public:
    int byte_count;

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
    myfile_information.file_path = new char[len];
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

    /*获取.*td文件名称*/
    len = strlen(myfile_information.file_name);
    for(int i=0; i<len; i++)
    {
        if(myfile_information.file_name[i]=='.')
        {
            myfile_information.file_name_td[i] = myfile_information.file_name[i];
            break;
        }
        myfile_information.file_name_td[i] = myfile_information.file_name[i];
    }
    sprintf(myfile_information.file_name_td, "%s*td",myfile_information.file_name_td);
    //cout << "myfile_information.file_name_td: " << myfile_information.file_name_td << endl;
    return HTTPS;
}

/*发送HTTP请求头，接收HTTP响应头，对头部内容进行解析*/
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
    myfile_information.file_length = atol(length);
    
    /*int fd = open(myfile_information.file_name, O_CREAT | O_WRONLY, S_IRWXG | S_IRWXO | S_IRWXU);
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
    close(fd);*/
}

void* Baseclient :: work(void *arg)
{
    int s,e;
    char *buffer;
    struct thread_package *my = (struct thread_package *)arg;
   // cout << "myfile_information.file_name: " << my->file_name<<endl;
    int len = (my->end) - (my->start);
    buffer = new char[len];
    //cout << "len:" << len <<endl;
    int fd = open(my->file_name, O_CREAT | O_WRONLY, S_IRWXG | S_IRWXO | S_IRWXU);
   // cout << my->myfile_information.file_name_td << endl;
    assert(fd > 0);
    off_t offset;
    if((offset = lseek(fd, my->start, SEEK_SET)) < 0)
    {
        cout << "lseek is wrong!\n";
    }
    int ave = len;
 //   int read_ret=0;
    int r_ret = 0;
    int w_ret = 0;
    // int write_ret = 0;
    s = clock();
    while((r_ret = read(my->sockfd, buffer, len))>0 && my->read_ret!=ave)
    {
        my->read_ret = my->read_ret + r_ret;
        len = ave - my->read_ret;
        w_ret = write(fd, buffer, r_ret);
        my->write_ret = my->write_ret + w_ret;
       // cout << "write buf:" << write_ret << endl;
    }
   // cout << "read_ret:" << read_ret << endl;
    if(r_ret < 0)
    {
        cout << "read is wrong!\n";
    }
    delete [] buffer;
    close(fd);
    e = clock();
    cout << "&&&&&&:" << e-s << endl;
    return 0;
}
void Baseclient :: thread_download()
{
    void *statu;
    int ave_bit;//线程平均字节数目
    ave_bit = myfile_information.file_length / thread_number;
    cout << "平均每个线程下载:" << ave_bit << endl;
    struct thread_package *Thread_package;
    Thread_package = new struct thread_package[thread_number];
    
    /*如果.*td文件不存在，则为一个新的下载*/
    if(access(myfile_information.file_name_td, F_OK) != 0)
    {
        int start = 0;
        pthread_t pid;
        int i = 0;
        /*多线程下载*/
        for(i=0; i<thread_number; i++)
        {
            Thread_package[i].read_ret = 0;
            Thread_package[i].write_ret = 0;
            Thread_package[i].sockfd = sockfd;
            Thread_package[i].start = start;
            start += start + ave_bit;
            Thread_package[i].end = start;
            //strcpy(Thread_package[i].file_name, myfile_information.file_name);
            strcpy(Thread_package[i].file_name, myfile_information.file_name_td);
           // cout << "start:" << Thread_package[i].start << " " << "end: " << Thread_package[i].end << endl;
        }
        //int start1;
        //start1 = clock();
        for(i=0; i<thread_number; i++)
        {
            pthread_create(&pid, NULL, work, &Thread_package[i]);
            pthread_join(pid, &statu);    
        }
            //pthread_join(pid, &statu);    

        /*统计.*td文件中的字数是否等于总字符数量*/
        int sum1 = 0;
        int sum2 = 0;
        for(auto i=0; i<thread_number; i++)
        {
            sum1 = sum1 + Thread_package[i].read_ret;
            sum2 = sum2 + Thread_package[i].write_ret;
        }
       // int end;
        if(sum1==sum2 && sum1==myfile_information.file_length)//标识着下载成功
        {

            rename(myfile_information.file_name_td, myfile_information.file_name);
         //   end = clock();
            cout << "下载完毕!\n";
        }
        //cout << "clock:" << end-start1 << endl;
        //cout << "sum: " << sum1 << endl;
        //cout << "sum: " << sum2 << endl;
        
    }
    
    /*如果.*td文件存在,则属于断点下载*/
    else{

    }

}
void Baseclient :: mysocket()
{
    char *add;
    int len = strlen(address);
    add = new char[len];
    strcpy(add, address);
    parse_address();
    if(host == NULL)
    {
        status = HOST_WRONG;
        cout << "host id wrong!\n";
        exit(0);
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
    cout << "add: " << add << endl;
    sprintf(http_request,"GET %s HTTP/1.1\r\nHost: %s\r\nConnection: Close\r\n\r\n ",add ,fqdn);
    delete [] add;
    cout << "http_request:\n" << http_request << endl;
    
    /*分析收到的HTTP响应头*/
    parse_httphead();

    /*根据线程数量进行下载文件*/
    thread_download();
    
}

int main(int argc, char const *argv[])
{
    int second1,second2;
    //struct tm *p;
    //time(&second);
    //p = localtime(&second);
    //second = mktime(p);
   // second1 = clock();
    //cout << "time:" << second1 << endl;
    int thread_number = 1;
    char address[100]="http://pic1.sc.chinaz.com/files/pic/pic9/201903/zzpic16937.jpg";
    //char ad[100]="http://img.sccnn.com/bimg/341/11247.jpg";
    char ad[100]="http://jy.sccnn.com/zb_users/upload/2019/02/remoteimage2_20190215144726_32002.jpeg";
    //strcpy(add, argv[1]);
    Baseclient myclient(thread_number, ad);
   myclient.mysocket(); 
    //second2 = clock();
   // cout << "time:" << second2 << endl;
    //cout << "result:" << second2-second1 << endl;
}
