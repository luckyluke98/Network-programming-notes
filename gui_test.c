#include <ncurses.h>
#include <string.h>
#include <stdlib.h>

#define INPUT_HEIGHT 3
#define MAX_MSG_LEN 256

int main() {
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

    while (1) {
        // Cancella solo l’area interna della finestra input
        mvwgetnstr(input_win, 1, 2, msg, MAX_MSG_LEN - 1); // Input utentes

        if (strcmp(msg, "/quit") == 0) break;

        // Stampa il messaggio nella finestra dei messaggi
        wprintw(output_win, "Tu: %s\n", msg);
        wrefresh(output_win);

        // Pulisce input
        werase(input_win);
        box(input_win, 0, 0);
        wrefresh(input_win);
    }

    // Pulizia finale
    delwin(output_win);
    delwin(input_win);
    endwin();

    return 0;
}