# Network Programming Notes

Per interesse personale ho voluto approfondire la programmazione di rete in C. Mi ha sempre affascinato il controllo a basso livello che può darti C, quindi ho voulto approfondirlo studiando un argomento visto solo in superficie.
Per studiare ho seguito le fantastiche guide di Beej ([questa in particolare](https://beej.us/guide/bgnet/)).
Come piccolo progetto ho voluto realizzare una semplice chat multi-utente. Il server fa un bind di un porta sul socket e ascolta per connessioni in arrivo. Il client apre un socket e si connette al server sulla porta.
Il server per ogni connessione accettata gestisce i file descriptor dei socket attraverso un array di `struct pollfd`.
Trovate tutto nel codice.

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

![image](https://github.com/user-attachments/assets/3a9e9c8a-2e47-4e74-9449-115c77391ffb)


Robert perspective:

![image](https://github.com/user-attachments/assets/e09ec0de-bc93-441b-8d8b-eb9b17d84f0b)


Digitando ed inviando `/quit`, si lascierà la chat.

