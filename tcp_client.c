#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <sys/wait.h>
#include <signal.h>

#include <errno.h>

#define PORT "3490"

int main() {

    struct addrinfo hints, *srvinfo, *p;

    int socket_fd, status, new_socket, res_num_bytes;

    char res[50];

    memset(&hints, 0, sizeof hints);

    hints.ai_family = AF_UNSPEC;     
    hints.ai_socktype = SOCK_STREAM;
    //hints.ai_flags = AI_PASSIVE;
    
    if (status = getaddrinfo("localhost", PORT, &hints, &srvinfo) != 0) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        // The gai_strerror() function shall return a text string describing
        // an error value for the getaddrinfo() and getnameinfo() functions
        // listed in the <netdb.h> header.
        exit(1);
    } 

    for (p = srvinfo; p != NULL; p = p->ai_next) {
        
        if ((socket_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) { 
            perror("Error create socket file descriptor");
            continue;            
        } 

        if (connect(socket_fd, p->ai_addr, p->ai_addrlen) == -1) {
            close(socket_fd);
            perror("Error in connect");
            continue;
        } 

        break;
    }

    freeaddrinfo(srvinfo);

    if (p == NULL) {
        perror("Client fail to get a socket");
        exit(1);
    } 

    printf("CONNESSIONE RIUSCITA\n");

    // recv() è bloccante.

    if (send(socket_fd, "CIAO", 4, 0) == -1) {
        perror("Errore send");
    }

    /** 
    if ((res_num_bytes = recv(socket_fd, res, 50, 0)) == -1) {
        perror("Error in recv");
        exit(1);
    } 
    printf("%d\n", res_num_bytes);

    res[res_num_bytes] = '\0';
    
    printf("Data ricevuti: %s\n", res);
*/
    close(socket_fd);
    
    return 0;

} 

