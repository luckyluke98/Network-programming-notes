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
#define BACKLOG 10
#define MAX_FDS 5


int main() {

    struct addrinfo *srvinfo, hints;
    struct addrinfo *p; // Puntatore di supporto per scorrere srvinfo
    
    int status;

    int socket_fd; // file descriptor del socket
    socklen_t addrlen; // addrlen specifies the size, in bytes,
                       // of the address structure pointed to by addr. 

    struct pollfd *fds;
    int fds_count = 0;

    // Prepariamo hints per essere passato a getaddrinfo()
    memset(&hints, 0, sizeof hints);

    // Array di file descriptrs per più connessioni. Da cui faremo poll.
    fds = (struct pollfd *) malloc(sizeof (struct pollfd) * MAX_FDS);

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    // Cerchiamo di ottenere le info relative al nodo (nostra macchina)
    if ((status = getaddrinfo(NULL, PORT, &hints, &srvinfo)) != 0) {
        fprintf(stderr, "Errore getaddrinfo; %s\n", gai_strerror(status));
        exit(1);
    } 
    
    for (p = srvinfo; p != NULL; p = p -> ai_next) {
        
        // Adesso per le info che abbiamo ottenuto cerchiamo di ottenere un socket
        if ((socket_fd = socket(p -> ai_family, p -> ai_socktype, p -> ai_protocol)) == -1) {
            perror("Errore socket");
            continue;
        }

        // Se il socket è stato creato con successo facciamo il bind
        if ((status = bind(socket_fd, p -> ai_addr, p -> ai_addrlen)) == -1) {
            perror("Errore bind");
            close(socket_fd);
            continue;
        } 

        break;
    } 

    freeaddrinfo(srvinfo);

    if (p == NULL) {
        perror("Fallito bind socket, o creazione socket");
        exit(1);
    }  

    // Arrivati qui abbiamo un socket bindato a PORT
    if (listen(socket_fd, BACKLOG) == -1) {
        perror("Errore listen");
        close(socket_fd);
        exit(1);
    } 

    // Inseriamo socket listener nell'array dei fds
    fds[0].fd = socket_fd;
    fds[0].events = POLLIN;
    fds_count++;

    while (1) {
        int poll_count = poll(fds, fds_count, -1);

        if (poll_count == -1) {
            perror("Errore nella poll");
            continue;
        }

        for (int i = 0; i < poll_count; i++) {

            if (fds[i].revents & (POLLIN | POLLHUP) ) {
                
                // Se è il listener vuol dire che abbiamo una connessione
                // in entrata
                if (fds[i].fd == socket_fd) {

                    // Se c'è posto nella chat
                    if (fds_count <= MAX_FDS - 1) {
                        int new_socket;
                        struct sockaddr in_addr;
                        socklen_t addrlen;

                        addrlen = sizeof in_addr;

                        if ((new_socket = accept(socket_fd, &in_addr, &addrlen)) == -1) {
                            perror("accept");
                        }
                        // Aggiungiamo alle connessioni attive
                        else {
                            fds[fds_count].fd = new_socket;
                            fds[fds_count].events = POLLIN;
                            fds[fds_count].revents = 0; 
                            fds_count++; 

                            // Inviamo a tutti che si è unito qualcuno
                        } 
                    }
                }
                // Non è il listener
                else {
                    // Riceviamo i dati

                    // Se riceviamo i dati correttamente
                    // Inviamo a tutti i dati

                    // Altrimenti il client si è scollegato
                    // Notifichiamo a tutti
                }  

            } 

        }
    } 

    return 0;
} 