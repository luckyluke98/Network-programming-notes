#include <ncurses.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <sys/wait.h>
#include <signal.h>

#include <errno.h>

#include <pthread.h>

#define INPUT_HEIGHT 3
#define MAX_MSG_LEN 256
#define PORT "3490"

int setup_connection(char *nickname) {
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

    // Manda come primo messaggio il nickname
    if (send(socket_fd, nickname, strlen(nickname), 0) == -1) {
        perror("Errore send");
    }

    return socket_fd;
} 

struct thread_args {
    int socket_fd;
    WINDOW *output_win;
}; 

void * listen_server(void *args) {
    struct thread_args *t_args = (struct thread_args *) args;
    int res_num_bytes;
    char res[50]; 

    while (1) {
        if ((res_num_bytes = recv(t_args->socket_fd, res, 50, 0)) <= 0) {
            if (res_num_bytes == -1) {
                perror("Error in recv");
                
            } 
            // Se è 0 il server ha chiuso il socket
            printf("Il server ha chiuso la connessione\n");
            close(t_args->socket_fd);
            break;
        }
    
        res[res_num_bytes] = '\0';
        // Stampa il messaggio nella finestra dei messaggi
        wprintw(t_args->output_win, "%s\n", res);
        wrefresh(t_args->output_win);
    }
}  

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Inserire nick\n./chat.out <nickname>\n");
        exit(1);
    }

    char *nickname = argv[1];

    int socket_fd = setup_connection(nickname);

    initscr();              // Inizializza ncurses
    cbreak();               // Disabilita buffering
    echo();                 // Mostra input qaundo scriviamo
    curs_set(1);            // Mostra il cursore

    int rows, cols;
    getmaxyx(stdscr, rows, cols); // Ottieni dimensioni terminale

    // Crea due finestre: output (sopra) e input (sotto)
    WINDOW *output_win = newwin(rows - INPUT_HEIGHT, cols, 0, 0);

    // newwin:
    // height: altezza della finestra, in righe.
    // width: larghezza della finestra, in colonne.
    // starty: riga iniziale (in alto = 0).
    // startx: colonna iniziale (a sinistra = 0).

    // output_win:
    // rows - INPUT_HEIGHT: prende tutta l’altezza dello schermo esclusa quella riservata all’input in basso.
    // cols: tutta la larghezza del terminale.
    // 0, 0: parte in alto a sinistra (riga 0, colonna 0).

    WINDOW *input_win = newwin(INPUT_HEIGHT, cols, rows - INPUT_HEIGHT, 0);
    // INPUT_HEIGHT: es. 3 righe.
    // cols: sempre tutta la larghezza.
    // rows - INPUT_HEIGHT: la fa partire appena sotto la finestra dei messaggi.
    // 0: sempre colonna iniziale.

    scrollok(output_win, TRUE); // Permetti scroll automatico
    box(input_win, 0, 0);       // Bordo per l’input

    wrefresh(output_win);
    wrefresh(input_win);

    char msg[MAX_MSG_LEN];

    pthread_t listener;
    struct thread_args t_args;

    t_args.socket_fd = socket_fd;
    t_args.output_win = output_win;

    if (pthread_create(&listener, NULL, listen_server, &t_args) != 0) {
        printf("Errore nella creazione del thread\n");
        return 1;
    }

    while (1) {
        // Cancella solo l’area interna della finestra input
        mvwgetnstr(input_win, 1, 2, msg, MAX_MSG_LEN - 1); // Input utentes

        if (strcmp(msg, "/quit") == 0) {
            close(socket_fd);
            break;
        }

        if (send(socket_fd, msg, strlen(msg), 0) == -1) {
            perror("Errore send");
            close(socket_fd);
            exit(1);
        }

        // Pulisce input
        werase(input_win);
        box(input_win, 0, 0);
        wrefresh(input_win);
    }

    // Pulizia finale
    delwin(output_win);
    delwin(input_win);
    endwin();

    pthread_cancel(listener);

    return 0;
}