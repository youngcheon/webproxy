#include <stdio.h>
#include "csapp.h"
/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400
/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";
static const char *conn_hdr = "Connection: close\r\n"; 
static const char *prox_hdr = "Proxy-Connection: close\r\n";
static const char *host_hdr_format = "Host: %s\r\n";
static const char *requestlint_hdr_format = "GET %s HTTP/1.0\r\n";
static const char *endof_hdr = "\r\n";
static const char *connection_key = "Connection";
static const char *user_agent_key= "User-Agent";
static const char *proxy_connection_key = "Proxy-Connection";
static const char *host_key = "Host";
/*  트랜잭션 처리함수, GET Request이나 HEAD Request 들어오면 정적인지 동적인지 파악하여 각각의 함수를 실행 */
void doit(int fd);
void parse_uri(char *uri,char *hostname,char *path,int *port);
void build_http_header(char *endserver_http_header,char *hostname,char *path,int port,rio_t *client_rio);
int connect_endServer(char *hostname,int port);
/*
* argc: 메인 함수에 전달 되는 데이터의 수
* argv: 메인 함수에 전달 되는 실질적인 정보
*/
int main(int argc, char **argv) {
    /* 프로그램 실행 시 port를 안썼으면 */
    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(1);
    }
    int listenfd, connfd;                  /* listen 식별자, connfd 식별자 */
    char hostname[MAXLINE], port[MAXLINE]; /* hostname: 접속한 클라이언트 ip, port: 접속한 클라이언트 port */
    socklen_t clientlen;                   /* socklen_t 는 소켓 관련 매개 변수에 사용되는 것으로 길이 및 크기 값에 대한 정의를 내려준다 */
    struct sockaddr_storage clientaddr;    /* 어떤 타입의 소켓 구조체가 들어올지 모르기 때문에 충분히 큰 소켓 구조체로 선언 */
    /* listenfd: 이 포트에 대한 듣기 소켓 오픈 */
    listenfd = Open_listenfd(argv[1]);
    while (1) {
        clientlen = sizeof(clientaddr);                                                 /* 소켓 구조체 크기 */
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);                       /* 연결 요청 큐에 아무것도 없을 경우 기본적으로 연결이 생길때까지 호출자를 막아둠, 즉 대기 상태 */
        Getnameinfo((SA *)&clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE, 0); /* clientaddr: SA 구조체로 형변환, 소켓 정보를 가져옴 */
        printf("Accepted connection from (%s, %s)\n", hostname, port);                  /* 어떤 주소와 포트 번호를 가진 client가 들어왔는지 print */
        doit(connfd);                                                                   /* 트랜잭션 수행 */
        /* 연결이 끝났다고 print 하고 식별자(파일 디스크립트)를 닫아줌 */
        printf("CLOSE");
        Close(connfd);
    }
    return 0;
}
void doit(int fd) {
    int end_serverfd;
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE]; /* buf: request 헤더 정보 담을 공간*/
    char endserver_http_header[MAXLINE];                                /* 서버에 보낼 헤더 정보를 담을 공간 */
    char hostname[MAXLINE], path[MAXLINE];                              /* hostname: IP담을 공간, path: 경로 담을 공간 */
    rio_t rio, server_rio; //클라이언트 리오, 엔드서버 리오                                              /* rio: client rio 구조체, server_rio: proxy의 rio 구조체 */
    int port;                                                           /* port 담을 변수 */
    Rio_readinitb(&rio, fd);                                            /* rio 구조체 초기화 */
    Rio_readlineb(&rio, buf, MAXLINE);                                  /* buf에 fd에서 읽을 값이 담김 */
    sscanf(buf, "%s %s %s", method, uri, version);                      /* sscanf는 첫 번째 매개 변수가 우리가 입력한 문자열, 두 번째 매개 변수 포맷, 나머지 매개 변수에 포맷에 맞게 데이터를 읽어서 인수들에 저장 */
    /* GET이나 HEAD가 아닐 때 error 메시지 출력 */
    if (strcasecmp(method, "GET") && strcasecmp(method, "HEAD")) {
        printf("501 : GET이나 HEAD만 처리 가능합니다.");
        return;
    }
    /* ip 정보와 경로와 port번호 세팅해줄 함수 */
    parse_uri(uri, hostname, path, &port);
    /* 서버로 보낼 http header 만드는 함수 */
    build_http_header(endserver_http_header, hostname, path, port, &rio); //헤더, 호스트이름, 경로, 포트, 읽기 버퍼
    /* 서버와 connect하는 함수 */
    // 엔드 서버에 연결하면서 입출력에 대해 준비된 소켓 식별자 리턴.
    // 서버 입장에서는 프록시가 클라이언트.
    end_serverfd = connect_endServer(hostname, port);
    /* 서버와 연결 실패 */
    if(end_serverfd < 0) {
        printf("연결 실패\n");
        return;
    }
    /* 
     * server_rio 초기화, 즉, proxy와 서버에 연결되어 있는 식별자로 초기화 함 
     * rio_t 타입의 읽기 버퍼와 식별자 fd 연결
     */
    Rio_readinitb(&server_rio, end_serverfd);
    /* proxy와 서버에 연결되어있는 식별자에 http header 작성 */
    Rio_writen(end_serverfd, endserver_http_header, strlen(endserver_http_header));//아까만든 http헤더를 fd에 write
    /* server_rio의 내부 버퍼에 담긴 내용을 buf에 담고 클라이언트와 proxy에 연결되어 있는 식별자에 buf에 담긴 내용을 씀 */
    size_t n;
    while((n = rio_readlineb(&server_rio, buf, MAXLINE)) != 0) {
        printf("%ld byte를 받았습니다.\n",n);
        Rio_writen(fd, buf, n);
    }
    /* end_serverfd 닫기 */
    printf("CLOSE");
    Close(end_serverfd);
}
/* ip 정보와 경로와 port번호 세팅해줄 함수 */
void parse_uri(char *uri, char *hostname, char *path, int *port) {
    *port = 80;                                     /* 클라이언트에서 포트값을 안넣었을 경우 */
    char* pos = strstr(uri,"//");                   /* "http://"가 있으면 //부터 return */
    pos = pos != NULL? pos+2:uri;                   /* "//~~~~" 이렇게 나오니깐 + 2해서 ip가 시작하는 위치로 포인터 이동, 없으면 그냥 uri */
    char *pos2 = strstr(pos, ":");                 /* ':'뒤에는 port랑 path가 있음 */
    /* port랑 path가 있으면??????*/
    if(pos2 != NULL) { //http가 적혀있음
        *pos2 = '\0';                               /* ':'를 '\0'으로 변경 */
        sscanf(pos, "%s", hostname);                /* pos는 현재 ip 시작하는 위치이기 때문에 그 위치에서 문자열 포맷으로 ip를 hostname에 담음 */
        sscanf(pos2+1, "%d%s", port, path);         /* pos2+1은 포트가 시작하는 위치이기 때문에 정수형 포멧으로 port를 담고 문자열 포맷으로 경로를 path에 담음 */
    }
    else { 
        pos2 = strstr(pos, "/");                    /* '/'를 찾아서 있으면 포인터 변경 */
        if(pos2 != NULL) { //path가 있다.
            *pos2 = '\0';                           /* 포트가 적혀있음 */
            sscanf(pos, "%s", hostname);            /* pos에서 문자열 포맷으로 ip를 hostname에 담음 */
            *pos2 = '/';                            /* 다시 pos2위치에 '/'를 붙혀서 path작성할 준비 */
            sscanf(pos2, "%s", path);               /* pos2에서 문자열 포맷으로 경로를 path에 담음 */
        }
        /* path가 없으면 */
        else {
            sscanf(pos, "%s", hostname);            /* pos에서 문자열 포맷으로 ip를 hostname에 담음 */
        }
    }
    return;
}
/* 서버에 요청할 헤더 만듬 */
void build_http_header(char *endserver_http_header, char *hostname, char *path, int port, rio_t *client_rio) {
    char buf[MAXLINE], request_hdr[MAXLINE], other_hdr[MAXLINE], host_hdr[MAXLINE];
    /* request_hdr에 요청 헤더 담음 HTTP/1.0로 변경 */
    sprintf(request_hdr,requestlint_hdr_format, path); //파싱을 했던 path 삽입
    /* 클라이언트가 보낸 요청 라인 읽기 */
    while(Rio_readlineb(client_rio, buf, MAXLINE) > 0) {
        /* 마지막 종료 지점을 만나면 break */
        if(!strcmp(buf, endof_hdr)) {
            break;
        }
        /* 대소문자 구문하지 않고 host_key 찾았으면 host_hdr값 세팅, 
        *  즉, "Host"찾는 거임 */
        if(!strncasecmp(buf, host_key, strlen(host_key))) {
            printf("%s\n", buf);
            strcpy(host_hdr, buf);
            continue;
        }
        /* Connection, Proxy-Connection, User-Agent를 제외한 요청 라인을 other_hdr에 담음 */
        if(strncasecmp(buf, connection_key, strlen(connection_key)) 
          && strncasecmp(buf, proxy_connection_key, strlen(proxy_connection_key)) 
          && strncasecmp(buf, user_agent_key, strlen(user_agent_key))) {
            strcat(other_hdr, buf); //other헤더에 저장
        }
    }
    /* 요청 라인을 다 읽었는데 Host가 없으면 현재 domain을 host_hdr에 담음 */
    if(!strlen(host_hdr)) {
        sprintf(host_hdr, host_hdr_format, hostname);
    }
    /* http_header에 담음 */
    sprintf(endserver_http_header,"%s%s%s%s%s%s%s",
            request_hdr,
            host_hdr,
            conn_hdr,
            prox_hdr,
            user_agent_hdr,
            other_hdr,
            endof_hdr);
    return;
}
/* 서버와 proxy와 연결 */
int connect_endServer(char *hostname, int port) {
    char portStr[100];
    sprintf(portStr, "%d", port);
    return Open_clientfd(hostname, portStr);
}