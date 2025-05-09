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

void sigchld_handler(int s) {
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;

    while(waitpid(-1, NULL, WNOHANG) > 0);

    errno = saved_errno;
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main() {

    int status;

    struct addrinfo hints, *p;
    struct addrinfo *servinfo; // Punterà al risultato

    struct sockaddr_storage incoming_addr;

    struct sigaction sa;

    int socket_fd, new_fd; // la funzione socket() ritornerà un socket descriptor o -1 se ci sono errori.

    socklen_t addr_size;

    char s[INET6_ADDRSTRLEN];

    memset(&hints, 0, sizeof hints);

    hints.ai_family = AF_UNSPEC;     // don't care IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
    hints.ai_flags = AI_PASSIVE;     // fill in my IP for me

    // Se volevamo specificare un altro IP oltre al nostro
    // come primo parametro di getaddrinfo avremmo messo
    // per esempio "www.examle.com," oppure un IP.
    // La funzione risolve per noi i nomi, fa il DNS lookup.
    
    if (status = getaddrinfo(NULL, PORT, &hints, &servinfo) != 0) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        // The gai_strerror() function shall return a text string describing
        // an error value for the getaddrinfo() and getnameinfo() functions
        // listed in the <netdb.h> header.
        exit(1);
    }

    // se andata a buon fine dentro servinfo dovremmo avere una linked-list con
    // tutte le struct addrinfo che la funzione ha creato.

    for (p = servinfo; p != NULL; p = p->ai_next) {
        if (socket_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol) == -1) { 
            perror("Error create socket file descriptor");
            continue;            
        } 
        
        // Facciamo il bind del socket all'host sui il programma sta girando sulla pora 3490
        // Quindi è il socket che viene bind-ato alla porta specificata
        if (bind(socket_fd, p->ai_addr, p->ai_addrlen) == -1) {
            close(socket_fd); // Regular Unix file descriptor close. Se è fallito il bind allora
            // chiudiamo il socket che si era aperto e rifacciamo tutto, il continue blocca le iterazioni e 
            // fa ripartire all'iterazione dopo.
            // close() previene ogni altra lettura o scrittura al socket, ogni tentativo fatto dal host remoto darà errore.
            perror("Error bind socket");
            continue; 
        } 

        break;
    }
    freeaddrinfo(servinfo); // Non serve, la liberiamo.

    if (p == NULL){
        fprintf(stderr, "Server fail to bind");
        exit(1);
        // Exit code 0        Success
        // Exit code 1        General errors, Miscellaneous errors, such as "divide by zero" and other impermissible operations
        // Exit code 2        Misuse of shell builtins (according to Bash documentation)
    }

    // Se tutto andato bene dentro socket_fd abbiamo il fd del socket alla prima entru che va bene
    // in servinfo

    // Inoltre se tutto è andato bene abbiamo un socket file descriptor che è bind-ato alla porta 3490
    // Il bind si fa solitamente se dopo si aspettano connessioni in etrata ad un porta specifica
    // Prossimo step sarebbe il listen(), accept()

    // Attendere per chiamate in entrata
    if(listen(socket_fd, BACKLOG) == -1) {
        perror("Error in listen");
        exit(1);
    } 

    // BACKLOG è il numero di richieste entranti massimo per la coda. Le connessioni in entrata
    // attenderanno nella coda fino a qaundo una accept()

    // accept() dice di prendere dalla cosa un connessione in pending, ritornerà un nuovo socket file descriptor
    // da usare per la singola connessione.
    // Quello originale sta ancora ascoltando per connessioni.
    // Quello nuovo adesso potrà usare send() o recv().

    //   addr_size = sizeof incoming_addr;
    //   new_fd = accept(socket_fd, (struct sockaddr *)&incoming_addr, &addr_size);

    // accept(), prende come primo parametro il listening socket fd; addr è dove le informazioni 
    // della connessione in arrivo verranno salvate; addrlen dovrebbe essere settato come
    // sizeof(struct sockaddr_storage) prima che il suo indirizzo sia passato alla funzione accept().
    // accept() non metterà più di quello specificato da addrlen in addr. se viene riempito meno
    // allora dentro addrlen troveremo un valore aggiornato.

    // vediamo come farlo per più richieste

    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    // sigaction dice come comportarsi in caso di seganle SIGCHLD assegando l'handler creato.

    // Quando un processo figlio termina, il sistema invia al processo padre un segnale SIGCHLD. 
    // Se il padre non gestisce questo segnale, i processi figli terminati rimangono in stato di "zombie", 
    // occupando risorse di sistema fino a quando il padre li "reap" (cioè li raccoglie) usando wait() o simili.
    // while(waitpid(-1, NULL, WNOHANG) > 0);
    // Usa waitpid in un ciclo per raccogliere tutti i figli terminati.

    // -1: indica "qualunque processo figlio"
    // NULL: non ci interessa lo status di uscita
    // WNOHANG: non bloccare, restituisci subito se non ci sono figli terminati

    // Scorre tutti i figli del processo padre, se uno è terminato waitpid libra il suo spazio e lo rimuove sta stato di zombie.

    // Il segnale SIGCHLD è non accodabile: se ne arriva uno mentre un altro è già "in sospeso", non verrà generato un secondo segnale.
    // Per questo facciamo il while. per che finiscono in contemporanea un solo seganel verrà lanciato

    // se più figli terminano quasi contemporaneamente, **il kernel invia **solo un segnale SIGCHLD al padre — non uno per ogni figlio.
    // Se un segnale è già "pendente" (cioè il processo padre non lo ha ancora gestito), il kernel non aggiunge altri segnali uguali in attesa. 

    printf("server: waiting for connections...\n");

    // Quando un server chiama listen() e poi accept(), il kernel del sistema operativo gestisce internamente il 3-way 
    // handshake TCP tra il client e il server

    while(1) { // loop di accept
        addr_size = sizeof incoming_addr;
        new_fd = accept(socket_fd, (struct sockaddr *)&incoming_addr, &addr_size); // Se non ci sono client che cercano di connettersi, 
                                                                                   // il programma rimane fermo su questa riga finché qualcuno non si connette.

        if (new_fd == -1) {
            perror("Error in accept");
            continue;
        }

        inet_ntop(incoming_addr.ss_family,
            get_in_addr((struct sockaddr *)&incoming_addr),
            s, sizeof s);
        printf("server: got connection from %s\n", s);

        if (!fork()) { // this is the child process
            close(socket_fd); // child doesn't need the listener
            if (send(new_fd, "Hello, world!", 13, 0) == -1)
                perror("send");
            close(new_fd);
            exit(0);
        }
        close(new_fd);  // parent doesn't need this
    }
    
    return 0;
}