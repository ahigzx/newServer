#include <iostream>
#include <cassert>
#include <sys/epoll.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <cstdio>
#include <cstdlib>
#include <string.h>
#include <errno.h>
using namespace std;

int setNoBlock(int fd)
{
    int oldOption = fcntl(fd, F_GETFL);
    int newOption = oldOption | O_NONBLOCK;
    fcntl(fd, F_SETFL, newOption);
    return oldOption;
}

const int bufferSize = 100;

int unblockConnect(const char* ip, int port, int time)
{
    int res = 0;
    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_port = htons(port);
    inet_pton(AF_INET, ip, &address.sin_addr);
    address.sin_family = AF_INET;

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    int oldOpt = setNoBlock(sockfd);
    if(connect(sockfd, (struct sockaddr*)&address, sizeof(address)) == 0)
    {
        printf("connect successed\n");
        fcntl(sockfd, F_SETFL, oldOpt);
        return sockfd;
    }
    else if(errno != EINPROGRESS)
    {
        printf("unboock connect not support\n");
        return -1;
    }

    fd_set readfds;
    fd_set writefds;
    struct timeval timeout;
    timeout.tv_sec = time;
    timeout.tv_usec = 0;

    FD_ZERO(&readfds);
    FD_SET(sockfd, &writefds);
    
    res = select(sockfd + 1, NULL, &writefds, NULL, &timeout);
    if(res <= 0)
    {
        printf("connect timeout\n");
        close(sockfd);
        return -1;
    }

    if(!FD_ISSET(sockfd, &writefds))
    {
        printf("no events on sockfd found\n");
        close(sockfd);
        return -1;
    }
    
    int error = 0;
    socklen_t length = sizeof(error);
    
    if(getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &length) < 0)
    {
        printf("get socket option failed\n");
        close(sockfd);
        return -1;
    }

    if(error != 0)  //connect fail
    {
        printf("connection failed after select with the error : %d \n", error);
        close(sockfd);
        return -1;
    }

    printf("connection ready after select with the socket :%d \n", sockfd);
    fcntl(sockfd, F_SETFL, oldOpt);
    char sendbuf[bufferSize];
    char recvbuf[bufferSize];
    bzero(sendbuf, sizeof(sendbuf));
    bzero(recvbuf, sizeof(recvbuf));
    while(fgets(sendbuf, sizeof(sendbuf), stdin) != NULL) //这里可以将读写分离，以便及时对服务器端断开连接做出反应
    {
        write(sockfd, sendbuf, strlen(sendbuf));
        int ret = read(sockfd, recvbuf, sizeof(recvbuf));
        if(ret == 0)
        {
            cout << "server close" << endl;
            break;
        }
        cout << recvbuf << endl;
        bzero(sendbuf, sizeof(sendbuf));
        bzero(recvbuf, sizeof(recvbuf));
    }
    return sockfd;

}

int main(int argc, char** argv)
{
    if(argc <= 2)
    {
        //printf("usage: %s ip_address port_number\n", basename(argv[0]));
        return -1;
    }

    const char* ip = argv[1];
    int port = atoi(argv[2]);
    int sockfd = unblockConnect(ip, port, 10);
    if(sockfd < 0)
    {
        return 1;
    }
    return 0;
}
