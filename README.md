# 탐험준비 - Week07 웹서버 만들기
## 💡 나만의 웹서버를 만들어보기! (프록시 서버까지)

클라이언트의 request를 받고, response를 내어주는 웹서버를 만들어봅니다. ㄴ 웹서버는 어떤 기능들의 모음일까요?
‘컴퓨터 시스템’ 교재의 11장을 보면서 차근 차근 만들어주세요.(기본 코드는 모두 있습니다!)
웹 서버를 완성했으면 프록시(proxy) 서버 과제에 도전합니다.
http://csapp.cs.cmu.edu/3e/proxylab.pdf
출처: CMU (카네기멜론)
https://github.com/SWJungle/webproxy-jungle 의 내용대로 진행합니다.
# 진행방법
- 책에 있는 코드를 기반으로, tiny 웹서버를 완성하기 (tiny/tiny.c, tiny/cgi-bin/adder.c 완성)
- AWS 혹은 container 사용시 외부로 포트 여는 것을 잊지 말기
- 숙제 문제 풀기 (11.6c, 7, 9, 10, 11)
- 프록시 과제 도전 (proxy.c 완성)
# GOAL
- tiny 웹서버를 만들고 → 숙제문제 11.6c, 7, 9, 10, 11 중 세문제 이상 풀기 → 프록시 과제 도전


> 아래는 원본 README입니다

~~~
####################################################################
# CS:APP Proxy Lab
#
# Student Source Files
####################################################################

This directory contains the files you will need for the CS:APP Proxy
Lab.

proxy.c
csapp.h
csapp.c
    These are starter files.  csapp.c and csapp.h are described in
    your textbook. 

    You may make any changes you like to these files.  And you may
    create and handin any additional files you like.

    Please use `port-for-user.pl' or 'free-port.sh' to generate
    unique ports for your proxy or tiny server. 

Makefile
    This is the makefile that builds the proxy program.  Type "make"
    to build your solution, or "make clean" followed by "make" for a
    fresh build. 

    Type "make handin" to create the tarfile that you will be handing
    in. You can modify it any way you like. Your instructor will use your
    Makefile to build your proxy from source.

port-for-user.pl
    Generates a random port for a particular user
    usage: ./port-for-user.pl <userID>

free-port.sh
    Handy script that identifies an unused TCP port that you can use
    for your proxy or tiny. 
    usage: ./free-port.sh

driver.sh
    The autograder for Basic, Concurrency, and Cache.        
    usage: ./driver.sh

nop-server.py
     helper for the autograder.         

tiny
    Tiny Web server from the CS:APP text
~~~
