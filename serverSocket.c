#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h> 
#include <sys/un.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>

#define SOCKETNAME  "selectServerSocket"


/* define a struct of ticket on server . */
struct Ticket  //Here's
	{
	int ticketNum;
	int status;
	};

int GenerateTicketNum();
int GetAvailableTicket(struct Ticket arr[]);
void UpdateTicketStatus(int ticketNum, struct Ticket arr[], int status);

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

	struct	Ticket arrTicket[10];           /* Set of 10 tickets */

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

        /* Accept another connection. */
        if ((ns2 = accept(s, (struct sockaddr *) &name, &len)) < 0) {
        perror("accept");
        exit(1);
        }
        
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
                        nread = recv(ns, buf, sizeof(buf), 0);
                        /* If error or eof, terminate. */
                        if(nread < 1){
                                close(ns);
                                close(ns2);
                                exit(0);
                        }

						/* check the command from client */
						if (stricmp(buf, "buy") == 0)
							{
							/* process buy command */
							/* pick up an available ticket from the array, then send the ticket number to the client */
							int ticketNum = GetAvailableTicket(arrTicket);
							/* send back to client the ticketNum */
							if (ticketNum > 0)
								{
								/* update ticket is sold in ticket array */
								UpdateTicketStatus(ticketNum, arrTicket, 1);

								//send back to client the ticket number.
								sprintf(buf, "%d", ticketNum);
								send( ns, buf, nread, 0);
								}
							}						
                }

                if( FD_ISSET(ns2, &fds))
                {
                        nread = recv(ns2, buf, sizeof(buf), 0);
                        /* If error or eof, terminate. */
                        if(nread < 1){
                                close(ns);
                                close(ns2);
                                exit(0);
                        }
                        send( ns2, buf, nread, 0);
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

