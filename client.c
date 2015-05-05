/******************************************************************************
Project 3: Reliable File Transfer Over UDP
Course: CSCI 6760
Deadline: 03/21
File: client.c
Authors: Akshay Choche.
	 Chinmay Murugkar.
*/


#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h> 
#include <sys/time.h> 
#include <fcntl.h>


#define MAX_MSG 562


#define SOCKET_ERROR -1

FILE *fp,*fptr;
char buffer[512];
int counter = 0;
char tempbuffer[500];
char ack[10];

void clearACK(){
	int i;	
	for(i=0;i<10;i++){
		ack[i] = '\0';
	}
}

void clearBuffer(){
	int i;	
	for(i=0;i<512;i++){
		buffer[i] = '\0';
	}
}

void clearTempBuffer(){
	int i;	
	for(i=0;i<500;i++){
		tempbuffer[i] = '\0';
	}
}

int isReadable(int sd,int * error,int timeOut) {
  fd_set socketReadSet;
  FD_ZERO(&socketReadSet);
  FD_SET(sd,&socketReadSet);
  struct timeval tv;
  if (timeOut) {
    tv.tv_sec  = timeOut / 1000;
    tv.tv_usec = (timeOut % 1000) * 1000;
  } else {
    tv.tv_sec  = 0;
    tv.tv_usec = 0;
  } // if
  if (select(sd+1,&socketReadSet,0,0,&tv) == SOCKET_ERROR) {
    *error = 1;
    return 0;
  } // if
  *error = 0;
  return FD_ISSET(sd,&socketReadSet) != 0;
} /* isReadable */



