#include <iostream>
#include <cassert>
#include <sys/epoll.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <string.h>
#include <errno.h>
#include <vector>

#define ERROR_EXIT(m) \
    do \
    { \
        perror(m);\
        exit(EXIT_FAILURE);\
    }while(0)


using namespace std;

const int maxListenFd = 5;
const int initEventListSize = 16;
const int maxEvents = 500;
const int bufferSize = 10;


void setNoBlock(int fd)
{
    int op = fcntl(fd, F_GETFL);
    op |= O_NONBLOCK;
    fcntl(fd, F_SETFL, op);
}

int createAndListen()
{
    int ret = 0;
    int on = 1;
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    assert(listenfd >= 0);

    ret = setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    assert(ret != -1);
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(8080);
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    //address.sin_addr.s_addr = inet_addr("127.0.0.1");
    //inet_aton("127.0.0.1",&address.sin_addr);
    
    ret = bind(listenfd, (struct sockaddr*)&address, sizeof(address));
    assert(ret != -1);
    
    ret = listen(listenfd, maxListenFd);
    assert(ret != -1);
    return listenfd;
}

void addfd(int epollfd, int fd, bool etmode)
{
    epoll_event ev;
    ev.data.fd = fd;
    ev.events = EPOLLIN;
    if(etmode)
    {
        ev.events |= EPOLLET;
    }
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev);
    setNoBlock(fd);
}

typedef vector<epoll_event> EpollList;

int main(int argc, char* argv[])
{
    int ret = 0;
    char buffer[bufferSize];
    EpollList eventList(initEventListSize);
    int epollfd = epoll_create1(EPOLL_CLOEXEC);
    assert(epollfd >= 0);
    int listenfd = createAndListen();
    addfd(epollfd, listenfd, true);
    sockaddr_in cliaddr; 
    int clilen = sizeof(cliaddr);
    while(true)
    {
        int number = epoll_wait(epollfd, &*eventList.begin(), static_cast<int>(eventList.size()), -1);
        if(number == -1)
        {
            if(errno == EINTR)
                continue;
            cout << "epoll_wait error ,errno :" << errno << endl;
            break;
        }
        if(number == 0)
            continue;
        if((size_t)number == eventList.size())
        {
            eventList.resize(eventList.size()*2);
        }
            
        for(int i = 0; i < number; ++i)
        {
            int fd = eventList[i].data.fd;
            if(fd == listenfd)
            {
                //int conn = accept(fd, (sockaddr*)&cliaddr, (socklen_t*)&clilen);
                int conn = accept4(fd, (sockaddr*)&cliaddr, (socklen_t*)&clilen, SOCK_CLOEXEC);
                cout << "client ip :" << inet_ntoa(cliaddr.sin_addr) << "client port :" << ntohs(cliaddr.sin_port) << endl;
                addfd(epollfd, conn, true);
            }
            else if(eventList[i].events & EPOLLIN)
            {
                while(true)
                {
                    bzero(buffer, bufferSize);
                    ret = read(fd, buffer, bufferSize);
                    if(ret < 0)
                    {
                        if(errno == EAGAIN)
                        {
                            cout << endl;
                            cout << "read later " << endl;
                            break;
                        }
                        close(fd);
                        break;
                    }
                    else if(ret == 0)
                    {
                        cout << "read 0 close sockfd :" << fd << endl;
                        close(fd);
                        break;
                    }
                    else
                    {
                        cout << buffer;
                        if(write(fd, buffer, ret) != ret)
                        {
                            cout << "error: cant finish one time" << endl;
                        }
                    }
                }
            }
            else
            {
                cout << "something else happened" << endl;
            }
        }
    }
    close(listenfd);
}
