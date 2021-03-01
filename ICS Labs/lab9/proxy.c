/*
 * proxy.c - ICS Web proxy
 *
 * 518021910184 DengShiyi
 *
 */

#include "csapp.h"
#include <stdarg.h>
#include <sys/select.h>

#define BLOCK_SIZE 24586

/* Global variables */
/* thread argument */
typedef struct {
    int connfd;
    struct sockaddr_in clientaddr;
} threadArg_t;

sem_t mutex;

/*
 * Function prototypes
 */
ssize_t Rio_readlineb_w(rio_t *rp, void *usrbuf, size_t maxlen);
int Rio_writen_w(int fd, void *usrbuf, size_t n);
ssize_t Rio_readn_w(rio_t *rp, void *usrbuf, size_t n);
ssize_t Rio_readnb_w(rio_t *rp, void *usrbuf, size_t n);

int parse_uri(char *uri, char *target_addr, char *path, char *port);
void format_log_entry(char *logstring, struct sockaddr_in *sockaddr, char *uri, size_t size);
void *thread(void *vargp);
void doit(threadArg_t *vargp);
void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg);



/*
 * Rio_io_wrapper
 */

ssize_t Rio_readlineb_w(rio_t *rp, void *usrbuf, size_t maxlen) {
    ssize_t rc;

    if ((rc = rio_readlineb(rp, usrbuf, maxlen)) < 0){
        return 0;
    }
    return rc;
} 

int Rio_writen_w(int fd, void *usrbuf, size_t n) {
    if (rio_writen(fd, usrbuf, n) != n){
        return -1;
    }
    return n;
}

ssize_t Rio_readn_w(rio_t *rp, void *usrbuf, size_t n) {
    ssize_t rc;

    if ((rc = rio_readnb(rp, usrbuf, n)) < 0){
        return 0;
    }
    return rc;
}

ssize_t Rio_readnb_w(rio_t *rp, void *usrbuf, size_t n){
    ssize_t rc;

    if ((rc = rio_readnb(rp, usrbuf, n)) < 0){
        return 0;
    }
    return rc;
}

/*
 * main - Main routine for the proxy program
 */
int main(int argc, char **argv){
    int listenfd, connfd;
    socklen_t clientlen;
    struct sockaddr_in clientaddr;
    pthread_t tid;
    threadArg_t *arg;

    /* Check arguments */
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port number>\n", argv[0]);
        exit(0);
    }

    Signal(SIGPIPE, SIG_IGN);
    Sem_init(&mutex, 0, 1);
    listenfd = Open_listenfd(argv[1]);
    while (1){
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *) &clientaddr, &clientlen);
        arg = (threadArg_t *)Malloc(sizeof(threadArg_t));
        arg->connfd = connfd;
        arg->clientaddr = clientaddr;
        Pthread_create(&tid, NULL, thread, arg);
    }

    exit(0);
}

/* Thread routine */
void *thread(void *vargp){
    Pthread_detach(pthread_self());
    doit((threadArg_t *) vargp);
    Close(((threadArg_t *)vargp)->connfd);
    Free(vargp);
    return NULL;
}

/*
 * hostname is from the browser
 */
