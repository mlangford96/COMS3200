/* Matthew Langford 43596135  */
/* COMS3200 - Assignment 1 Part C*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>      
#include <unistd.h>

#define requestLimit 512
#define replyLimit 512

int handleConnection(int fd, char *hostname);
int getLine(int fd, char line[], int max);
int outputToFile(char** out);


int main(int argc, char **argv) {
    int sockfd;
    struct sockaddr_in serv_addr;
    struct hostent *serverptr;
    struct in_addr *ptr;
    unsigned short port_number;
    
    // check command line args for hostname and port-number
    if (argc < 2) {
        printf("Usage: ass1 <hostname> <port number>\n");
        return 1;
    }

    port_number = atoi(argv[2]);

    if ((serverptr = (struct hostent *) gethostbyname(argv[1])) == NULL) {
        perror("hostname error");
        return 1;
    }

    ptr = (struct in_addr *) *(serverptr->h_addr_list);
    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = ptr->s_addr;
    serv_addr.sin_port = htons(port_number);
    
    // create socket file descriptor
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("error establishing socket");
        return 1;
    }
    // connect to smtp server
    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("error connecting to server socket");
        return 1;
    }

    handleConnection(sockfd, argv[1]);  
    close(sockfd);  
    return 0;
}

// handles connection with smtp server//
int handleConnection(int sockfd, char *hostname) {
    char **output;
    int i = 0;
    char request[requestLimit+1];
    char reply[replyLimit+1];

    output = (char**) malloc(20*sizeof(char *));
    for(i = 0; i < 20; i++) {
        output[i] = (char *) malloc(512*sizeof(char));    
    }    
    output[0] = "Matthew Langford - 4359613\n";
    getLine(sockfd, reply, replyLimit);
    sprintf(output[1], "RX> %s\n", reply);
    
    //HELO
    printf("sending HELO\n");
    sprintf(request,"HELO %s\r\n", hostname);
    write(sockfd, request, strlen(request));
    getLine(sockfd, reply, replyLimit);
    sprintf(output[2], "TX> %s\n", request);
    sprintf(output[3], "RX> %s\n", reply);   

    //Sender 
    printf("sending MAIL FROM\n");
    sprintf(request, "MAIL FROM: s4359613@student.uq.edu.au\r\n");
    write(sockfd, request, strlen(request));
    getLine(sockfd, reply, replyLimit);
    sprintf(output[4], "TX> %s\n", request);
    sprintf(output[5], "RX> %s\n", reply);
   
    //Reciever 
    printf("sending RCPT TO\n");
    sprintf(request, "RCPT TO: s4359613@student.uq.edu.au\r\n");
    write(sockfd, request, strlen(request));
    getLine(sockfd, reply, replyLimit);
    sprintf(output[6], "TX> %s\n", request);
    sprintf(output[7], "RX> %s\n", reply);   

    //DATA
    sprintf(request, "DATA\r\n");
    write(sockfd, request, strlen(request));
    getLine(sockfd, reply, replyLimit);
    sprintf(output[8], "TX> %s\n", request);
    sprintf(output[9], "RX> %s\n", reply);   
    
    //Message
    sprintf(request, "Subject: Testing For Assignment 1\r\nWelcome to COMS3200\r\n.\r\n");
    write(sockfd, request, strlen(request));
    getLine(sockfd, reply, replyLimit);
    sprintf(output[10], "TX> %s\n", request);
    sprintf(output[11], "RX> %s\n", reply);
    
    //QUIT
    sprintf(request, "QUIT\r\n");
    write(sockfd, request, strlen(request));
    getLine(sockfd, reply, replyLimit);
    sprintf(output[12], "TX> %s\n", request);
    sprintf(output[13], "RX> %s\n", reply);   

    //Output message log to text file
    outputToFile(output);

    return 0;
}

// gets a line from the file descriptor
int getLine(int fd, char* line, int charLim) {
    char c;
    int x = 0;
   
    while (--charLim > 0 && read(fd, &c, 1) > 0 && c != '\n' && c!='\0') {
        line[x++] = c;
    }
    
    if (c=='\n') {
       line[x++] = c;
       line[x] = '\0';
    }

    return x;
}         

//sends smtp command log 2D char array to .txt log file
int outputToFile(char** out) {
    int x;
    FILE *fp = fopen("smtplog.txt", "ab+");

    for(x = 0; x < 14; x++){
        fprintf(fp, "%s", out[x]);
    }

    fclose(fp);
    return 0;
}
