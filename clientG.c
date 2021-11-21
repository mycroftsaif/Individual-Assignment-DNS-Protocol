#include <stdio.h>      // for printf() and fprintf()
#include <sys/socket.h> // for socket(), connect(), send(), and recv()
#include <arpa/inet.h>  // for sockaddr_in and inet_addr()
#include <stdlib.h>     // for atoi() and exit()
#include <string.h>     // for memset()
#include <unistd.h>     // for close()
#include <stdbool.h>    // for ip check
#include <ctype.h>      // for tolower()


/* Function to validate the IP Address entered by Client */
bool isValidIpAddress(char *ipAddress)
 {
 struct sockaddr_in sa;
 int result = inet_pton(AF_INET, ipAddress, &(sa.sin_addr));
 return result != 0;
 }


#define RCVBUFSIZE 1000             // Size of receive buffer

void errorMessage(char *errorMessage);  /* Error handling function */
char * toString(char str[], int num);



//MAIN FUNCTION   
int main(int argc, char *argv[]){
    int sock;                           // Socket descriptor
    struct sockaddr_in serverAddr;      // Echo server address
    char echoServer[100];               // String to send to echo server
    char echoBuffer[RCVBUFSIZE];        // Buffer for echo string
    unsigned long echoServerLen;        // Length of string to echo
    long bytesRcvd, totalBytesRcvd;     // Bytes read in single recv() and total bytes read
    char *process;                      
    char *domainName;                  
    char *ipToAdd;
    char str[2];                        //= argc;
    
    
    
    printf("\n\n\tDNS Protocol System Client \t\t\n");
                                                   
    printf("\n\t\tRequest Code \t\tprocess"
           "\n\t\t1. \t\t\tFind IP for a domain"
		   "\n\t\t2. \t\t\tDNS Library "
           "\n\t\t3. \t\t\tDelete a record from the list"
           "\n\t\t4. \t\t\tReport the most requested record(s)"
           "\n\t\t5. \t\t\tReport the least requested record(s)"
           "\n\t\t6. \t\t\tShutdown");
		   
	 if ((argc < 1) || (argc > 3))       // Test for correct number of arguments
    {
        printf("\n\t\tNo of command line parametes aren't enough and proper for the request");
        exit(1);
    }
    
    process = argv[1];
    
    strcpy(echoServer,toString(str,argc));
    strcat(echoServer, "#");            // Formatting the string to be sent with "#" in between args
    strcat(echoServer, process);
    strcat(echoServer, "#");            // Check if valid process code is entered
    
    if (atoi(process)> 6 || atoi(process) <0)
        errorMessage("Invalid request code entered by the client");
    
    switch (argc){                      // Check for the number of args entered by the Client
        case 3:     domainName = argv[2];
                    strcat(echoServer,domainName);
                    strcat(echoServer,"#");
                    printf("\nCommand Sent: %s %s",argv[1],argv[2]);
                    break;
 
        
        default:    printf("\nCommand Sent: %s ",argv[1]);
                    break;
            
    }
	
	
    
	
    /* Create a reliable, stream socket using TCP */
    if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        errorMessage("socket() failed");
    
    /* Construct the server address structure */
    memset(&serverAddr, 0, sizeof(serverAddr));         
    serverAddr.sin_family      = AF_INET;              
    serverAddr.sin_addr.s_addr = inet_addr("192.168.236.3");     
    serverAddr.sin_port        = htons(8888);     
    
    /* Establish the connection to the echo server */
    if (connect(sock, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) < 0)
        errorMessage("connect() failed");
    
    echoServerLen = strlen(echoServer);                 // Determine input length
    
   

    /* Send the string to the server */
    if (send(sock, echoServer, echoServerLen, 0) != echoServerLen)
        errorMessage("send() sent a different number of bytes than expected");
    
    /* Receive the same string back from the server */
    totalBytesRcvd = 0;
    printf("\nReceived: ");                             // Setup to print the echoed string */
    
    /* Receive up to the buffer size (minus 1 to leave space for
     a null terminator) bytes from the sender */
    while (totalBytesRcvd < RCVBUFSIZE)
    {
        /* Receive up to the buffer size (minus 1 to leave space for
         a null terminator) bytes from the sender */
        if ((bytesRcvd = recv(sock, echoBuffer, RCVBUFSIZE - 1, 0)) <= 0)
            errorMessage("recv() failed or connection closed prematurely");
        
        totalBytesRcvd += bytesRcvd;                // Keep tally of total bytes
        echoBuffer[bytesRcvd] = '\0';               // Terminate the string!
        printf("%s", echoBuffer);                   // Print the echo buffer
    }
    
    printf("%s", echoBuffer);                       // Print the echo buffer
    printf("\n");                                   // Print a final linefeed
    
    close(sock);                                    // CLOSE SOCKET
    exit(0);
}

// Die with error - Error handling function
void errorMessage(char *errorMessage)
{
    perror(errorMessage);
    exit(1);
}

char * toString(char * str, int num){
    int i, rem, len = 0, n;
    
    n = num;
    while (n != 0)              // Iterates over to count the number of digits
    {
        len++;
        n /= 10;
    }
    for (i = 0; i < len; i++)   // Converts each digit into a character in the string
    {
        rem = num % 10;
        num = num / 10;
        str[len - (i + 1)] = rem +'0' ;
    }
    str[len] = '\0';
    
    return str;                 // return the integer converted to String
}