void doit(threadArg_t *vargp){
    char buf[MAXLINE], long_buf[BLOCK_SIZE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
    char serverHostname[MAXLINE], serverPort[MAXLINE], path[MAXLINE];
    int clientfd, fd, read_size = 0, total_size = 0, content_length = 0;
    rio_t rio_client, rio_server;

    fd = vargp->connfd;

    /* Read request line and headers */
    Rio_readinitb(&rio_client, fd);
    if(Rio_readlineb_w(&rio_client, long_buf, BLOCK_SIZE) == 0){
        return;
    }
    if(sscanf(long_buf, "%s %s %s", method, uri, version) != 3){
        return;
    }
    if(parse_uri(uri, serverHostname, path, serverPort) == -1){
        return;
    }

    /* Establish connection to end server */
    if((clientfd = Open_clientfd(serverHostname, serverPort)) < 0){
        return;
    }
    Rio_readinitb(&rio_server, clientfd);
    sprintf(long_buf, "%s /%s %s\r\n", method, path, version);
    Rio_writen_w(clientfd, long_buf, strlen(long_buf));

    /* Deal with request header */
    content_length = 0;
    while((read_size = Rio_readlineb_w(&rio_client, buf, MAXLINE)) != 0){
        char *head;
        if((head = strstr(buf, "Content-Length:"))){
            head = head + 16;
            content_length = atoi(head);
        }
        Rio_writen_w(clientfd, buf, read_size);
        if(strcmp(buf, "\r\n") == 0){
            break;
        }
    }

    /* deal with request body */
    if(strstr(method, "POST")){
        if(content_length != 0){
            for (int i = 0; i < content_length; ++i){
                if(Rio_readnb_w(&rio_client, buf, 1) == 0){
                    continue;
                }
                Rio_writen_w(clientfd, buf, 1);
            }
        }
        else{
            while((read_size = Rio_readlineb_w(&rio_client, buf, MAXLINE)) != 0){
                Rio_writen_w(clientfd, buf, read_size);
                if(strcmp(buf, "\r\n") == 0){
                    break;
                }
            }
        }
    }


    /* Get respond from end server */
    content_length = 0;
    total_size = 0;

    /* rewrite header */
    while(( read_size = Rio_readlineb_w(&rio_server, buf, MAXLINE)) != 0){
        total_size += read_size;
        char *head;
        if((head = strstr(buf, "Content-Length:"))){
            head = head + 16;
            content_length = atoi(head);
        }
        Rio_writen_w(fd, buf, read_size);
        if(strcmp(buf, "\r\n") == 0){
            break;
        }
    }

    /* rewrite body */
    if(content_length != 0){
        for (int i = 0; i < content_length; ++i){
            if(Rio_readnb_w(&rio_server, buf, 1) > 0){
                total_size++;
                Rio_writen_w(fd, buf, 1);
            }
            
        }
    }
    else{
        while((read_size = Rio_readlineb_w(&rio_server, buf, MAXLINE)) != 0){
            Rio_writen_w(fd, buf, read_size);
            total_size += read_size;
            if(strcmp(buf, "\r\n") == 0){
                break;
            }
        }
    }

    /* Output log */
    P(&mutex);
    format_log_entry(buf, &vargp->clientaddr, uri, total_size);
    printf("%s\n", buf);
    V(&mutex);

    Close(clientfd);
}

/*
 * parse_uri - URI parser
 *
 * Given a URI from an HTTP proxy GET request (i.e., a URL), extract
 * the host name, path name, and port.  The memory for hostname and
 * pathname must already be allocated and should be at least MAXLINE
 * bytes. Return -1 if there are any problems.
 */
int parse_uri(char *uri, char *hostname, char *pathname, char *port){
    char *hostbegin;
    char *hostend;
    char *pathbegin;
    int len;

    if (strncasecmp(uri, "http://", 7) != 0) {
        hostname[0] = '\0';
        return -1;
    }

    /* Extract the host name */
    hostbegin = uri + 7;
    hostend = strpbrk(hostbegin, " :/\r\n\0");
    if (hostend == NULL)
        return -1;
    len = hostend - hostbegin;
    strncpy(hostname, hostbegin, len);
    hostname[len] = '\0';

    /* Extract the port number */
    if (*hostend == ':') {
        char *p = hostend + 1;
        while (isdigit(*p))
            *port++ = *p++;
        *port = '\0';
    } else {
        strcpy(port, "80");
    }

    /* Extract the path */
    pathbegin = strchr(hostbegin, '/');
    if (pathbegin == NULL) {
        pathname[0] = '\0';
    }
    else {
        pathbegin++;
        strcpy(pathname, pathbegin);
    }

    return 0;
}

/*
 * format_log_entry - Create a formatted log entry in logstring.
 *
 * The inputs are the socket address of the requesting client
 * (sockaddr), the URI from the request (uri), the number of bytes
 * from the server (size).
 */
void format_log_entry(char *logstring, struct sockaddr_in *sockaddr,
                      char *uri, size_t size)
{
    time_t now;
    char time_str[MAXLINE];
    char host[INET_ADDRSTRLEN];

    /* Get a formatted time string */
    now = time(NULL);
    strftime(time_str, MAXLINE, "%a %d %b %Y %H:%M:%S %Z", localtime(&now));

    if (inet_ntop(AF_INET, &sockaddr->sin_addr, host, sizeof(host)) == NULL)
        unix_error("Convert sockaddr_in to string representation failed\n");

    /* Return the formatted log entry string */
    sprintf(logstring, "%s: %s %s %zu", time_str, host, uri, size);
}
