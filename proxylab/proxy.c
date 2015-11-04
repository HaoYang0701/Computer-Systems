//Hao Yang andrewid: haoyang

#include <stdio.h>
#include "csapp.h"
/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400
#define DEF_PORT 80

/* You won't lose style points for including these long lines in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";
static const char *accept_hdr = "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n";
static const char *accept_encoding_hdr = "Accept-Encoding: gzip, deflate\r\n";

static const char *closeconnection =  "Connection: close\r\n";
static const char *closeprox = "Proxy-Connection: close\r\n";


void doit(int fd);
void read_requesthdrs(rio_t *rp);
void clienterror(int fd, char *cause, char *errnum,
                         char *shortmsg, char *longmsg);

void *thread(void *vargp);




// This is the struct for the cache
//consistiing of tags,valid and block
//which are made into lines and sets
typedef struct{
  int valid;
  int tag;
  int block;
}cline;

typedef struct{
  cline *lines;
    }cset;

typedef struct{
  cset *sets;
}ccache;


//signal handler
void sigchld_handler(int sig){
  while(waitpid(-1,0,WNOHANG)>0){

  };
  return;
}

//initializes the cache
void kash(){
  ccache *cache = (ccache*)malloc(sizeof(ccache));
  cache->sets = (cset*)malloc(sizeof(cset)*20);
  int i,j;
  for(i= 0; i < 20; i++){
    cache->sets[i].lines = malloc(sizeof(cline)*10);
    cset citers = cache->sets[i];
    for(j = 0;j<10;j++){
      citers.lines[j].valid = 0;
      citers.lines[j].tag = MAXLINE;
      citers.lines[j].block = MAXLINE;
    }
    //NVM LOL
}

}

//parses the uri and grabs the port if there is one
//by findig the : character and reading ints after
//that char to /
int parse_uri(char *uri, char *filename, int *port)
{
    char *str;
    char *end;
    str = calloc(MAXLINE,MAXLINE*sizeof(uri));

    strncpy(str, uri, MAXLINE);
    end = uri + strlen(uri);
    str += 7;
    while (str < end) {//run until buf is over
      if (*str == ':') {
          *str = '\0';
          str++;// if there are numbers
          if((&end-&str) != 0){
          sscanf(str, "%d%s",port,str);
          }}
      //check that there isnt more numbers
      if (*str != '/' && *str != ':') {
        sprintf(filename,"%s%c%c", filename, *str,'\0');}
      else {
            strncpy(uri, str,MAXLINE);
            str = end;
        }
      str++;//increments the char we are looking at
    }
    return 0;

}


//main function for sending requests and connecting
int main(int argc, char **argv)
{

  int listenfd,clientlen; //init all valyes
    int *connfd;

    char hostname[MAXLINE];
    char port[MAXLINE];
    struct sockaddr_in clientaddr;
    pthread_t tid; // for conccurrent operations


    if (argc != 2) {
            fprintf(stderr, "usage: %s <port>\n", argv[0]);
            exit(1);
    }
    kash();

    signal(SIGCLD,sigchld_handler);
    signal(SIGPIPE, SIG_IGN); // block signals
    listenfd = Open_listenfd(argv[1]);//takes in a port to listen to
    while (1) {
            clientlen = sizeof(clientaddr);
            connfd = malloc(sizeof(int));
            *connfd = Accept(listenfd, (SA *)&clientaddr,
                             (socklen_t*) &clientlen);
            Getnameinfo((SA *) &clientaddr, clientlen, hostname, MAXLINE,
                    port, MAXLINE, 0);
            printf("Accepted connection from (%s, %s)\n", hostname, port);


            pthread_create(&tid, NULL, thread, connfd); // create thread
            // pthread_join(tid,NULL);
    }
    close(*connfd);
    return 0;
}


//function in which requests are made to server/ website
void doit(int fd)
{
  int serverfd,is_static; //intialize vairbales
  int *port = calloc(MAXLINE,MAXLINE*sizeof(int));
  char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];

  char *filename= malloc(MAXLINE*sizeof(char));
  char *hostname = malloc(MAXLINE*sizeof(char));

  char *request = calloc(MAXLINE,MAXLINE*sizeof(char));
  char srcp[MAXLINE];
  rio_t rio;
  int rio_ser;

  *port = 80;

  if(fd < 0){//error check for valid file desc
    return;
  }
  Rio_readinitb((rio_t*)&rio,fd);
  Rio_readlineb(&rio, buf, MAXLINE);

  if(&rio == NULL){
    return;
  }
  sscanf(buf, "%s %s %s\r\n", method, uri, version); // gets method etc
  if (strcasecmp(method, "GET")) {
        clienterror(fd, method, "501", "Not Implemented",
        "proxy does not implement this method");
        return;
    }
  else{
    read_requesthdrs(&rio);
    is_static = parse_uri(uri, hostname, port);

    if(is_static){ // check to see if static page or not
       clienterror(fd, filename, "403", "Forbidden",
                        "proxy couldn't read the file");
            return;
    }
    sprintf(request, "%s %s %s\r\n", method, uri, version);
    }
  if(hostname != NULL && request != NULL){ // request not empty

    char* temp = malloc(MAXLINE*sizeof(char));
    if(request == "Host"){ // appends varaible and host to temp
      strcat(temp, "Host: ");//which will then be appended to request
     strcat(temp, hostname);
     strcat(temp, "\r\n");
     }

     if(temp!= NULL){//check to see if temp is empty
     strcat(request,temp);
     strcat(request, accept_hdr);//add all the connection requests

     strcat(request, accept_encoding_hdr);

     strcat(request, user_agent_hdr);

     strcat(request, closeprox);
     strcat(request, closeconnection);
     strcat(request, "\r\n");} // end string
     free(temp); // free what u malloc
     }

  sprintf(port, "%d", *port);
        if((serverfd = open_clientfd(hostname,(char*) port)) < 0)
          {//no connection
            return;
        }
        //int srcfd;

        Rio_readinitb((rio_t*)&rio_ser, serverfd);

        Rio_writen(serverfd, request, MAXLINE);
        // srcfd = Open(request,O_RDONLY,0);
        //srcp2 = Mmap(0,MAXLINE,PROT_READ,MAP_PRIVATE,srcfd,0);
        while(!(rio_readnb((rio_t*)&rio_ser, srcp,
                           sizeof(srcp))) <= 0){
          Rio_writen(fd, srcp, MAXLINE);
            close(&rio_ser);

        }
        //free(request);
}
/* $end doit */






//thread function that allows concurrent running
void *thread(void *vargp)
{
    int connfd = *((int *)vargp);
    pthread_detach(pthread_self());
    free(vargp);
    doit(connfd);
    close(connfd);
    return NULL;
}

//reads the request header
void read_requesthdrs(rio_t *rp)
{
    char buf[MAXLINE];

    Rio_readlineb(rp, buf, MAXLINE);
    printf("%s", buf);
    while(strcmp(buf, "\r\n")) {
        Rio_readlineb(rp, buf, MAXLINE);
        printf("%s", buf);
    }
    return;
}




//client erorr function thtat prints out errors to the websites/server
void clienterror(int fd, char *cause, char *errnum,
                 char *shortmsg, char *longmsg)
{
    char buf[MAXLINE], body[MAXBUF];


    sprintf(body, "<html><title>Tiny Error</title>");
    sprintf(body, "%s<body bgcolor=""ffffff"">\r\n", body);
    sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
    sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
    sprintf(body, "%s<hr><em>The Tiny Web server</em>\r\n", body);

    sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-type: text/html\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-length: %d\r\n\r\n", (int)strlen(body));
    Rio_writen(fd, buf, strlen(buf));
    Rio_writen(fd, body, strlen(body));
}