int main(int argc, char *argv[]) {
  
  int sd, rc, n, echoLen, flags, error, timeOut;
  int i=2;
  struct sockaddr_in cliAddr, remoteServAddr, echoServAddr;
  struct hostent *h;
  char msg[MAX_MSG];
  char fname[100];
  int pt =0;
  

  /* check command line args */
  if(argc != 4) {
    printf("Error : Arguments are not specified as per the given specification <Server IP> <Server Port> <File Name>\n");
    exit(1);
  }

  /* get server IP address (no check if input is IP address or DNS name */
  int REMOTE_SERVER_PORT = atoi(argv[2]);
  h = gethostbyname(argv[1]);
  if(h==NULL) {
    printf("Error : Unable to Identify the Host please re run the program with a proper host \n");
    exit(1);
  }

//  printf("%s: sending data to '%s' (IP : %s) \n", argv[0], h->h_name,inet_ntoa(*(struct in_addr *)h->h_addr_list[0]));

  remoteServAddr.sin_family = h->h_addrtype;
  memcpy((char *) &remoteServAddr.sin_addr.s_addr, 
	 h->h_addr_list[0], h->h_length);
  remoteServAddr.sin_port = htons(REMOTE_SERVER_PORT);

  /* socket creation */
  sd = socket(AF_INET,SOCK_DGRAM,0);
  if(sd<0) {
    printf("Error : cannot open socket \n");
    exit(1);
  }
  
  /* bind any port */
  cliAddr.sin_family = AF_INET;
  cliAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  cliAddr.sin_port = htons(0);
  
  rc = bind(sd, (struct sockaddr *) &cliAddr, sizeof(cliAddr));
  if(rc<0) {
    printf("Error : cannot bind port\n");
    exit(1);
  }

  //Setting Time Out
  timeOut = 1000; // ms
  fd_set socketReadSet;
  FD_ZERO(&socketReadSet);
  FD_SET(sd,&socketReadSet);
  struct timeval tv;
  tv.tv_sec  = 1;
  select(sd+1,&socketReadSet,0,0,&tv);

  //retreving size of file
  char ch;
  if((fptr = fopen(argv[3],"rb"))==NULL) {
    printf("Cannot open file.\n");
    exit(1);
  }
  
  
  int x=0;
  fseek(fptr,0,SEEK_END);
  x = ftell(fptr);
  rewind(fptr);  
  //printf("total size of the file is %d \n",x);
  fclose(fptr);
  
  //Sending Connection Request
  char connectionReq[512];
  for(counter = 0; counter<512; counter++){
	connectionReq[counter] = '\0';
  }

  int pos = 0;
  char* fileName = argv[3];  
  int noOfParts = (x/500) + 1;
  char noOfPartsChar[10];
  char fileSizeChar[30];
  sprintf(fileSizeChar, "%d", x);
  sprintf(noOfPartsChar, "%d", noOfParts);
  
  counter = strlen(fileSizeChar);
  fileSizeChar[counter] = '\0';
  counter = strlen(noOfPartsChar);
  noOfPartsChar[counter] = '\0';

  int lengthOfFileName = strlen(fileName);
  //printf("%d\n", lengthOfFileName);
  
  connectionReq[pos] = '#';
  pos++;
  for(counter = 0; counter<lengthOfFileName; counter++){
	connectionReq[pos] = fileName[counter];
	pos += 1;
  }
  connectionReq[pos] = '#';
  pos += 1;
  counter = 0;	
  while(fileSizeChar[counter] != '\0'){
	connectionReq[pos] = fileSizeChar[counter];
	pos += 1;
	counter += 1;	
  }
  connectionReq[pos] = '#';
  pos += 1;
  counter = 0;
  while(noOfPartsChar[counter] != '\0'){
	connectionReq[pos] = noOfPartsChar[counter];
	pos += 1;
	counter += 1;
  }
  connectionReq[pos] = '#';
  pos += 1;
  

  
  for(counter = 0; counter<10; counter++){
	ack[counter] = '\0';	
  }
  echoLen = sizeof(echoServAddr);
  while(1){
	printf("%s\n", connectionReq);
  	rc = sendto(sd,connectionReq,pos, 0, (struct sockaddr *) &remoteServAddr, sizeof(remoteServAddr));	
	clearACK();
	n = recvfrom(sd, ack, 3, MSG_DONTWAIT,(struct sockaddr *) &echoServAddr, &echoLen);
	ack[n] = '\0';	
	//printf("\nn: %d\n%s\n", n, ack);
	if(n == 3){
		if(ack[0] == '#' && ack[2] == '#' && ack[1] == '1'){
			printf("Connection Established Successfully.\n");
			break;		
		}
	}
	sleep(1);		
  }//while Connection has been established

  //Sending data
  if((fp = fopen(fileName,"rb"))==NULL) {
    	printf("Cannot open file.\n");
    	exit(1);
  }
  int partcnt = 0;
  char header[12],packet[512];
  for(counter = 0; counter<512; counter++){
	packet[counter] = '\0';
  }
  for(counter = 0; counter<12; counter++){
	header[counter] = '\0';
  }
  int sizeOfFile = x;
  int num = 0;  
  int posnew = 0;
  //printf("File Opened.\n");
  for(partcnt = 0; partcnt < noOfParts; partcnt++){
	//printf("File Opened.\n");
        clearBuffer();
	if(partcnt < noOfParts-1){	
		sprintf(header,"%d#%d#0#",partcnt,512);
		clearTempBuffer();		
		num = fread(tempbuffer,500,1,fp);
		sizeOfFile -= 500;
		//printf("%s\n", header);
		counter = 0;
		posnew = 0;				
		while(posnew < strlen(header)){
			buffer[counter] = header[posnew];
			posnew += 1;
			counter += 1;
		}		
		posnew = 0;			
		while(posnew<500){
			buffer[counter] = tempbuffer[posnew];
			posnew += 1;
			counter += 1;
		}		
		//printf("%s\n", buffer);		
		while(1){
			//printf("If\n");
			rc = sendto(sd,buffer,counter, 0, (struct sockaddr *) &remoteServAddr, sizeof(remoteServAddr));
			//printf("Part Number:%d Sent\n", partcnt);			
			clearACK();
			n = recvfrom(sd, ack, 8, MSG_DONTWAIT,(struct sockaddr *) &echoServAddr, &echoLen);
			if(ack[0] == '1'){
				char seq[6];
				int test = 2;
				int test1 = 0;
				while(ack[test] != '#'){
					seq[test1] = ack[test];
					test1++;
					test++;
				}
				test = atoi(seq);
				if(test == partcnt) break;
			}//if			
			sleep(1);
		}//while
	}else{
		sprintf(header,"%d#%d#1#",partcnt,(sizeOfFile+12));
		clearTempBuffer();		
		num = fread(tempbuffer,sizeOfFile,1,fp);
		//printf("%s\n", header);
		counter = 0;
		posnew = 0;				
		while(posnew < strlen(header)){
			buffer[counter] = header[posnew];
			posnew += 1;
			counter += 1;
		}		
		posnew = 0;			
		while(posnew<sizeOfFile){
			buffer[counter] = tempbuffer[posnew];
			posnew += 1;
			counter += 1;
		}	
		//printf("%s\n", buffer);
		while(1){
			//printf("Else\n");
			rc = sendto(sd,buffer,counter, 0, (struct sockaddr *) &remoteServAddr, sizeof(remoteServAddr));
			//printf("Part Number:%d Sent\n", partcnt);			
			clearACK();
			n = recvfrom(sd, ack, 8, MSG_DONTWAIT,(struct sockaddr *) &echoServAddr, &echoLen);
			if(ack[0] == '1'){
				char seq[6];
				int test = 2;
				int test1 = 0;
				while(ack[test] != '#'){
					seq[test1] = ack[test];
					test1++;
					test++;
				}
				test = atoi(seq);
				if(test == partcnt) break;
			}//if			
			sleep(1);
		}//while
	}//else
  }//for
  fclose(fp);

  //Terminate the connection.
  char fin[3] = "#1#";
  counter = 0;
  while(counter<5){
  	rc = sendto(sd,fin,3, 0, (struct sockaddr *) &remoteServAddr, sizeof(remoteServAddr));  
	printf("ACK Send\n");  	
  	clearACK();
  	n = recvfrom(sd, ack, 3, MSG_DONTWAIT,(struct sockaddr *) &echoServAddr, &echoLen);	
	if(ack[0] == '#' || ack[2] == '#' || ack[1] == '1'){	
		break;
	}//if
	counter++;
	sleep(1);
  }//while
  printf("Connection terminated successfully\n");
  return 1;
}

