/*************************************************************************
	> File Name: myhttp_coon.h
	> Author:jialuhu 
	> Mail: 
	> Created Time: Tue 11 Sep 2018 02:22:33 PM CST
 ************************************************************************/

#ifndef _MYHTTP_COON_H
#define _MYHTTP_COON_H
#include<iostream>
#include<stdio.h>
#include<string.h>
#include<sys/socket.h>
#include<errno.h>
#include<stdlib.h>
#include<unistd.h>
#include<assert.h>
#include<sys/sendfile.h>
#include<sys/epoll.h>
#include<sys/fcntl.h>
#include<sys/stat.h>
#include<sys/types.h>
using namespace std;
#define READ_BUF 2000
class http_coon{
public:
    enum HTTP_CODE{NO_REQUESTION, GET_REQUESTION, BAD_REQUESTION, FORBIDDEN_REQUESTION,FILE_REQUESTION,INTERNAL_ERROR,NOT_FOUND};
    enum CHECK_STATUS{HEAD,REQUESTION};
private:
    char requst_head_buf[1000];//响应头的填充
    char read_buf[READ_BUF];//客户端的http请求读取
    char filename[250];//文件总目录
    int file_size;//文件大小
    int check_index;//目前检测到的位置
    int read_buf_len;//读取缓冲区的大小
    char *method;//请求方法
    char *url;//文件名称
    char *version;//协议版本
    bool m_linger;//是否保持连接
    int m_http_count;//http长度
    char *m_host;
    char path_400[17];
    char path_403[23];
    char path_404[40];
    char message[1000];
    CHECK_STATUS status;
    bool m_flag;
public:
    int epfd;
    int client_fd;
    int read_count;

    http_coon();
    void init(int e_fd, int c_fd);
    ~http_coon();
    int myread();//读取请求
    bool mywrite();//响应发送
    void doit();//线程接口函数
    void close_coon();
private:
    HTTP_CODE analyse();//解析Http请求头的函数
    int jude_line(char *temp, int &check_index, int &read_buf_len);//该请求是否是完整的以行\r\n
    HTTP_CODE head_analyse(char *temp);
    HTTP_CODE requestion_analyse(char *temp);
    HTTP_CODE do_file();
    void modfd(int epfd, int sock, int ev);
    bool bad_respond();
    bool forbiden_respond();
    bool succeessful_respond();
    bool not_found_request();
   // void addfd(int e_fd, int client_fd, bool flag);
};

void http_coon::init(int e_fd, int c_fd)
{
    epfd = e_fd;
    client_fd = c_fd;
    read_count = 0;
}

http_coon::http_coon()
{
    
}

http_coon::~http_coon()
{

}
/*void addfd(int e_fd, int client_fd, bool flag)
{
    epoll_event ev;
    ev.data.fd = client_fd;
    ev.events = EPOLLIN | EPOLLENT|EPOLLRHUP;
    if(flag)
    {
        ev.events |= EPOLLONESHOT;
    }
    epoll_ctl(e_fd, EPOLL_CTL_ADD, client_fd, &ev);
} */
void http_coon::close_coon()
{
    epoll_ctl(epfd, EPOLL_CTL_DEL, client_fd, 0);
    close(client_fd);
    client_fd = -1;

}
void http_coon::modfd(int epfd, int client_fd, int ev)
{
    epoll_event event;
    event.data.fd = client_fd;
    event.events = ev | EPOLLET | EPOLLONESHOT | EPOLLRDHUP;
    epoll_ctl(epfd, EPOLL_CTL_MOD, client_fd, &event);
    
}
int http_coon::myread()
{
    bzero(&read_buf,sizeof(read_buf));
    while(true)
    {
        int ret = recv(client_fd, read_buf+read_count, READ_BUF-read_count, 0 );
        if(ret == -1)
        {
            if(errno == EAGAIN || errno == EWOULDBLOCK)//读取结束
            {
                break;
            }
            return 0;
        }
        else if(ret == 0)
        {
            return 0;
        }
        read_count = read_count + ret;
    }
    cout << read_buf << endl;
    return 1;
}

