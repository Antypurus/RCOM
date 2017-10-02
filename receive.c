/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>

#define BAUDRATE B38400
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1
#define FLAG 0x7E
#define A 0x03
#define CSET 0x03
#define ADDRS 0x03
#define SET 0x03
#define ADDR2 0x01
#define UA 0x07

volatile int STOP=FALSE;

int main(int argc, char** argv)
{
	unsigned char address = ADDR2;
	unsigned char control = UA;

	unsigned char bcc = (address ^ control);
	unsigned char control_frame[5];
	control_frame[0] = FLAG;
	control_frame[1] = ADDR2;
	control_frame[2] = UA;
	control_frame[3] = bcc;
	control_frame[4] = FLAG;
    int fd, res;
	unsigned char c;
    struct termios oldtio,newtio;
    char buf[255];

    if ( (argc < 2) || 
  	     ((strcmp("/dev/ttyS0", argv[1])!=0) && 
  	      (strcmp("/dev/ttyS1", argv[1])!=0) )) {
      printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
      exit(1);
    }


  /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
  */
  
    
    fd = open(argv[1], O_RDWR | O_NOCTTY );
    if (fd <0) {perror(argv[1]); exit(-1); }

    if ( tcgetattr(fd,&oldtio) == -1) { /* save current port settings */
      perror("tcgetattr");
      exit(-1);
    }

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;

    newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
    newtio.c_cc[VMIN]     = 5;   /* blocking read until 5 chars received */



  /* 
    VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a 
    leitura do(s) próximo(s) caracter(es)
  */



    tcflush(fd, TCIOFLUSH);

    if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }


    printf("New termios structure set\n");


		

	int state = 0;

	while(STOP == FALSE)
	{
		read(fd,&c,1);
		switch(state)
		{
		case 0:
		if(c == FLAG) {state = 1; printf("State 0\n"); } 
		else state = 0;
		break;

		case 1:
		if(c == A) {state = 2; printf("State 1\n");}
		else if(c == FLAG) state = 1;
		else state = 0;
		break;

		case 2:
		if(c == CSET) {state = 3; printf("State 2\n");}
		else if(c == FLAG) state = 1;
		else state = 0;
		break;

		case 3:
		if(c == CSET ^ A) {state = 4; printf("State 3\n");}
		else if(c == FLAG) state = 1;
		else state = 0;
		break;

		case 4:
		if(c == FLAG)
		{
		write(fd, control_frame,5);
		printf("State 4\n");
		STOP = TRUE;
		break;
		}
		else state = 0;
		break;
		}
	}

	



  /* 
    O ciclo WHILE deve ser alterado de modo a respeitar o indicado no guião 
  */



    tcsetattr(fd,TCSANOW,&oldtio);
    close(fd);
    return 0;
}
