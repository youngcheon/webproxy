/*
 * adder.c - a minimal CGI program that adds two numbers together
 */
/* $begin adder */
#include "../csapp.h"

int main(void)
{
    char *buf, *p;
    char arg1[MAXLINE], arg2[MAXLINE], content[MAXLINE];
    int n1=0, n2=0;

    // 두개 뽑아내기
    if ((buf = getenv("QUERY_STRING")) != NULL){
        // buf에서 & 인덱스 찾기
        p = strchr(buf, '&');
        *p = '\0';// & 위치를 NULL로 채워줘서 문자열 자르기(?)
        strcpy(arg1,buf);
        strcpy(arg2,p+1);
        // char to int
        n1 = atoi(arg1);
        n2 = atoi(arg2);
    }
    //response body만들기
    sprintf(content, "QUERY_STRING=%s", buf);
    sprintf(content, "Welcome to add.com: ");
    sprintf(content, "%sTHE Internet addition portal.\r\n<p>", content);
    sprintf(content, "%sThe answer is : %d + %d = %d\r\n<p>",
            content, n1,n2,n1+n2);
    sprintf(content, "%sThanks for visiting!\r\n", content);
    
    // HTTP response 만들기
    printf("Connection : close\r\n");
    printf("Content-length : %d\r\n", (int)strlen(content));
    printf("Content-type: text/html\r\n\r\n");
    printf("%s", content);
    fflush(stdout);
    exit(0);
}
/* $end adder */
