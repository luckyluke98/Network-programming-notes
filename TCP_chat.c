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

#include <poll.h>

#include <errno.h>

#define PORT "3490"
#define BACKLOG 10
#define MAX_FDS 5

/**
 * Questa struct serve per memorizzare le informazioni del client che si connette
 * La struct è appaiata con quella dei file descriptors dei socket.
 * Serve perchè per i dati in arrivo conosco solo il socket. Al momento della connessione
 * mi salvo l'ip. Successivamente, il primo messaggio (che sarà di join) il client
 * mi invia il suo nick che mi salvo. Al primo messaggio avrà has_nick a false.
 */
struct client_info {
    int has_nick;
    char nick[50];
    char ip[INET6_ADDRSTRLEN];
};

/**
 * Serve perche nel main per memorizzare le info del client
 * che si connette, uso "struct sockaddr_storage", che serve per memorizzare le info in arrivo
 * senza sapere se sono IPv4 o IPv6. Facciamo un cast prima di passarla alla funzione.
 * Il cast funziona perchè nelle due struct i primi 2 bytes contengono la famiglia (AF_INET o AF_INET6),
 * che è l'info che serve poi per fare il successivo cast.
 * Quindi in base a quello poi casterò sockaddr in base al tipo di socket.
 */
void * get_in_addr(struct sockaddr * ip) {
    if (ip->sa_family == AF_INET) {
        return &(((struct sockaddr_in *) ip)->sin_addr);
    } 

    return &(((struct sockaddr_in6 *) ip)->sin6_addr);
} 

