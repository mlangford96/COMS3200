#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdbool.h>

#define requestLimit 128
#define replyLimit 1024

int error(int errorCode);
int getLine(int fd, char* line, int charLim); 
int handleConnection(int sockfd, char *hostname, char *fileName, bool filePathBool, char* filePath);
int openDataConnection(char* address);

int main(int argc, char** argv) {
    char*               host;
    char*               fileName;    
    int                 sockfd;
    int                 i, j;
    struct sockaddr_in  serv_addr;
    struct hostent      *serverptr;
    struct in_addr      *ptr;
    unsigned short      port_number;
    char* filePath;    
    char* str;
    char* str2;

    if(argc < 2) {
        error(0);
    }
    host = malloc(strlen(argv[1]) * sizeof(char));
    if(strncmp(argv[1], "ftp://", 6) == 0) {
        for(i = 0; i < strlen(argv[1]); i++) {
            if(argv[1][i+6] != '/') {
                sprintf(host, "%s%c", host, argv[1][i+6]);
            } else {
                host[i] = '\0';
                break;
            }
        }
    } else {
        error(1);
    }

    bool filePathBool = false;
    filePath = "";
    str = malloc(strlen(argv[1]) * sizeof(char));
    for(j = 0; j < strlen(argv[1]); j++){
        str[j] = argv[1][j+7+strlen(host)];
        if(str[j] == '/') {
            filePathBool = true; 
        }
        
    }
     
    fileName = malloc(strlen(str) * sizeof(char));
    if(!filePathBool) {
        strcpy(fileName, str);    
    } else {
        char t[2] = "/";
        char *token;
        filePath = malloc(strlen(str) * sizeof(char));
        strcpy(filePath, str);
        token = strtok(str, t);
        while(token != NULL) {
            str2 = malloc(strlen(token) * sizeof(char));
            str2 = token;            
            token = strtok(NULL, t);
       
        }
        i = 0;
        j = strlen(str2); 
        filePath[strlen(filePath) - j] = '\0';
        while(j > 0) {
            fileName[i] = filePath[strlen(filePath) - j];
            i++;
            j--;
        }
        filePath[strlen(filePath)-1] = '\0';
        fileName = str2;
    }
    
    port_number = 21;

    if((serverptr = (struct hostent *) gethostbyname(host)) == NULL) {
        error(2);    
    }
    
    ptr = (struct in_addr *) *(serverptr->h_addr_list);
    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = ptr->s_addr;
    serv_addr.sin_port = htons(port_number);

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        error(3);
    }

    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        error(4);
    }

   
    printf("connected to server\n");
    handleConnection(sockfd, "ftp.uq.edu.au", fileName, filePathBool, filePath);
    close(sockfd);

return 0;
}

int error(int errorCode) {

    switch(errorCode) {
    case 0 :
        fprintf(stdout, "Usage: webget PROTOCOL://HOST/FILENAME\n");
        exit(0);
        break;
    case 1 :
        fprintf(stdout, "invalid protocol - use ftp\n"); 
        exit(1);
        break;
    case 2 :
        fprintf(stdout, "invalid hostname\n");
        exit(2);
        break;
    case 3:
        fprintf(stdout, "error establishing socket\n");
        exit(3);
    case 4:
        fprintf(stdout, "error connecting to server socket\n");
        exit(4);
    case 5:
        fprintf(stdout, "requested directory does not exist\n");
        exit(5);
    case 6:
        fprintf(stdout, "requested file does not exist\n");
        exit(6);
    case 7:
        fprintf(stdout, "error establishing data socket\n");
        exit(7);
    case 8:
        fprintf(stdout, "error connecting to data socket\n");
        exit(8);
    default : 
    exit(99);
    }

return 0;
}

int handleConnection(int sockfd, char *hostname, char *fileName, bool filePathBool, char* filePath) {
    char request[requestLimit+1];
    char reply[replyLimit+1];
    int flags = fcntl(sockfd, F_GETFL, 0);
    
    fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
    getLine(sockfd, reply, replyLimit); 
    //send user
    printf("sending username\n");
    sprintf(request, "USER anonymous\r\n");
    write(sockfd, request, strlen(request)); 
    getLine(sockfd, reply, replyLimit);
    //send pass
    printf("sending password\n");
    sprintf(request, "PASS -coms3200@uq.edu.au\r\n");
    write(sockfd, request, strlen(request));
    getLine(sockfd, reply, replyLimit);
    //CWD
    if(filePathBool == true) {
    printf("changing working directory\n");
    sprintf(request, "CWD %s\r\n", filePath);
    write(sockfd, request, strlen(request));
    getLine(sockfd, reply, replyLimit);
    if(reply[0] != '2') {
        error(5);
        }
    }    
    //PASV
    printf("setting transfer mode\n");
    sprintf(request, "PASV\r\n");
    write(sockfd, request, strlen(request));
    getLine(sockfd, reply, replyLimit);
    
    //open data connection
    int datafd = openDataConnection(reply);   

    //RETR
    printf("retrieving file\n");
    sprintf(request, "RETR %s\r\n", fileName);
    write(sockfd, request, strlen(request));
    getLine(sockfd, reply, replyLimit);
    if(reply[0] != '1') {
        error(6);
    }
   
    //get data and read into local file
    getLine(datafd, reply, replyLimit);    
    FILE *fp = fopen(fileName, "ab+");
    fprintf(fp, "%s", reply);
    fclose(fp);
    close(datafd);
    printf("file saved -exiting\n");
    return 0;
}


int getLine(int fd, char* line, int charLim) {
    int count;
    sleep(2);
    
    count = read(fd, line, 1024);
    if(count < 0) {
        return 1;
    } else {
        line[count++] = '\0';
        return 0;
    }
    return 0;
}         

int openDataConnection(char* address){
    char        str[1024];
    char        search[2] =  "(";
    int         x = 0, y = 0, port1, port2;
    char        *token;
    char        ip_str[20];
    char        port_str[20];
    int         port_num;
    struct      sockaddr_in sa;

    strcpy(str, address);
    token = strtok(str,search);
    token = strtok(NULL, search); 
    
    while(token[x] != '\0') {
        if(token[x] == ',') {
            token[x] = '.';
        } else if( token[x] == ')') {
            token[x] = '\0';
        }
        x++;
    }
   
    x = 0;
    while(x < 4) {
        if(token[y] == '.') {
            x++;
            if(x == 4) {
                continue;
            }
        }
        ip_str[y] = token[y];      
        y++;
    }
    x = strlen(ip_str);
    y = 0;

    while(token[x] != '\0') {
        port_str[y] = token[x];
        if(token[x] == '.') {
            port_str[y] = ' ';
        }
        y++;
        x++;
    }
   
    memset(token, 0, strlen(token));    
    strcpy(search, " ");
    token = strtok(port_str, search);
    port1 = atoi(token);
    token = strtok(NULL, search);
    port2 = atoi(token); 
   
    port_num = ((port1 * 256) + port2);
    

    inet_pton(AF_INET, ip_str, &(sa.sin_addr));
    
    int datafd;
    struct sockaddr_in  serv_addr;       
    struct hostent      *serverptr;
    struct in_addr      *ptr;

    if((serverptr = (struct hostent *) gethostbyname(ip_str)) == NULL) {
        error(2);    
    }
    
    ptr = (struct in_addr *) *(serverptr->h_addr_list);
    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = ptr->s_addr;
    serv_addr.sin_port = htons(port_num);

    if ((datafd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        error(7);
    }

    if (connect(datafd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        error(8);
    }
    printf("data connection open\n");
    return datafd;
    
}