bool http_coon::succeessful_respond()
{
    m_flag = false;
    bzero(requst_head_buf,sizeof(requst_head_buf));
    sprintf(requst_head_buf,"HTTP/1.1 200 ok\r\nConnection: close\r\ncontent-length:%d\r\n\r\n",file_size);
}
bool http_coon::bad_respond()//400
{
    bzero(url, strlen(url));
    strcpy(path_400,"bad_respond.html");
    url = path_400;
    bzero(filename,sizeof(filename));
    sprintf(filename,"/home/jialuhu/linux_net/web_sever/%s",url);
    struct stat my_file;
    if(stat(filename,&my_file)<0)
    {
        cout << "草拟\n";
    }
    file_size = my_file.st_size;
    bzero(requst_head_buf,sizeof(requst_head_buf));
    sprintf(requst_head_buf,"HTTP/1.1 400 BAD_REQUESTION\r\nConnection: close\r\ncontent-length:%d\r\n\r\n",file_size);
}
bool http_coon::forbiden_respond()//403
{
    bzero(url, strlen(url));
    strcpy(path_403,"forbidden_request.html");
    url = path_403;
    bzero(filename,sizeof(filename));
    sprintf(filename,"/home/jialuhu/linux_net/web_sever/%s",url);
    struct stat my_file;
    if(stat(filename,&my_file)<0)
    {
        cout << "失败\n";
    }
    file_size = my_file.st_size;
    bzero(requst_head_buf,sizeof(requst_head_buf));
    sprintf(requst_head_buf,"HTTP/1.1 403 FORBIDDEN\r\nConnection: close\r\ncontent-length:%d\r\n\r\n",file_size);
}
bool http_coon::not_found_request()
{
    bzero(url, strlen(url));
    strcpy(path_404,"not_found_request.html");
    url = path_404;
    bzero(filename,sizeof(filename));
    sprintf(filename,"/home/jialuhu/linux_net/web_sever/%s",url);
    struct stat my_file;
    if(stat(filename,&my_file)<0)
    {
        cout << "草拟\n";
    }
    file_size = my_file.st_size;
    bzero(requst_head_buf,sizeof(requst_head_buf));
    sprintf(requst_head_buf,"HTTP/1.1 404 NOT_FOUND\r\nConnection: close\r\ncontent-length:%d\r\n\r\n",file_size);
}
void http_coon::doit()
{
    cout << "doit\n";
    int choice = analyse();//根据解析请求头的结果做选择
    cout << "choice:" << choice << endl;
    switch(choice)
    {
        case NO_REQUESTION:
        {
            cout << "NO_REQUESTION\n";
            /*改变epoll的属性*/
            modfd(epfd, client_fd, EPOLLIN);
            return;
        }
        case BAD_REQUESTION: //400
        {
            cout << "BAD_REQUESTION\n";
            bad_respond();
            modfd(epfd, client_fd, EPOLLOUT);
            break;
        }
        case FORBIDDEN_REQUESTION://403
        {
            cout << "forbiden_respond\n";
            forbiden_respond();
            modfd(epfd, client_fd, EPOLLOUT);
            break;
        }
        case NOT_FOUND://404
        {
            cout<<"not_found_request"<< endl;
            not_found_request();
            modfd(epfd, client_fd, EPOLLOUT);
            break;   
        }
        case FILE_REQUESTION://文件资源无问题
        {
            cout << "文件file request\n";
            succeessful_respond();
            modfd(epfd, client_fd, EPOLLOUT);
            break;
        }
        default:
        {
            close_coon();
    }

    }
}
/*判断一行是否读取完整*/
int http_coon::jude_line(char *temp, int &check_index, int &read_buf_len)
{
    char ch;
   // int len = strlen(temp);
    for( ; check_index<read_buf_len; check_index++)
    {
        if(check_index==26)
        {
            cout << temp[check_index] << endl;
        }
        ch = temp[check_index];
        if(ch == '\r' && check_index+1<read_buf_len && temp[check_index+1]=='\n')
        {
            temp[check_index++] = '\0';
            temp[check_index++] = '\0';
            return 1;//完整读入一行
        }
        if(ch == '\r' && check_index+1==read_buf_len)
        {
            return 0;
        }
        if(ch == '\n')
        {
            if(check_index>1 && temp[check_index-1]=='\r')
            {
                temp[check_index-1] = '\0';
                temp[check_index++] = '\0';
                return 1;
            }
            else{
                return 0;
            }
        }
    }
    return 0;
}
/*http请求解析*/
http_coon::HTTP_CODE http_coon::analyse()
{
    status = REQUESTION;
    int flag;
    char *temp = read_buf;
    int star_line = 0;
    check_index = 0;
    int star = 0;
    read_buf_len = strlen(read_buf);
    cout << "read_buf_len:" << read_buf_len << endl;
    while((flag=jude_line(read_buf, check_index, read_buf_len))==1)
    {
        cout << "star:"<< star << endl;
        temp = temp + star;
        cout << "temp:" << temp << endl;
        star = check_index;
        switch(status)
        {
            case REQUESTION://请求行分析，包括文件名称和请求方法
            {
                cout << "requestion\n";
                int ret;
                ret = requestion_analyse(temp);
                if(ret==BAD_REQUESTION)
                {
                    cout << "ret == BAD_REQUESTION\n";
                    //请求格式不正确
                    return BAD_REQUESTION;
                }
                break;
            }
            case HEAD://请求头的分析
            {
                int ret;
                ret = head_analyse(temp);
                if(ret==GET_REQUESTION)
                {
                    return do_file();//文件名判断函数
                }
                break;
            }
            default:
            {
                return INTERNAL_ERROR;
            }
        }
    }
    return NO_REQUESTION;//请求不完整，需要继续读入
}
/*解析请求行*/
http_coon::HTTP_CODE http_coon::requestion_analyse(char *temp)
{
    char *p = temp;
    cout << "p=" << p << endl;
    for(int i=0; i<2; i++)
    {
        if(i==0)
        {
            method = p;//请求方法保存
            int j = 0;
            while((*p != ' ') && (*p != '\r'))
            {
                p++;
            }
            p[0] = '\0';
            p++;
            cout << "method:" <<method << endl;
          //  method++;
        }
        if(i==1)
        {
            url = p;//文件路径保存
            while((*p != ' ') && (*p != '\r'))
            {
                p++;
            }
            p[0] = '\0';
            p++;
            cout << "url:" << url << endl;
        }
    }
    version = p;//请求协议保存
    while(*p != '\r')
    {
        p++;
    }
    p[0] = '\0';
    p++;
    p[0] = '\0';
    p++;
    cout << version << endl;
    if(strcmp(method,"GET")!=0)
    {
        return BAD_REQUESTION;
    }
    if(!url || url[0]!='/')
    {
        return BAD_REQUESTION;
    }
    if(strcmp(version,"HTTP/1.1")!=0)
    {
        return BAD_REQUESTION;
    }
    status = HEAD;//状态转移到解析头部
    return NO_REQUESTION;//继续解析
}

