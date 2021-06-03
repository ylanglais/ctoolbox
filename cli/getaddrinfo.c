#include <stdio.h>
#include <stdlib.h>


#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>


int main(void) {
    char dotted_quad[16];
    struct addrinfo *addr, *rp;


    if (getaddrinfo("google.com", NULL, NULL, &addr) == -1) {
        perror("getaddrinfo");
        return 1;
    }


    for (rp = addr; rp != NULL; rp = rp->ai_next) {
        if (inet_ntop(AF_INET, &(((struct sockaddr_in *) rp->ai_addr)->sin_addr), dotted_quad, sizeof(dotted_quad)) == NULL) {
            perror("inet_ntop");
        } else {
            puts(dotted_quad);
        }
    }

    return 0;
}
