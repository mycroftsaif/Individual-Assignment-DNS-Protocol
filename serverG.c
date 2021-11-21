#include <stdio.h>                          // Standard input and output
#include <sys/socket.h>                     // For socket(), connect(), send(), and recv()
#include <arpa/inet.h>                      // For sockaddr_in and inet_addr()
#include <stdlib.h>                         // For atoi() and exit()
#include <string.h>                         // For memset()
#include <unistd.h>                         // For close()
#include <netdb.h>                          // For gethostbyname()
#include <time.h>                           // For time_t
#define MAXCONNECTION 5                     // Maximum outstanding connection requests
#define RCVBUFSIZE 1000                      // Size of receive buffer


void ErrorMessage(char *errorMessage);      // Error handling function
void TCPClientHandler(int clntSocket, int serverSock, char ip[]);
                                            // TCP client handler function

/*** Structure for Documenting last served client ***/
struct{
    char ipAddess[16];
    time_t timeStamp;
} clientNode;

/*** Structure for Linked List ***/
struct Node{
    char domainName[20];
    int count;
    char ipAdd[65];
    struct Node *next;
}one;                                         // Structure variable "l"

typedef struct Node *nodePointer;           // nodePointer points to a structure of type linkedList

//************  Function Prototype Declarations  ***************//

void readInput(char *, nodePointer *);

char * addDomain(char *, nodePointer *, int);

void displayNode(nodePointer);

void * searchDomain(char *, nodePointer *, int);

void deleteDomain(char *, nodePointer *);

void  nodeToString(nodePointer, int);

char * toString(char str[], int);

void sortDomain(char , nodePointer);

void linkedListToFile(nodePointer);

char * resolveName(char name[]);

//*****************     Global variables   ********************//

nodePointer head = NULL;
char buffer[RCVBUFSIZE];
char fileLocation[200];
char respMessg[100];
char resolveNameMessage[100];
unsigned int  timeOut;
unsigned int oneTimeCount = 0;

//-------------------------------------------------------------//
//*****************     MAIN FUNCTION      ********************//
//-------------------------------------------------------------//

