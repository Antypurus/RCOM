#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <signal.h>

#define FLAG 0x7E
#define ADDRS 0x03
#define SET 0x03
#define ADDR2 0x01

#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

#define READ_FLAG 0
#define READ_ADDR 1
#define READ_CTR 2
#define CHECK_BCC 3
#define READ_FLAG_2 4
#define DONE_READING 5

volatile int STOP=FALSE;
volatile int HAS_RECEIVED = FALSE;
unsigned int resend_counter = 0;

unsigned int fdd;
unsigned int *sd;

void  ALARMhandler(int sig)
{
  if(HAS_RECEIVED==FALSE){
  	if(resend_counter>=3){
  		printf("Connection Timed Out\n");
  		exit(-1);
  	}else{
  		send(fdd,sd,5);
  		printf("Resent Packet, Attempt %d\n",resend_counter+1);
  		resend_counter++;
  		alarm(3);
  	}
  } 
}

void main(int argc,char** argv){
	signal(SIGALRM, ALARMhandler);

	unsigned char address = ADDRS;
	unsigned char control = SET;

	unsigned char bcc = (address ^ control);
	unsigned char control_frame[5];
	control_frame[0] = FLAG;
	control_frame[1] = ADDRS;
	control_frame[2] = SET;
	control_frame[3] = bcc;
	control_frame[4] = FLAG;

	printf("created controll frame\n");

	int fd,c, res;
    struct termios oldtio,newtio;
    int i, sum = 0, speed = 0;

    if ( (argc < 2) ||
  	     ((strcmp("/dev/ttyS0", argv[1])!=0) &&
  	      (strcmp("/dev/ttyS1", argv[1])!=0) )) {
      printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
      exit(1);
    }

       fd = open(argv[1], O_RDWR | O_NOCTTY);
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

    res = write(fd,control_frame,5);
    printf("%d bytes written\n", res);
    fdd = fd;
    sd = control_frame;

    printf("created controll frame:%s\n",control_frame);

    unsigned char curr_state = READ_FLAG;
    unsigned char reade = 0;
    unsigned char addr_r=0;
    unsigned char ctr_r =0;

    unsigned int readd = 0;

    while(curr_state != DONE_READING){
    	alarm(3);
    	
    	readd = read(fd,&reade,1);

 		if(readd!=0){
 			alarm(0);
 			readd = 0,
 			HAS_RECEIVED = TRUE;
 		}

    	switch(curr_state){
    		case(READ_FLAG):{
    			if(reade == FLAG){
    				curr_state = READ_ADDR;
    			}else{
    				printf("[ERROR] \t INCONRRECT FLAG\n");
    			}
    			break;
    		}
    		case(READ_ADDR):{
    			if(reade == 0x01){
    				addr_r = reade;
    				curr_state = READ_CTR;
    			}else{
    				printf("[ERROR] \t INCORRECT ADDRESS RECEIVED\n");
    				curr_state = READ_FLAG;
    			}
    			break;
    		}
    		case(READ_CTR):{
    			if(reade == 0x07){
    				ctr_r = reade;
    				curr_state = CHECK_BCC;
    			}else{
    				printf("[ERROR] \t UNORDERED ACK NOT RECEIVED\n");
    				curr_state = READ_FLAG;
    			}
    			break;
    		}
    		case(CHECK_BCC):{
    			if(reade == (addr_r ^ ctr_r)){
    				curr_state = READ_FLAG_2;
    			}else{
    				printf("[ERROR] \t PARTITY ERROR\n");
    				curr_state = READ_FLAG;
    			}
    			break;
    		}
    		case(READ_FLAG_2):{
    			if(reade==FLAG){
    				curr_state = DONE_READING;
    				printf("[SUCCESS] \t CONTROLL FRAME PROPERLY REEAD AND PROCESSED\n");
    			}else{
    				curr_state = READ_FLAG;
    				printf("[ERROR] \t FINAL FLAG READ ERROR\n");
    			}
    			break;
    		}
    		default:break;
    	}
    }

    if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

    close(fd);
    return 0;
}
