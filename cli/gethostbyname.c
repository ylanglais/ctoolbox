#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

void
hostent_print(struct hostent *h) {
	int i = 0;
	printf("\n\thost->h_name = %s\n", h->h_name);
	while (h->h_aliases[i]) {
		printf("\thost->h_aliases[%d] = %s\n", i, h->h_aliases[i]);			
		i++;
	}
	printf("\thost->h_addrtype = %d\n", h->h_addrtype);
	printf("\thost->h_length   = %d\n", h->h_length);
	for (i = 0; i < h->h_length && h->h_addr_list[i] != NULL; i++) {
		printf("\th->h_addr_list = %s\n", inet_ntoa(*(struct in_addr *) (h->h_addr_list[i])));
	}
}

int main(int n, char *a[]) {
	int i;
	struct hostent *host;

	printf("\
***************************************************************\n\
*                                                             *\n\
* Please note:                                                *\n\
* ------------                                                *\n\
* Gethostbyname is a deprecated method for getting host infos *\n\
* use getaddrinfo() instead!                                  *\n\
*                                                             *\n\
***************************************************************\n");

	for (i = 1; i < n ; i++) {
		host = gethostbyname(a[i]);
		printf("host number %d %s : ", i, a[i]);
		if (host) hostent_print(host);
		else      printf("not found\n");
	}
	return 0;
}

