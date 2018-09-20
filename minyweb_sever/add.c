/*************************************************************************
	> File Name: add.cpp
	> Author:jialuhu 
	> Mail: 
	> Created Time: Fri 14 Sep 2018 05:20:33 PM CST
 ************************************************************************/

#include<stdio.h>
#include<string.h>
int main(int argc, char *argv[])
{
    char re_head[1000];
    char message[1000];
    int ret;
    int a,b,result;
    ret = sscanf(argv[0],"a=%d&b=%d", &a, &b);
    //printf("a=%d\t b=%d\n",a,b);
    if(ret < 0 || ret != 2)
    {
        sprintf(message,"<html><body>\r\n");
        sprintf(message,"%s<p>failure</p>\r\n",message);
        sprintf(message,"%s</body></html>");

        sprintf(re_head,"HTTP/1.1 GET\r\n");
        sprintf(re_head,"%scontent-length: %d\r\n",re_head,strlen(message));
        sprintf(re_head,"%scontent-type: text/html\r\n",re_head);
        sprintf(re_head,"%sconection: close\r\n\r\n");
        /*错误提示消息*/
    }
    else{
        result = a+b;
        /*返回正确信息*/
        sprintf(message,"<html><body>\r\n");
        sprintf(message,"%s<p>%d + %d = %d</p><br>\r\n",message,a,b,result);
        sprintf(message,"%s<p>welcome to the word of jialuhu</p><br>\r\n",message);
        sprintf(message,"%s</body></html>\r\n",message);
        
        sprintf(re_head,"HTTP/1.1 200 ok\r\n");
        sprintf(re_head,"%sContent-length: %d\r\n",re_head,(int)strlen(message));
        sprintf(re_head,"%scontent-type: text/html\r\n\r\n",re_head);
       // sprintf(re_head,"%sconection: close\r\n\r\n");
    }
    printf("%s",re_head);
    printf("%s",message);
    fflush(stdout);
    return 0;
}
