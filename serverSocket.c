/*Group members: Loc Huynh, Christopher Hines, Peter Ogunrinde, & Edmund Sannda
 *Instructor: Mark Thompson		Class: CSCE3600		Due Date: 06/29/2015
 *Details: Minor Assignment 3 --- Using Linux Sockets
 */

#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h> 
#include <sys/un.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>

#define SOCKETNAME  "selectServerSocket"
#define BUY_COMMAND "buy"
#define CANCEL_COMMAND "cancel"


/* define a struct of ticket on server . */
struct Ticket  //Here's
	{
	int ticketNum;
	int status; /* 1: sold, 0: available */
	};

int GenerateTicketNum();
int GetAvailableTicket(struct Ticket arr[]);
void UpdateTicketStatus(int ticketNum, struct Ticket arr[], int status);
int IsValidTicketNum(int ticketNum, struct Ticket arr[]);
int IsTicketAvailable(int ticketNum, struct Ticket arr[]);

int main(void)
{
        char buf[1024];					/* Buffer for messages to others. */
        int s;                          /* Listen socket */
        int ns;                         /* Socket for first connection. */
        int ns2;                        /* Socket for second connection. */
        int len;                        /* len of sockaddr */   
        int maxfd;                      /* descriptors up to maxfd-1 polled*/
        int nread;                      /* # chars on read()*/
        int nready;                     /* # descriptors ready. */
        struct sockaddr_un name;
        fd_set fds;                     /* Set of file descriptors to poll*/

		struct	Ticket arrTicket[10];   /* Set of 10 tickets */
		int n;							/* read or write socket status */

		/* used for generate random number */
		srand((unsigned)time(0));

		/* Initial a random ticket number */
		int i;
		for (i = 0; i < 10; i++)
			{
			struct Ticket t;
			t.ticketNum = GenerateTicketNum();
			t.status = 0; //0:not sale, 1: sale
			arrTicket[i] = t;
			}

        /* Remove any previous socket.*/
        unlink(SOCKETNAME);

        /* Create the socket. */

        if ((s = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
        {
        perror("socket");
        exit(1);
         }

        /* Create the address of the server.*/

        memset(&name, 0, sizeof(struct sockaddr_un));

        name.sun_family = AF_UNIX;
        strcpy(name.sun_path, SOCKETNAME);
        len = sizeof(name.sun_family) + strlen(name.sun_path);

        /*Bind the socket to the address.*/

        if (bind(s, (struct sockaddr *) &name, len) < 0) {
        perror("bind");
        exit(1);
        }
        
        /* Listen for connections. */
        if (listen(s, 5) < 0){
        perror( "listen");
        exit(1);
        }


        /*Accept a connection.*/
        if ((ns = accept(s, (struct sockaddr *) &name, &len)) < 0) {
        perror("accept");
        exit(1);
        }
		printf("Client # 1 Connected.\n");

        /* Accept another connection. */
        if ((ns2 = accept(s, (struct sockaddr *) &name, &len)) < 0) {
        perror("accept");
        exit(1);
        }
		printf("Client # 2 Connected.\n");
        
        maxfd = (ns > ns2 ? ns : ns2) + 1;
        while(1){
                /* Set up polling using select. */
                FD_ZERO(&fds);
                FD_SET(ns,&fds);
                FD_SET(ns2,&fds);

                /* Wait for some input. */
                nready = select(maxfd, &fds, (fd_set *) 0, (fd_set *) 0,
                                (struct timeval *) 0);
                /* If either descriptor has some input,
                   read it and copy it to the other. */

                if( FD_ISSET(ns, &fds))
                {
						bzero(buf,256);
						n = read(ns,buf,255);
						if (n < 0) 
							{
							perror("ERROR reading from socket");
							close(ns);
                            close(ns2);
                            exit(0);
							}

						printf("[Client 1:] %s\n",buf);
						
						/* correct command received from client */
						//buf[strlen(buf) - 1] = '\0';				
						
						int clientTicketNum;
						clientTicketNum = 0;
						char * pCommand;
						pCommand = strtok (buf, " ");
						if (pCommand != NULL)
							{
							/* Cancel xxxxx */
							if (strcasecmp(pCommand, CANCEL_COMMAND) == 0 
								&& n >= 12)
									{
									pCommand = strtok (NULL, " ");
									clientTicketNum = atoi(pCommand);					
									}
							}						
					
						/* check the command from client */
						/* Process Buy command */
						if (strcasecmp(pCommand, BUY_COMMAND) == 0)							
							{
							/* process buy command */
							/* pick up an available ticket from the array, then send the ticket number to the client */
							
							int ticketNum = GetAvailableTicket(arrTicket);
							/* send back to client the ticketNum */
							if (ticketNum > 0)
								{
								/* update ticket is sold in the ticket array */
								UpdateTicketStatus(ticketNum, arrTicket, 1);

								//send back to client the ticket number.
								bzero(buf,256);
								sprintf(buf, "%d", ticketNum);
								//send( ns, buf, nread, 0);	
								send( ns, buf, strlen(buf), 0);	
								}
							else /* out of ticket */
								{
									//send back to client "out of ticket".
									bzero(buf,256);
									sprintf(buf, "%s", "Out Of ticket");
									send( ns, buf, strlen(buf), 0);									
								}
							}
						else if (clientTicketNum > 0)
							{
							/* process Cancel xxxxx ticket*/
							if (IsValidTicketNum(clientTicketNum, arrTicket) == 0)
								{
								if (IsTicketAvailable(clientTicketNum, arrTicket) == 0)
									{
										/* cannot cancel available ticket */
										//send back to client "ticket clientTicketNum is canceled".
										bzero(buf,256);
										sprintf(buf, "Cannot Cancel available ticket: %d", clientTicketNum);
										send( ns, buf, strlen(buf), 0);
									}
								else
									{
										/* Cancel Sold ticket */
										UpdateTicketStatus(clientTicketNum, arrTicket, 0); 
										//send back to client "ticket clientTicketNum is canceled".
										bzero(buf,256);
										sprintf(buf, "Ticket %d is canceled", clientTicketNum);
										send( ns, buf, strlen(buf), 0);
									}
								}
							else /* cancel a wrong ticket number */
								{
									//send back to client "Wrong ticket number: clientTicketNum".
									bzero(buf,256);
									sprintf(buf, "Wrong Ticket number: %d", clientTicketNum);
									send( ns, buf, strlen(buf), 0);
								}
							}
						else /* Invalid Command */
							{
								//send back to client "out of ticket".
								bzero(buf,256);
								sprintf(buf, "%s", "Invalid Command");								
								send( ns, buf, strlen(buf), 0);
							}						
                }

                if( FD_ISSET(ns2, &fds))
                {
                        /* process for second client */
						bzero(buf,256);
						n = read(ns2,buf,255);
						if (n < 0) 
							{
							perror("ERROR reading from socket");
							close(ns);
                            close(ns2);
                            exit(0);
							}

						printf("[Client 2:] %s\n",buf);
						/*printf("Length of buffer is: %d\n",strlen(buf));*/

						/* correct command received from client */
						//buf[strlen(buf) - 1] = '\0';				
						
						int clientTicketNum;
						clientTicketNum = 0;
						char * pCommand;
						pCommand = strtok (buf, " ");
						if (pCommand != NULL)
							{
							/* Cancel xxxxx */
							if (strcasecmp(pCommand, CANCEL_COMMAND) == 0 
								&& n >= 12)
									{
									pCommand = strtok (NULL, " ");
									clientTicketNum = atoi(pCommand);					
									}
							}
						
					
						/* check the command from client */
						/* Process Buy command */
						if (strcasecmp(pCommand, BUY_COMMAND) == 0)							
							{
							/* process buy command */
							/* pick up an available ticket from the array, then send the ticket number to the client */
							
							int ticketNum = GetAvailableTicket(arrTicket);
							/* send back to client the ticketNum */
							if (ticketNum > 0)
								{
								/* update ticket is sold in the ticket array */
								UpdateTicketStatus(ticketNum, arrTicket, 1);

								//send back to client the ticket number.
								bzero(buf,256);
								sprintf(buf, "%d", ticketNum);
								send( ns2, buf, strlen(buf), 0);								
								}
							else /* out of ticket */
								{
									//send back to client "out of ticket".
									bzero(buf,256);
									sprintf(buf, "%s", "Out Of ticket");
									send( ns2, buf, strlen(buf), 0);									
								}
							}
						else if (clientTicketNum > 0)
							{
							/* process Cancel xxxxx ticket*/
							if (IsValidTicketNum(clientTicketNum, arrTicket) == 0)
								{
								if (IsTicketAvailable(clientTicketNum, arrTicket) == 0)
									{
										/* cannot cancel available ticket */
										//send back to client "ticket clientTicketNum is canceled".
										bzero(buf,256);
										sprintf(buf, "Cannot Cancel available ticket: %d", clientTicketNum);
										send( ns2, buf, strlen(buf), 0);
									}
								else
									{
										/* Cancel Sold ticket */
										UpdateTicketStatus(clientTicketNum, arrTicket, 0); 
										//send back to client "ticket clientTicketNum is canceled".
										bzero(buf,256);
										sprintf(buf, "Ticket %d is canceled", clientTicketNum);
										send( ns2, buf, strlen(buf), 0);
									}
								}
							else /* cancel a wrong ticket number */
								{
									//send back to client "Wrong ticket number: clientTicketNum".
									bzero(buf,256);
									sprintf(buf, "Wrong Ticket number: %d", clientTicketNum);
									send( ns2, buf, strlen(buf), 0);
								}
							}
						else /* Invalid Command */
							{
								//send back to client "out of ticket".
								bzero(buf,256);
								sprintf(buf, "%s", "Invalid Command");								
								send( ns2, buf, strlen(buf), 0);
							}		
                }

        } 
}

/* generate a random 5 digits for a ticket from 10000 - 99999*/
int GenerateTicketNum()
	{
	int r;
	const unsigned int min = 10000;
	const unsigned int max = 99999;
    const unsigned int range = 1 + max - min;
    const unsigned int buckets = RAND_MAX / range;
    const unsigned int limit = buckets * range;

    /* Create equal size buckets all in a row, then fire randomly towards
     * the buckets until you land in one of them. All buckets are equally
     * likely. If you land off the end of the line of buckets, try again. */
    do
    {
		r = rand();
    } while (r >= limit);

    return min + (r / buckets);
	}

/* Get available ticket from ticket array */
int GetAvailableTicket(struct Ticket arr[])
	{
	int i;
	for (i = 0; i < 10; i++)
		{
		if (arr[i].status == 0)
			{
			return arr[i].ticketNum;
			}
		}
	return -1; //no ticket available
	}

/* update ticket status in ticket array */
void UpdateTicketStatus(int ticketNum, struct Ticket arr[], int status)
	{
	int i;
	for (i = 0; i < 10; i ++)
		{
		if (arr[i].ticketNum == ticketNum)
			{
			arr[i].status = status; //ticket sold
			break;
			}
		}
	}

/* check a ticket number is valid or not */
/* return 0 if valid, otherwise is not */
int IsValidTicketNum(int ticketNum, struct Ticket arr[])
	{
	int i;
	for (i = 0; i < 10; i ++)
		{
		if (arr[i].ticketNum == ticketNum)
			{
			return 0;
			}
		}
	return -1;
	}

/* check a ticket number is available or not */
/* return 0 if available, otherwise is not */
int IsTicketAvailable(int ticketNum, struct Ticket arr[])
	{
	int i;
	for (i = 0; i < 10; i ++)
		{
		if (arr[i].ticketNum == ticketNum
			&& arr[i].status == 0)
			{
			return 0;
			}
		}
	return -1;
	}