int main() {
    // In srvinfo memorizzo le info che mi da getaddrinfo(), hints invece è una 
    // variabile che mi serve per "instruire" getaddrinfo() sul tipo address (AF_INET ecc)
    // sul tipo di socket e l'hostname.
    struct addrinfo *srvinfo, hints;
    struct addrinfo *p; // Puntatore di supporto per scorrere srvinfo
    
    int status; // Per memorizzare stati di ritorno
    int socket_fd; // File descriptor del socket listener, quello
                   // che si occuperà  di gestire le nuove connessioni in arrivo.

    // Variabili per gestire file descriptors e info sui client
    struct pollfd *fds;
    int fds_count = 0;
    struct client_info *clients_info;

    // Variabili per gestinre nuova connessione
    char remote_ip[INET6_ADDRSTRLEN]; // Dove memorizzo l'ip
    int new_socket; // file descriptor per il nuovo socket dopo la accept()
    struct sockaddr_storage in_addr; // Serve per memorizzare l'address senza assumere che sia IPv4 o IPv6
    socklen_t addrlen; // addrlen specifies the size, in bytes,
                       // of the address structure pointed to by addr. 

    // Prepariamo hints per essere passato a getaddrinfo()
    memset(&hints, 0, sizeof hints);

    // Array di file descriptrs per più connessioni. Da cui faremo poll.
    fds = (struct pollfd *) malloc(sizeof (struct pollfd) * MAX_FDS);
    // Array "appaiato" a fds che memorizza le info sui client
    clients_info = (struct client_info *) malloc(sizeof (struct client_info) * MAX_FDS);

    // Prepariamo hints, quindi sul tipo di address, socket ed eventualmente hostname
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    //hints.ai_flags = AI_PASSIVE;  // Se mettiamo questo allora stiamo dicendo che 
                                    // vogliamo come IP quello della macchina

    // Cerchiamo di ottenere le info relative al nodo (nostra macchina)
    if ((status = getaddrinfo("localhost", PORT, &hints, &srvinfo)) != 0) {
        fprintf(stderr, "Errore getaddrinfo; %s\n", gai_strerror(status));
        exit(1);
    } 
    
    // getaddrinfo() ci ritorna una lista di addrinfo, noi faremo il bind sulla prima
    // che avrà successo
    for (p = srvinfo; p != NULL; p = p->ai_next) {
        // Adesso per le info che abbiamo ottenuto cerchiamo di ottenere un socket
        if ((socket_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("Errore socket");
            continue;
        }
        // Se il socket è stato creato con successo facciamo il bind
        if ((status = bind(socket_fd, p->ai_addr, p->ai_addrlen)) == -1) {
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

        for (int i = 0; i < fds_count; i++) {
            if (fds[i].revents & (POLLIN | POLLHUP) ) {
                // Se è il listener vuol dire che abbiamo una connessione in entrata
                if (fds[i].fd == socket_fd) {
                    
                    addrlen = sizeof in_addr;

                    if ((new_socket = accept(socket_fd, (struct sockaddr *) &in_addr, &addrlen)) == -1) {
                        perror("Errore accept");
                    }
                    else {
                        // Limite raggiunto, chiudiamo la connessione immediatamente
                        if (fds_count >= MAX_FDS) {
                            close(new_socket);
                            continue;
                        }

                        printf("[Server] Nuova connessione da %s\n", 
                            inet_ntop(in_addr.ss_family,
                                get_in_addr((struct sockaddr*)&in_addr),
                                remote_ip, INET6_ADDRSTRLEN));
                            
                        // Aggiungiamo alle connessioni attive
                        fds[fds_count].fd = new_socket;
                        fds[fds_count].events = POLLIN;
                        fds[fds_count].revents = 0; 
                        
                        clients_info[fds_count].has_nick = 0;
                        snprintf(clients_info[fds_count].ip, sizeof(clients_info[fds_count].ip), "%s", remote_ip);

                        fds_count++;
                    }  
                }
                // Non è il listener
                else {
                    // Riceviamo i dati
                    char msg[50]; // Salviamo Messaggio qua
                    char buf[50]; // buffer di supporto
                    int bytes = recv(fds[i].fd, msg, sizeof msg, 0);

                    if (bytes <= 0) {
                        if (bytes == 0) {
                            // Connection closed
                            snprintf(buf, sizeof buf, "*** %s ha lasciato la chat! ***", clients_info[i].nick); 
                              
                            printf("[Server] Utente \"%s\" si è disconesso!\n", clients_info[i].nick);
                            
                            // Notifichiamo a tutti
                            for (int c = 0; c < fds_count; c++) {
                                if (fds[c].fd != socket_fd) {
                                    
                                    if (send(fds[c].fd, buf, sizeof buf, 0) == -1) {
                                        perror("Errore send");
                                    }
                                }
                            }
                        } else {
                            // se non gestisci correttamente un recv() 
                            // che ritorna -1 o 0, la poll continuerà a 
                            // dirti che c'è qualcosa da leggere, va chiuso il socket anche da server
                            perror("recv");
                        }

                        close(fds[i].fd);

                        fds[i] = fds[fds_count-1];
                        clients_info[i] = clients_info[fds_count-1];  
                        fds_count--;
                        i--;
                    } else {
                        msg[bytes] = '\0';
                        if (!clients_info[i].has_nick) {
                            clients_info[i].has_nick = 1; 
                            snprintf(clients_info[i].nick, sizeof(clients_info[i].nick), "%s", msg);
                            snprintf(buf, sizeof buf, "*** %s si è unito alla chat! ***", clients_info[i].nick); 
                                
                            printf("[Server] %s si è unito come \"%s\"!\n", 
                                clients_info[i].ip, 
                                clients_info[i].nick);
                            
                            // Notifichiamo a tutti
                            for (int c = 0; c < fds_count; c++) {
                                if (fds[c].fd != socket_fd) {
                                    
                                    if (send(fds[c].fd, buf, sizeof buf, 0) == -1) {
                                        perror("Errore send");
                                    }
                                }
                            }

                        } else {
                            snprintf(buf, sizeof buf, "[%s] %s", clients_info[i].nick, msg); 
                              
                            printf("[%s] %s\n", clients_info[i].nick, msg);
                            for (int c = 0; c < fds_count; c++) {
                                if (fds[c].fd != socket_fd) {
                                    
                                    if (send(fds[c].fd, buf, strlen(buf), 0) == -1) {
                                        perror("Errore send");
                                    }
                                }
                            }
                        }
                    }   
                }  
            }
        }
    }

    return 0;
} 