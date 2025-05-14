# Network Programming Notes

Per puro interesse personale ho voluto approfondire la programmazione di rete in C. Mi ha sempre affascito il controllo a basso livello che può darti C, quindi ho voulto approfondirlo studiando un argomento visto solo in superfice.
Per studiare ho seguito le fantastiche guide di Beej ([questa in particolare](https://beej.us/guide/bgnet/)). Le ho trovate complete per qualcuno che si sta approcciando all'argomento, inoltre lo stile con cui scrive ti tiene appiccicato al monitor (sembra di leggere un romanzo :-D).

Ho voluto realizzare come semplice progetto una chat multiutente, andando ad utilizzare tutto il corredo di funzioni che permetto di accedere alle funzioanalità di rete in una macchina Unix.

# Progetto TCP Chat

Compilare il server:
```bash
gcc TCP_chat.c -o TCP_chat.out
```

Compilare il client:
```bash
gcc TCP_chat_client.c -o TCP_chat_client.out -lncurses
```
Eseguire Server:
```bash
./TCP_chat.out
```

Eseguire clinets:
```bash
# Tab 1
./TCP_chat_client.out Luca

# Tab 2
./TCP_chat_client.out John

# Tab 3
./TCP_chat_client.out Mike

# Tab 4
./TCP_chat_client.out Robert
```

Server Logs:

![image](https://github.com/user-attachments/assets/a11a715d-400e-4b86-9f24-0a65e644080c)

Robert perspective:

![image](https://github.com/user-attachments/assets/e09ec0de-bc93-441b-8d8b-eb9b17d84f0b)


Digitando ed inviando `/quit`, si lascierà la chat.

