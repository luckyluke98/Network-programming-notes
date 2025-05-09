#include <unistd.h>
#include <fcntl.h>

sockfd = socket(PF_INET, SOCK_STREAM, 0);
fcntl(sockfd, F_SETFL, O_NONBLOCK);

// accept(), recv() sono bloccanti. Quando creiamo il socket il kernel lo setta
// come bloccante. Se vogliamo con lo sia dobbiamo fare una chimata a
// fcntl(), in questo modo possiamo fare polling tra i socket file descriptor.
// Se leggiamo un non-blocking socket e non ci sono dati non si bloccherà
// in attesa di questi.
// Ritornerà -1 e errno sarà settato come EAGAIN o EWOULDBLOCK.

// L'idea di questo polling non è top. f you put your program in a busy-wait 
// looking for data on the socket, you’ll suck up CPU time like it was 
// going out of style.

// Una soluzione elgante è poll(). Farà per noi un controllo tra i vari socket fd che
// abbiamo se ce ne qualcuno pronto. This way you don’t have to continuously poll all 
// those sockets to see which are ready to read.

// The general gameplan is to keep an array of struct pollfds with 
// information about which socket descriptors we want to monitor, 
// and what kind of events we want to monitor for. The OS will block 
// on the poll() call until one of those events occurs (e.g. “socket 
// ready to read!”) or until a user-specified timeout occurs.

#include <poll.h>

int poll(struct pollfd fds[], nfds_t nfds, int timeout);

// fds is our array of information (which sockets to monitor for what), 
// nfds is the count of elements in the array, and timeout is a timeout 
// in milliseconds. It returns the number of elements in the array that 
// have had an event occur.
// You can specify a negative timeout to wait forever.

struct pollfd {
    int fd;         // the socket descriptor
    short events;   // bitmap of events we're interested in
    short revents;  // when poll() returns, bitmap of events that occurred
};

// The events field is the bitwise-OR of the following:
// - POLLIN : Alert me when data is ready to recv() on this socket.
// - POLLOUT : Alert me when I can send() data to this socket without blocking.
// - POLLHUP : Alert me when the remote closed the connection.