/*解析头部信息*/
http_coon::HTTP_CODE http_coon::head_analyse(char *temp)
{
    if(temp[0]=='\0')
    {
        //获得一个完整http请求
        return GET_REQUESTION;
    }
    //处理其他头部
    else if(strncasecmp(temp,"Connection:", 11) == 0)
    {
        temp = temp+11;
        while(*temp==' ')
        {
            temp++;
        }
        if(strcasecmp(temp, "keep-alive") == 0)
        {
            m_linger = true;
        }
    }
    else if(strncasecmp(temp,"Content-Length:", 15)==0)
    {
        temp = temp+15;
        while(*temp==' ')
        {
            temp++;
        }
        m_http_count = atol(temp);
    }
    else if(strncasecmp(temp,"Host:",5)==0)
    {
        temp = temp+5;
        while(*temp==' ')
        {
            temp++;
        }
        m_host = temp;
    }
    else{
        cout << "can't handle it's hand\n";
    }
    return NO_REQUESTION;
}
http_coon::HTTP_CODE http_coon::do_file()
{
   char path[40]="/home/jialuhu/linux_net/web_sever";
    strcpy(filename,path);
    strcat(filename,url);
    struct stat m_file_stat;
    if(stat(filename, &m_file_stat) < 0)
    {
        cout << "打不开\n";
        return NOT_FOUND;//NOT_FOUND 404
    }
    if( !(m_file_stat.st_mode & S_IROTH))//FORBIDDEN_REQUESTION 403
    {
        return FORBIDDEN_REQUESTION;
    }
    if(S_ISDIR(m_file_stat.st_mode))
    {
        return BAD_REQUESTION;//BAD_REQUESTION 400
    }
    //int fd = open(filename, O_RDONLY);
    file_size = m_file_stat.st_size;
    return FILE_REQUESTION;
}
bool http_coon::mywrite()
{
   /* if(m_flag)
    {
        int r;
        int fd;
        struct stat errno_file;
        if(stat("not_found_request.html",&errno_file)<0)
        {
            return false;
        }
        file_size = errno_file.st_size;
        fd = open("not_found_request.html",O_RDONLY);
        assert(fd != -1);
        r=send(client_fd,requst_head_buf,strlen(requst_head_buf),0);
        cout << "^^^^r=" << r << endl;
        if(r>0)
        {
            cout << "kjsljf\n";
            r = sendfile(client_fd, fd, NULL, file_size);
            cout << "sendfile-r:"<<r << endl;
            if(r<=0)
            {
                cout << "失败\n";
                return false;
            }
            return true;
        }
        return false;
    }*/
    cout << "filename:"<<filename << endl;
    int fd = open(filename,O_RDONLY);
    assert(fd != -1);
    int ret;
    ret = write(client_fd,requst_head_buf,strlen(requst_head_buf));
    cout << "client_fd:" << client_fd << " ret:" << ret << endl;
    if(ret < 0)
    {
        close(fd);
        return false;
    }
    ret = sendfile(client_fd, fd, NULL, file_size);
    cout << "ret:&&&" << ret << endl;
    if(ret < 0)
    {
        close(fd);
        return false;
    }
    close(fd);
    return true;
}
#endif
