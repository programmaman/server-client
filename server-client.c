//Alec Braynen

#include <sys/types.h>
#include <semaphore.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#define LISTEN_PORT 1050
#define IP_ADDR "131.247.3.8"
#define NUMBER_OF_THREADS 3

char sharedMemoryBuffer[15];
sem_t csMutex, runningNumberMutex;


void *messageHandlingThread(void *arg)
{
    int rec; //Variable to Store Recv Returns
    int tsocket = *((int *)arg); //Assign received socket to thread local variable
    
    sem_wait(&csMutex);
    memset(sharedMemoryBuffer, 0, sizeof(sharedMemoryBuffer)); //Ensure Buffer has no remnants or garbage in it
    printf("Attempting to receive message. \n");
    rec = recv(tsocket, sharedMemoryBuffer, 15, 0); //I believe recv is blocking, so I didn't use a while loop. Feel free to subtract points here if false
    printf("Message Received. It was: %s \n", sharedMemoryBuffer);
    sleep(2);
    send(tsocket, sharedMemoryBuffer, 15, 0); //Send message back to client
    sem_post(&csMutex);

    close(tsocket);
}

int main(int argc, char **argv)
{

    /*Threading Variables*/
    int numberOfRanThreads = 0;
    pthread_t threads[NUMBER_OF_THREADS];
    pthread_attr_t attr[1];
    sem_init(&csMutex, 0, 1);
    sem_init(&runningNumberMutex, 0, 1);
    pthread_attr_init(&attr[0]);
    pthread_attr_setscope(&attr[0], PTHREAD_SCOPE_SYSTEM);

    /*Socket Variables*/
    struct sockaddr_in server, client;
    int socketfd, newsocketfd, socketLength, threadsocket;
    int on = 1;

    /*Configure Socket, Port, IP, Protocol*/
    socketfd = socket(AF_INET, SOCK_STREAM, 0);
    server.sin_port = htons(LISTEN_PORT);
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;

    /*Set Socket Behavior [Tell OS to Release Socket Immediately]*/
    if (setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on)) < 0)
    {
        perror("Stream Socket option set failed.");
        exit(1);
        /* some error has occurred, etc */
    }

    /*Bind Socket to the DEFINED PORT*/
    if(bind(socketfd, (struct sockaddr *)&server, sizeof(server)) < 0){
        perror("Error binding to Port.");
    }


    socketLength = sizeof(client); //Socket Size for Creating Additional Socets with Accept

    listen(socketfd, 3); //Listen, allowing up to 3 backlogged connections 

    printf("Listening for Clients. \nPress Control C to Exit Server. \n"); //Statement to help debugging/ Server is Listening

    while (numberOfRanThreads < 3)
    {
        
        newsocketfd = accept(socketfd, (struct sockaddr *)&client, &socketLength);
        printf("Starting thread. \n");
        pthread_create(&threads[numberOfRanThreads], NULL, messageHandlingThread, &newsocketfd);
        numberOfRanThreads++;
    }

    for(int i = 0; i < NUMBER_OF_THREADS; i++){
        pthread_join(threads[i], NULL);
    }

    close(socketfd);
}
