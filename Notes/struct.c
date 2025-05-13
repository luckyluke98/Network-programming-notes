// Struttura usata per preparare le strutture degli indirizzi socket.
// You can force it to use IPv4 or IPv6 in the ai_family field, or 
// leave it as AF_UNSPEC to use whatever. This is cool because your 
// code can be IP version-agnostic.
// the next element—there could be several results for you to choose from. 
// I’d use the first result that worked, but you might have different b
struct addrinfo {
    int              ai_flags;     // AI_PASSIVE, AI_CANONNAME, etc.
    int              ai_family;    // AF_INET, AF_INET6, AF_UNSPEC
    int              ai_socktype;  // SOCK_STREAM, SOCK_DGRAM
    int              ai_protocol;  // use 0 for "any"
    size_t           ai_addrlen;   // size of ai_addr in bytes
    struct sockaddr *ai_addr;      // struct "sockaddr_in" or _in6
    char            *ai_canonname; // full canonical hostname

    struct addrinfo *ai_next;      // linked list, next node
};

// Questa struttura, sockaddr(), fortunatamente possiamo non scriverci direttamente.
// lo puo fare per noi getaddrinfo().
// In ogni caso la struttura è cosi formata
struct sockaddr {
    unsigned short    sa_family;    // address family, AF_xxx, 2 bytes
    char              sa_data[14];  // 14 bytes of protocol address
};
// sa_family -> AF_INET(IPv4), AF_INET6(IPv6), ecc
// sa_data -> contains a destination address 
// and port number for the socket

// Risulterebbe scomodo inserire i dati a mano in sa_data. I programmatori per 
// gestire struct sockaddr hanno creato la struttura "struct sockadr_in" per 
// essere usato con IPv4.

// (IPv4 only--see struct sockaddr_in6 for IPv6)

struct sockaddr_in {
    short int          sin_family;  // Address family, AF_INET. 2 bytes
    unsigned short int sin_port;    // Port number. 2 bytes (65535)
    struct in_addr     sin_addr;    // Internet address. 4 bytes
    unsigned char      sin_zero[8]; // Same size as struct sockaddr. 8 byte
};

// Un pointer a struct sockaddr_in può essera castato as una struct sockaddr e viceversa.
// quindi per esempio anche se connect() vuole una struct sockaddr*, è possibile
// usare struct sockaddr_in e castarla l'ultimo.

// Note that sin_zero (which is included to pad the structure to the length of a 
// struct sockaddr) should be set to all zeros with the function memset().
// sin_port must be in Network Byte Order (by using htons()!)

// (IPv4 only--see struct in6_addr for IPv6)

// Internet address (a structure for historical reasons)
struct in_addr {
    uint32_t s_addr; // that's a 32-bit int (4 bytes)
};

// struct sockaddr_storage that is designed to be large enough to hold both IPv4 
// and IPv6 structures. See, for some calls, sometimes you don’t know in advance 
// if it’s going to fill out your struct sockaddr with an IPv4 or IPv6 address. 
// So you pass in this parallel structure, very similar to struct sockaddr except 
// larger, and then cast it to the type you nee
struct sockaddr_storage {
    sa_family_t  ss_family;     // address family

    // all this is padding, implementation specific, ignore it:
    char      __ss_pad1[_SS_PAD1SIZE];
    int64_t   __ss_align;
    char      __ss_pad2[_SS_PAD2SIZE];
};

// esempio
struct sockaddr_in sa; // IPv4
struct sockaddr_in6 sa6; // IPv6

inet_pton(AF_INET, "10.12.110.57", &(sa.sin_addr)); // IPv4
inet_pton(AF_INET6, "2001:db8:63b3:1::3490", &(sa6.sin6_addr)); // IPv6

// First, let’s say you have a struct sockaddr_in ina, and you have an IP address 
// “10.12.110.57” or “2001:db8:63b3:1::3490” that you want to store into it. 
// The function you want to use, inet_pton(), converts an IP address in 
// numbers-and-dots notation into either a struct in_addr or a struct in6_addr 
// depending on whether you specify AF_INET or AF_INET6. (“pton” stands for 
// “presentation to network”—you can call it “printable to network” if that’s 
// easier to remember.

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

int getaddrinfo(const char *restrict node,
                const char *restrict service,
                const struct addrinfo *restrict hints,
                struct addrinfo **restrict res);

void freeaddrinfo(struct addrinfo *res);

const char *gai_strerror(int errcode);

// domain: AF_INET, AF_INET6, ecc
// type: SOCK_STREAM, SOCK_DGRAM, ecc

#include <sys/socket.h>

int socket(int domain, int type, int protocol);

// On success, zero is returned.  On error, -1 is returned, and errno
// is set to indicate the error.

#include <sys/socket.h>

int bind(int sockfd, const struct sockaddr *addr,
        socklen_t addrlen);

// On success, zero is returned.  On error, -1 is returned, and errno
// is set to indicate the error.

#include <sys/socket.h>

int listen(int sockfd, int backlog);

#include <sys/socket.h>

int accept(int sockfd, struct sockaddr *_Nullable restrict addr,
            socklen_t *_Nullable restrict addrlen);
            
// The argument sockfd is a socket that has been created with
// socket(2), bound to a local address with bind(2), and is listening
// for connections after a listen(2).
// The argument addr is a pointer to a sockaddr structure.  This
// structure is filled in with the address of the peer socket, as
// known to the communications layer.  The exact format of the
// address returned addr is determined by the socket's address family
// (see socket(2) and the respective protocol man pages).  When addr
// is NULL, nothing is filled in; in this case, addrlen is not used,
// and should also be NULL.
// The addrlen argument is a value-result argument: the caller must
// initialize it to contain the size (in bytes) of the structure
// pointed to by addr; on return it will contain the actual size of
// the peer address.
// On success, these system calls return a file descriptor for the
// accepted socket (a nonnegative integer).  On error, -1 is
// returned, errno is set to indicate the error, and addrlen is left
// unchanged.