int main(int argc, char *argv[]) {
    
    int serverSock;                         // Socket descriptor for server
    int clientSock;                         // Socket descriptor for client

    struct sockaddr_in serverAddr;          // Structure variable for Local address
    struct sockaddr_in clientAddr;          // Structure variable for Client address
    
    unsigned short serverPort;              // Server port
    unsigned int clientLen;                 // Length of client address data structure
    
    
    char line[1024];                        
    
    printf("\n\n\tDNS Protocol System Server \t\t\n");
                                                   
    printf("\n\t\tRequest Code \t\tprocess"
           "\n\t\t1. \t\t\tFind IP for a domain"
		   "\n\t\t2. \t\t\tDNS Library "
           "\n\t\t3. \t\t\tDelete a record from the list"
           "\n\t\t4. \t\t\tReport the most requested record(s)"
           "\n\t\t5. \t\t\tReport the least requested record(s)"
           "\n\t\t6. \t\t\tShutdown");
    
    
    timeOut = 1;
    
    /* Create socket for incoming connections */
    if ((serverSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        ErrorMessage("socket() failed");
    
    /* Construct local address structure */
    memset(&serverAddr, 0, sizeof(serverAddr));     // Zero out structure
    serverAddr.sin_family = AF_INET;                // Internet address family
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY); // Any incoming interface
    serverAddr.sin_port = htons(8888);        // Local port
    
    /* Bind to the local address */
    if (bind(serverSock, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) < 0)
        ErrorMessage("\nbind failed.......");
    
    /* Mark the socket so it will listen for incoming connections */
    if (listen(serverSock, MAXCONNECTION) < 0)
        ErrorMessage("\nlisten failed.......");
    printf("\n");
    int i =0;
    
    //*****************    Run forever   *****************//
    
    for (;;)
    {
        clientLen = sizeof(clientAddr);             // Set the size of the in-out parameter
        
        /* Wait for a client to connect */
        if ((clientSock = accept(serverSock, (struct sockaddr *) &clientAddr,&clientLen)) < 0)
            ErrorMessage("accept() failed");
        
        /* clientSock is connected to a client! */
        
        while(i==0){
            strcpy(clientNode.ipAddess, inet_ntoa(clientAddr.sin_addr));
            clientNode.timeStamp= time(NULL);
            i++;
        }
        
        TCPClientHandler(clientSock, serverSock,inet_ntoa(clientAddr.sin_addr));     // Calling TCP handler
    }
}



// TCP Function
void TCPClientHandler(int clntSocket, int serverSock, char clientIp[]){
    printf("\n\n\n########################################################################################");
    printf("\n\t\tEntered TCP handler for %s ", clientIp);
    char *word;                             // Temporary word variable to obtain words from string
    int  process;                            // process requested
    char dName[40];                         // Domain name
    time_t now;                             // Time variable
    char sec[5];
    long recvMsgSize;                       // Size of received message
    char *ipPointer; 
    int  statusCode = 0;                    // Client request is outside the timeout window and request will be processed directly
	char line[1024];
    // Receive message from client
    if ((recvMsgSize = recv(clntSocket, buffer, RCVBUFSIZE, 0)) < 0){
        ErrorMessage("recv() failed");}
    
    buffer[recvMsgSize]='\0';               //Terminating the receive buffer with null character
    
    now =time(NULL);                        // Current time
    
    if ((now - clientNode.timeStamp )< 10 && oneTimeCount != 0){
        // Calculates the time elapsed since previous query
        statusCode = 3;                     // Do not process the query, ask Client to wait till the timeout
     }
    else{
        strcpy(clientNode.ipAddess, clientIp);
        clientNode.timeStamp = now;         // Timestamp the current time for a Client
    }
    if( statusCode == 0){                   // Normal operation - Prcoess the query received from the Client
    
    switch(buffer[0]){
        
        //Process 2, 4, 5, 6   
        case '2':            
						//2. DNS Library
                        if (buffer[0] == '2'){
                            printf("\n\n\t\tIncoming request from client  %s : Most requested domain\n", clientIp);
                            printf("\n\t\tDNS Dossier will present at most 3 most requested domains in case of tie b/w counts\n");
							
							FILE *fp = fopen("Domain_Library.txt","r");
    
							if( fp == NULL ){                               
								if ((fp = fopen("Domain_Library.txt","w")) == NULL)
									ErrorMessage("\nFile Fail to open......");
							}
							
							else{
								while(fgets(line,1024,fp)){                 

									readInput(line, &head);
								}
								
								displayNode(head);                          
							}
								
                            break;
                            
                        }
						//4. Most Requested Domain
						else if (buffer[0] == '4'){
                            printf("\n\n\t\tIncoming request from client  %s : Most requested domain\n", clientIp);
                            printf("\n\t\t3 most requested domains:\n");
                            sortDomain('4', head);
                            break;
                            // Function to print top most
                        }
						//5. Least Requested Domain
                        else if (buffer[0] == '5'){
                            printf("\n\n\t\tIncoming request from client  %s : Least requested domain\n", clientIp);
                            printf("\n\t\t 3 Least requested domains:\n");
                            sortDomain('5', head);
                            break;
                            // Function to calculate the number of times a domain was requested the LEAST
                        }
						//6. Shutdown
                        else if (buffer[0] == '6'){
                            printf("\n\n\t\tIncoming request from client  %s : Shutdown DNS Dossier", clientIp);
							linkedListToFile(head);  
							
							printf("\n\n\t\tLinked List written to the File\n");
							strcpy(buffer,"Server is going down. Connection will be lost!!");
                            
                            printf("\n\t\tClosing Client Socket");
							close(clntSocket);
							
							printf("\n\t\tClosing Server listening socket");
							close(serverSock);
							
							exit(0);
                            break;
                            
                        }
                        else{
                            printf("\n\t\tIncorrect request from client....");    
                            strcpy(buffer,"Incorrect request from client");
                            strcat(buffer,clientIp);
                        }
                        break;
        //Process 1 & 3
        case '3':       
						//1. Find IP domain
                        if (buffer[0] == '1'){          
                            //action = (int)buffer[0];
                            word = strtok(buffer,"#");
                            strcpy(dName,word);
                            printf("\n\n\t\tRequest from client  %s : Search IP for domain %s", clientIp, word);
                            if((ipPointer = searchDomain(word, &head, 1)) !=NULL){
                                strcpy(buffer, ipPointer);
                                buffer[strlen(ipPointer)]='\0';
                            }
            
                            else
                                strcpy(buffer,"IP not found");
                            
                            break;
                        }
						//3. Delete Domain
                        else if(buffer[0] == '3'){           
                            word = strtok(buffer,"#");
                            strcpy(dName,word);
                            printf("\n\n\t\tIncoming request from client  %s : Delete domain %s", clientIp, word);
                            deleteDomain(dName, &head);
                            strcpy(buffer,respMessg);
                            break;
                        }
            
                        else {
                                printf("\n\t\tIncorrect request from client....");    
                                strcpy(buffer,"Incorrect request passed by client  ");
                                strcat(buffer,clientIp);
                                break;
                        }
      
        default:    {
                        printf("\n\t\tIncorrect request from client....");    // if command isn't in write format
                        strcpy(buffer,"Incorrect request passed by client ");
                        strcat(buffer,clientIp);
                    }
            
    }
        
    
    while (recvMsgSize > 0)      
    {
        /* Echo message back to client */
        if (send(clntSocket, buffer, RCVBUFSIZE, 0) != RCVBUFSIZE)
            ErrorMessage("send() failed");
        
        /* See if there is more data to receive */
        if ((recvMsgSize = recv(clntSocket, buffer, RCVBUFSIZE, 0)) < 0)
            ErrorMessage("recv() failed");
    }
    printf("\n\t\tSent Response to the Client: %s", buffer);
    printf("\n\n########################################################################################");
  
  }// end of if loop with STATUS code 0

    
   // Do not process the query, ask Client to wait till the timeout
    
    if(statusCode == 3){
        strcpy(buffer,"Another inquiry had been made ");
        strcat(buffer,toString(sec, (int) (now - clientNode.timeStamp)));
        strcat(buffer," seconds ago, wait ");
        strcat(buffer,toString(sec,timeOut));
        strcat(buffer," seconds before another submission" );
        if (send(clntSocket, buffer, RCVBUFSIZE, 0) != RCVBUFSIZE)
            ErrorMessage("send() failed");
        printf("\n\n\t\tSent Response to the Client: %s", buffer);
        printf("\n\n########################################################################################");

    }
    
    
    oneTimeCount = 1;
    close(clntSocket);    /* Close client socket */
}


//Display Node

void displayNode(nodePointer first){
    while(first != NULL){
        printf("\n\t\t%s %d %s",first->domainName,first->count,first->ipAdd);
        first=first->next;
    }
}


//Sorting Domain
void sortDomain(char type, nodePointer first){

    nodePointer nextNode = first;
    nodePointer maxNode = first;        
    nodePointer minNode = first;
    int count = 0;                           

    switch(type){
		// Most requested 
        case '4':                           
                    while(nextNode!=NULL){
                        
                        if(nextNode->count > maxNode->count){
                            maxNode = nextNode;
                        }
                        nextNode = nextNode->next;
                    }
            
                    nextNode = first;
                    while(nextNode != NULL){
                        
                        if((nextNode->count == maxNode->count) && count <3){
                            nodeToString(nextNode, count);
                            count++;
                        }
                        nextNode = nextNode->next;
                    }
            
            
                    break;
        // Least requested      
        case '5' :                          
                    while(nextNode!=NULL){
                        // Searching through the Linked List till the end
                        // Check for finding least of two values
                
                        if(nextNode->count < minNode->count){
                            minNode = nextNode;
                        }
                        nextNode = nextNode->next;
                    }

                    nextNode = first;
                    while(nextNode != NULL)
                    {
                        if((nextNode->count == minNode->count) && count <3){
                            nodeToString(nextNode, count);
                            count++;
                        }
                        nextNode = nextNode->next;
                    }
            
                    break;
            
        default:        break;
    }
}



//Read textfile into linked list

void readInput(char ch[], nodePointer *first){
    char *word;                                     
    
    nodePointer current = malloc(sizeof(one));        
    
    if((*first) == NULL){                           
        *first = current;
        (*first)->next = NULL;
    }
    else{                                           
        (*current).next = *first;
        *first = current;
    }
    
    

    word = strtok(ch," ");
    strcpy((*first)->domainName, word);
    
    word = (strtok(NULL," "));
    (*first)->count = atoi(word);
    
    word = (strtok(NULL,"\n"));
    strcpy((*first)->ipAdd, word);

    
}


//search  Domain Name
void *searchDomain(char *dName, nodePointer *first, int type){
    
    char temp[100];
    int check=0;                                 // check=0/1, Tells if a domain is found or not
    nodePointer current = *first;
    
    while(current!=NULL){                           // Search for domain till the end of the linked list
        
        if(strcmp((current)->domainName,dName)==0){ // Compare the domain name received to those existing in database
            printf("\n\n\t\tMatch Found");
            printf("\t\t%s %d %s",(current)->domainName,(current)->count,(current)->ipAdd);
            check = 1;                           // Set check = 1, if domain is found
            break;
        }
        current=current->next;
    }
    
    
    if (check == 0 && type == 1){                // If IP for the domain is NOT found
        printf("\n\n\t\tDomain record not found, trying gethostbyname() : ");
        
        if (resolveName(dName)!=NULL){
            strcpy(resolveNameMessage,resolveName(dName));
            /* Search to find its IP and add if found */
            strcpy(temp, dName);
            strcat(temp, " ");
            strcat(temp, resolveNameMessage);
            
            if(*first != NULL)              // Add domain
                addDomain(temp,&head,1);
            else
                addDomain(temp,&head,2);
            
            return (char *) resolveNameMessage;
        }
        else
            return NULL;
    }
    
    else if (check == 1 && type==1){         // If domain match found and process requested is type 1
            current->count += 1;
            return (char *) current->ipAdd;    // Return the pointer to the node found
    }
    
    else if (check == 1 && type == 2){       // If Domain match found and process requested is of type 2
            return (nodePointer) current;       // Return the pointer of the searched node
        }
    
    else
        return NULL;                        // No match found
}



char * addDomain(char *domainIp, nodePointer *first, int type){
    char *word;
    char domainPart[40];
    char ipPart[18];
    
    /* Split the string to obtain domain name and IP */
    word = strtok(domainIp," ");
    strcpy(domainPart, word);
    word = strtok(NULL, "\0");
    strcpy(ipPart,word);
    
    nodePointer nodeTOBeAdded = malloc(sizeof(one)); // Allocating memory for the current pointer to the structure
    nodePointer samePointer;
    
   
  
    if (*first !=NULL && (samePointer= searchDomain(domainPart, first, 2))!= NULL && type == 2){
        printf("\n\n\t\tDomain already exist in the linked list");
        printf("\n\t\tChecking Room for IP Address");
        
        char *tempComparator;
        char oldIpPart[64];
        strcpy(oldIpPart, samePointer->ipAdd);
        
          // if library contain 1 ip
        if (strlen(samePointer->ipAdd)< 18 && strlen(ipPart)< 18 && strlen(samePointer->ipAdd) >6){
      
             if( strcmp(samePointer->ipAdd,ipPart)!=0){
                  strcat(samePointer->ipAdd, " ");
                  strcat(samePointer->ipAdd, ipPart);
                  printf("\n\t\tNew IP add for %s  ",samePointer->domainName);
                  strcpy(respMessg,"New IP add for  ");
                  strcat(respMessg,samePointer->domainName);
             }
             else{ // Record already exists
                  printf("\n\t\tIP Address added already exists  %s ",samePointer->domainName);
                  strcpy(respMessg,"The record to be added already exists ");
                  strcat(respMessg,samePointer->domainName);
             }
        }
        
        // If the record contains 2 IP addresses for the domain to be added
        else if (strlen(samePointer->ipAdd)< 34 && strlen(ipPart)< 18 && strlen(samePointer->ipAdd)>14){
            
            tempComparator = strtok(oldIpPart," ");
            
            if( strcmp(tempComparator,ipPart)!=0){
                tempComparator = strtok(NULL,"\0");
                
                if( strcmp(tempComparator,ipPart)!=0){
                   
                    strcat(samePointer->ipAdd, " ");
                    strcat(samePointer->ipAdd, ipPart);
                    printf("\n\t\tNew IP address add for %s %s",samePointer->domainName, samePointer->ipAdd);
                    strcpy(respMessg,"New IP address add for ");
                    strcat(respMessg,samePointer->domainName);
                }// inner if ends here
                else{
                    printf("\n\t\tThe IP address to be add already exists %s ",samePointer->domainName);
                    strcpy(respMessg,"The IP address to be add already exists");
                    strcat(respMessg,samePointer->domainName);
                }
            }// outer if ends here
            else{
                printf("\n\t\tThe IP address to be add already exists %s ",samePointer->domainName);
                strcpy(respMessg,"The IP address to be add already exists ");
                strcat(respMessg,samePointer->domainName);
            }
        }// else-if ends here
        
        // If the record contains 3 IP addresses for the domain to be added
        else if (strlen(samePointer->ipAdd)< 50 && strlen(ipPart)< 18 && strlen(samePointer->ipAdd)>22){
            
            tempComparator = strtok(oldIpPart," ");
            if( strcmp(tempComparator,ipPart)!=0){
                
                tempComparator = strtok(NULL," ");
                if( strcmp(tempComparator,ipPart)!=0){
                    
                     tempComparator = strtok(NULL,"\0");
                     if( strcmp(tempComparator,ipPart)!=0){
                         
                         strcat(samePointer->ipAdd, " ");
                         strcat(samePointer->ipAdd, ipPart);
                         printf("\n\t\tNew IP address add for %s ",samePointer->domainName);
                         strcpy(respMessg,"New IP address add for ");
                         strcat(respMessg,samePointer->domainName);
                     }// inner if ends here
                     else{
                        printf("\n\n\t\tThe IP address to be add already exists %s  ",samePointer->domainName);
                        strcpy(respMessg,"The IP address to be add already exists ");
                        strcat(respMessg,samePointer->domainName);
                     }
                }// second-inner most if ends here
                else{
                    printf("\n\n\t\tThe IP address to be added already exists %s  ",samePointer->domainName);
                    strcpy(respMessg,"The IP address to be added already exists");
                    strcat(respMessg,samePointer->domainName);
                }
            }// outer if ends here
            else{
                printf("\n\n\t\tThe IP addresses to be add already exists%s  ",samePointer->domainName);
                strcpy(respMessg,"The IP addresses to be add already exists ");
                strcat(respMessg,samePointer->domainName);
            }
        }// else-if ends here
        
        else{
            printf("\n\n\t\tNo space for new IP ");
            strcpy(respMessg,"No space for new IP");
            strcat(respMessg,samePointer->domainName);

            }
        }
    
    /* If Domain to be added DOES not exist in the list, then ADD */
   else {
        printf("\n\n\t\tNew Domain add  :%s \n",domainPart);
        strcpy(nodeTOBeAdded->domainName, domainPart);
        nodeTOBeAdded->count= 0;
        strcpy(nodeTOBeAdded->ipAdd, ipPart);
        nodeTOBeAdded->next = (*first);
        (*first) = nodeTOBeAdded;
        if (type ==2){
            strcpy(respMessg,"New IP add for ");
            strcat(respMessg,nodeTOBeAdded->domainName);
        }
    }
    return respMessg;
}



//covert node to string 
void nodeToString(nodePointer node, int n){
    char *content = node->domainName;
    char nodeCount[8];
    char *count = toString(nodeCount,node->count);     // Converting the Integer variable, 'count' to string
    
    /* Creating a string to be sent as a reply to the Client */
    if( n == 0){
        strcpy(buffer, content);
        strcat(buffer, " ");
        strcat(buffer,count);
    }
    else{
        strcat(buffer, content);
        strcat(buffer, " ");
        strcat(buffer,count);
    }
   
    strcat(buffer, " ");
    
}



// convert int to string
char *toString(char * str, int num){
    int i, r, len = 0, n;
    
    if(num >0){
        n = num;
        while (n != 0)      // Count the number of digits
        {   len++;
            n /= 10;
        }
        for (i = 0; i < len; i++)
        {   r = num % 10; // Convert each digit to char and store in the char array
            num = num / 10;
            str[len - (i + 1)] = r +'0' ;
        }
        str[len] = '\0';
    }
    else
        return "0";
    
    return str;
}



//Linked list to Textfile
void linkedListToFile(nodePointer first){
    
    FILE *fp;
    char line[100];
    char str[8];
    printf("\n\n\t\tLocation is :DomainLibrary.txt");
    
    /* Open the file to write */
    if( (fp = fopen("DomainLibrary.txt", "w") ) == NULL )
        ErrorMessage("\nError opening file to save linked list");
    
    else{
        while(first != NULL){          // Add all the nodes in the linked list
            strcpy (line, first->domainName);
            strcat (line, " ");
            strcat (line, toString(str,first->count));
            strcat (line, " ");
            strcat (line, first->ipAdd);
            strcat (line,"\n");
            fprintf(fp,"%s",line);
            first = first->next;        // Incrementing to next node
        }
        
        fclose(fp);                     // CLOSE FILE
    }

}



//Delete Domain
void deleteDomain(char *dName, nodePointer *first)
{
    
    nodePointer prev, current;
    int check =0;                // check = 1, record found | check = 0, recod NOT found
    
    prev = (*first);
    current = (*first);
    //Last node needs to be delete
    if(current->next == NULL && strcmp(dName,current->domainName)== 0){
        
            *first = NULL;
            strcpy(respMessg, current->domainName);
            strcat(respMessg, " deleted from the linked list");
            check = 1;
        }

    //Head node needs to be delete
    else if(current->next != NULL && strcmp(dName,current->domainName)== 0){
   
        *first = current->next;
        strcpy(respMessg, current->domainName);
        strcat(respMessg, " deleted from the linked list");
        check = 1;
    }
    //Node in between needs to be deleted
    else{   
            current = current->next;
            while(current != NULL){
                if(strcmp(dName,current->domainName) == 0){
                    prev->next = current ->next;
                    strcpy(respMessg, current->domainName);
                    strcat(respMessg, " deleted from the linked list");
                    check = 1;
                }
                
                prev = current;
                current = current->next;
                
            }
    }
    if (check ==0){
        strcpy(respMessg, dName);
        strcat(respMessg, "doesn't exist in the record database");
    
    }
    
    printf("\n\n\t\tNew Head: %s %s\n",(*first)->domainName, (*first)->ipAdd);
    
}


//find domain name
char * resolveName(char name[])
{
    struct hostent *hp = gethostbyname(name);
    
	// gethostbyname() unable to find the IP
    if (hp == NULL) {                                  
        printf("gethostbyname() failed\n");
        return NULL;
    }
	// gethostbyname() returns an IP address
    else {
        printf(" Domain Name Resolved");                     
        unsigned int i=0;
		
        // Add to domain library
        while ( hp -> h_addr_list[i] != NULL) {        
            if (i == 0){
                strcpy(respMessg, inet_ntoa( *( struct in_addr*)( hp -> h_addr_list[i])));
                strcat(respMessg, " ");
            }
            
            i++;
        }
        return respMessg;
    }
    
}

//Error Message
void ErrorMessage(char *errorMessage)
{
    perror(errorMessage);
    exit(1);
}