#include <sys/socket.h>
#include <iostream>
#include <sys/types.h>
#include <errno.h>

#define ERROR_EXIT(m) do\
            { \
                perror(m);\
                exit(EXIT_FAILURE);\
            }while(0)

using namespace std;

int main(void)
{
    int fd1 = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    int fd2 = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    sockaddr_in address;
    if(inet_aton("127.0.0.1", &address.sin_addr) == 0)
    {
        ERROR_EXIT("address error");
    }
    
    int on = 1;
    setsockopt(fd1, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    setsockopt(fd1, SOL_SOCKET, SO_REUSEPORT, &on, sizeof(on));
    setsockopt(fd2, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    setsockopt(fd2, SOL_SOCKET, SO_REUSEPORT, &on, sizeof(on));
    unsigned int port = 8080;
    address.sin_port = htons(port);
    bind(fd1, &address, sizeof(address));
    bind(fd2, &address, sizeof(address));
    
}
