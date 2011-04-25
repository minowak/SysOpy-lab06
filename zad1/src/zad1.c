#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <getopt.h>
#include <fcntl.h>	// nonblock teoretycznie nie ma wplywu na inne niz
					// fifo

#define MAXLINE 10

void usage()
{
	printf("Dostepne argumenty\n");
	printf("-h\t--help\t\t\tWyswietla pomoc\n");
	printf("-w\t--writer\t\tFlaga O_NONBLOCK dla pisarza\n");
	printf("-r\t--reader\t\tFlaga O_NONBLOCK dla czytelnika\n");
	
	exit(1);
}

void koniec_pisarz(int signo)
{
	printf("PISARZ -> Czytelnik konczy to ja tez\n");
	_exit(1);
}

int main(int argc, char * argv[])
{
	static struct option long_opts[] = { 
		{"help", 0, 0, 'h'},
		{"writer", 0, 0, 'w'},
		{"reader", 0, 0, 'r'},
		{0, 0, 0, 0}
	};
		
	int reader_non = 0, writer_non = 0;
	int option_index = 0;
	int c = getopt_long(argc, argv, "hwr", long_opts, &option_index);;
		
	while(c != -1) 
	{
		switch(c)
		{
			//case 0: usage();
			
			case 'h': usage();
			case 'r': reader_non = 1;
						break;
			case 'w': writer_non = 1;
						break;
			
	//		case '?': usage();
	//		default: usage();
		}
		
		c = getopt_long(argc, argv, "hwr", long_opts, &option_index);
	}
	
	pid_t PID;
	int error_code;

	int fd[2];

	if( pipe(fd) < 0 )
	{
		error_code = errno;
		fprintf(stderr, "Blad pipe: %s\n", strerror(error_code));
		exit(error_code);
	}

	// ustawienie nonblock (nie blokujace)
	if(reader_non)	// czytelnik
	{
		printf("Ustawiam O_NONBLOCK dla Czytelnika\n");
		int oldflags = 0;
		if((oldflags = fcntl(fd[0], F_GETFL, 0)) < 0)
		{
			error_code = errno;
			fprintf(stderr, "Blad fcntl: %s\n", strerror(error_code));
			exit(error_code);
		}
		
		if(fcntl(fd[0], F_SETFL, oldflags | O_NONBLOCK) < 0)
		{
			error_code = errno;
			fprintf(stderr, "Blad fcntl: %s\n", strerror(error_code));
			exit(error_code);
		}
	}
	
	if(writer_non)	// pisarz
	{
		printf("Ustawiam O_NONBLOCK dla Pisarza\n");
		int oldflags = 0;
		if((oldflags = fcntl(fd[1], F_GETFL, 0)) < 0)
		{
			error_code = errno;
			fprintf(stderr, "Blad fcntl: %s\n", strerror(error_code));
			exit(error_code);
		}
		
		if(fcntl(fd[1], F_SETFL, oldflags | O_NONBLOCK) < 0)
		{
			error_code = errno;
			fprintf(stderr, "Blad fcntl: %s\n", strerror(error_code));
			exit(error_code);
		}
	}

	if( (PID = fork()) < 0)
	{
		error_code = errno;
		fprintf(stderr, "Blad fork: %s\n", strerror(error_code));
		exit(error_code);
	}

	if(PID > 0)		// czytelnik
	{
		close(fd[1]);
		
		char cmd[MAXLINE];
		while(1)
		{
			int n;
			if( (n = read(fd[0], cmd, MAXLINE) ) < 0)
			{
				error_code = errno;
				if(error_code == EAGAIN)
				{
					printf("Potok pusty\n");
					sleep(1);
				} else
				{
					fprintf(stderr,"Blad read: %s\n", strerror(error_code));
					exit(error_code);
				}
			} else
			if(n == 0)
			{
				printf("CZYTELNIK -> Pisarz zakonczyl to ja tez\n");
				_exit(1);
			} else
			{
				cmd[n] = '\0';
				printf("CZYTELNIK -> %s ", cmd);
			}
		}
	}
	else
	if(PID == 0)		// pisarz
	{
		close(fd[0]);
		
		signal(SIGPIPE, koniec_pisarz);
		
		while(1)
		{
			char cmd[MAXLINE];
			
			scanf("%s", cmd);
			if(strcmp(cmd, "quit") == 0) break;
			
			if(write(fd[1], cmd, strlen(cmd)) < 0)
			{
				error_code = errno;
				if(error_code == EAGAIN)
				{
					printf("Potok pelny\n");
					sleep(1); // zeby nie spamowac
				} else
				{
					fprintf(stderr, "Blad write: %s\n", strerror(error_code));
					exit(error_code);
				}
			}
		}
		
	}

	return EXIT_SUCCESS;
}
