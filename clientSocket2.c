/*Group members: Loc Huynh, Christopher Hines, Peter Ogunrinde, & Edmund Sannda
 *Instructor: Mark Thompson		Class: CSCE3600		Due Date: 06/29/2015
 *Details: Minor Assignment 3 --- Using Linux Sockets
 */
#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h> 
#include <string.h> 
#include <sys/un.h>
#include <sys/time.h>
#include <fcntl.h>
#include <stdlib.h>

#define SOCKETNAME  "selectServerSocket"
#define BUY_COMMAND "Buy"
#define CANCEL_COMMAND "Cancel"

int GenerateRandomNumber(int, int);
void UpdateTicketNumber(int, int [], int);
int GetAvailableTicket(int [10]);
int main(void)
{
        int s;          /* This end of connection*/
        int len;        /* length of sockaddr */
        int nread;      /* return from read() */
        int nready;     /* # fd's ready. */
        int maxfd;      /* fd's 0 to maxfd-1 checked. */ 
        char buf[1024]; /* buffer from server */
		char buf1[1024]; /* buffer from client */
        fd_set fds;     /* set of file descriptors to check. */
        struct sockaddr_un name;
		int arrTicketNum[10];  /* array store ticket number received from server */
		int n;           /* status from read and write */


        if( (s = socket(AF_UNIX, SOCK_STREAM, 0) ) < 0){
                perror("socket");
                exit(1);
        }

		/*Create the address of the server.*/

        memset(&name, 0, sizeof(struct sockaddr_un));

        name.sun_family = AF_UNIX;
        strcpy(name.sun_path, SOCKETNAME);
        len = sizeof(name.sun_family) + strlen(name.sun_path);


        /*Connect to the server.*/

        if (connect(s, (struct sockaddr *) &name, len) < 0){
                perror("connect");
                exit(1);
        }

		/* Initialize array of ticket number */
		int i;
		for (i = 0; i < 10; i++)
			{
			arrTicketNum[i] = 0;
			}

		/* used for generate random number */
		srand((unsigned)time(0));

        /* generate a random 5 digit number to cancel invalid ticket at first */
		int ticketNumber;
		
		/* generate 7 Buy commands and 1 cancel randomly*/
		for (i = 0; i < 8; i ++)
			{
			int randomNum;
			randomNum = GenerateRandomNumber(0, 9);
			if (randomNum > 0) //send buy commmand
				{
				bzero(buf1,256);
				sprintf(buf1, "%s", BUY_COMMAND);
				printf("[Client 2:] %s\n",buf1);
				n = write(s,buf1,strlen(buf1));

				/* print out response from server */
				bzero(buf,256);
				n = read(s,buf,255);
				if (n < 0) 
						error("ERROR reading from socket");

				//printf("[Client 2:] %s\n",buf1);

				printf("[Server:] %s\n",buf);
				ticketNumber = atoi(buf);
				/* save ticket number to array */
				UpdateTicketNumber(ticketNumber, arrTicketNum,1);
				}
			else if (randomNum <= 3) 
				{
				/* cancel a ticket number got from list */
				ticketNumber = GetAvailableTicket(arrTicketNum);
				/* send valid cancel ticket */
				bzero(buf1,256);
				sprintf(buf1, "Cancel %d", ticketNumber);
				printf("[Client 1:] %s\n",buf1);
				n = write(s,buf1,strlen(buf1));

				/* print out response from server */
				bzero(buf,256);
				n = read(s,buf,255);
				if (n < 0) 
						error("ERROR reading from socket");

				//printf("[Client 2:] %s\n",buf1);
				printf("[Server:] %s\n",buf);
				UpdateTicketNumber(ticketNumber, arrTicketNum, 0);
				}
		}

		

		/* send out wrong command */
		bzero(buf1,256);
		sprintf(buf1, "%s", "Send Wrong Command");						
		n = write(s,buf1,strlen(buf1));
		printf("[Client 2:] %s\n",buf1);
		/* print out response from server */
		bzero(buf,256);
		n = read(s,buf,255);
		if (n < 0) 
				error("ERROR reading from socket");		
		printf("[Server:] %s\n",buf);

		/* print out ticket table */
		printf("Print out All Buy Tickets from this Client \n");
		for (i = 0; i < 10; i++)
		{
		if (arrTicketNum[i] != 0)
			{
			printf("Ticket %d: %d\n", i, arrTicketNum[i]);
			}
		}
	
	//	/* wait for a while to ensure that server is ready to exit */
		for (i = 0; i < 99999999; i++)
			;
	close(s);
    return 0;
}

/* update ticket number to the array of number 
   status = 1: valid ticket number from server
   status = 0: removed the ticket number from array by updating 0 to the corresponding element */
void UpdateTicketNumber(int ticketNumber, int arrTicketNumber[10], int status)
	{
	int i;
	for (i = 0; i < 10; i ++)
		{
		if (ticketNumber == arrTicketNumber[i] 
			&& status == 0)
			{
			/* update 0 to the ticket Number */
			arrTicketNumber[i] = 0;
			break;
			}
		else if (arrTicketNumber[i] == 0 
			&& status == 1)
			{
			/* save the ticketNumber to the array */
			arrTicketNumber[i] = ticketNumber;
			break;
			}
		}
	}

/* get the first available ticket number from list */
int GetAvailableTicket(int arrTicketNum[10])
	{
	int i;
	for (i = 0; i < 10; i++)
		{
		if (arrTicketNum[i] > 0)
			{
			return arrTicketNum[i];
			}
		}
	return -1;
	}


/* generate a random 5 digits for a ticket from 10000 - 99999*/
int GenerateRandomNumber(int min, int max)
	{
	int r;
	/*const unsigned int min = 10000;
	const unsigned int max = 99999;*/
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


