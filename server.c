/******************************************************************************
Project 3: Reliable File Transfer Over UDP
Course: CSCI 6760
Deadline: 03/21
File: server.c
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

#define MAX_MSG 512
char buffer[MAX_MSG];
char ack[10];
char newack[10];
char fileBuffer[501];
FILE *fp;

void clearBuffer(){
	int i;	
	for(i=0;i<MAX_MSG;i++){
		buffer[i] = '\0';
	}
}

void clearACK(){
	int i;	
	for(i=0;i<10;i++){
		ack[i] = '\0';
	}

}

void clearFileBuffer(){
	int i;	
	for(i=0;i<501;i++){
		fileBuffer[i] = '\0';
	}
}

void clearNEWACK(){
	int i;	
	for(i=0;i<10;i++){
		newack[i] = '\0';
	}

}

int main(int argc, char *argv[]) {
  
  int sd, rc, n, cliLen, counter, LOCAL_SERVER_PORT;
  if(argc != 2){
	printf("Error : Arguments are not specified as per the given specification <Port Number>\n");
	exit(1);
  }	
  LOCAL_SERVER_PORT = atoi(argv[1]);
  
  struct sockaddr_in cliAddr, servAddr;
  char msg[MAX_MSG];

  /* socket creation */
  sd=socket(AF_INET, SOCK_DGRAM, 0);
  if(sd<0) {
    printf("Error : cannot open socket \n");
    exit(1);
  }

  /* bind local server port */
  servAddr.sin_family = AF_INET;
  servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servAddr.sin_port = htons(LOCAL_SERVER_PORT);
  rc = bind (sd, (struct sockaddr *) &servAddr,sizeof(servAddr));
  if(rc<0) {
    printf("Error : Cannot bind Port number\n");
    exit(1);
  }

  printf("Server : waiting for data on port UDP %d\n",LOCAL_SERVER_PORT);

  //Accepting Connection
  char connectionReq[100];
  char fileName[50];
  char fileSize[50];
  char noOfParts[50];
  
  for(counter = 0; counter < 100; counter++){
	connectionReq[counter] = '\0';
	if(counter<50){
		fileName[counter] = '\0';
		fileSize[counter] = '\0';
		noOfParts[counter] = '\0';
	}
	if(counter<10){
		ack[counter] = '\0';
	}	
  }

  int pos = 0;
  cliLen = sizeof(cliAddr);
  while(1){
        pos = 0;
	n = recvfrom(sd, connectionReq, 100, MSG_WAITALL,(struct sockaddr *) &cliAddr, &cliLen);
	connectionReq[n] = '\0';
	//printf("%s\n %d\n", connectionReq,n);
	if(connectionReq[0] == '#'){		
		counter = 1;
		while(connectionReq[counter] != '#'){
			fileName[pos] = connectionReq[counter];
			counter++;
			pos++;
		}		
		fileName[pos] = '\0';
		//printf("%s\n", fileName);
		pos = 0;
		counter++;
		while(connectionReq[counter] != '#'){
			fileSize[pos] = connectionReq[counter];
			counter++;
			pos++;
		}
		fileSize[pos] = '\0';
		//printf("%s\n", fileSize);
		pos = 0;
		counter++;
		while(connectionReq[counter] != '#'){
			noOfParts[pos] = connectionReq[counter];
			counter++;
			pos++;
		}
		noOfParts[pos] = '\0';
		//printf("%s\n%s\n%s\n",fileName,fileSize,noOfParts);
		printf("Connection Established Successfully\n");
		clearACK();		
		sprintf(ack,"#%d#",1);
		//printf("%s\n%d\n", ack, strlen(ack));
		//sleep(1);
		sendto(sd,ack,3,0,(struct sockaddr *)&cliAddr, cliLen);
		break;
	}
  }//while

  int fileSizeNum, noOfPartsNum;
  fileSizeNum = atoi(fileSize);
  noOfPartsNum = atoi(noOfParts);
  int partNum = 0;
  int cnt = 0, cnt1 = 0;
  char seq[10];
  for(cnt  = 0; cnt<10; cnt++){
	seq[cnt] = '\0';
  }

  //File Open 
  
  fp = fopen(fileName,"wb");
  int amountcopied = 0;	
  
  while(partNum<noOfPartsNum){
	clearBuffer();
	if(partNum < noOfPartsNum-1){	
		n = recvfrom(sd, buffer, MAX_MSG, MSG_WAITALL,(struct sockaddr *) &cliAddr, &cliLen);			
		if(buffer[0] == '#'){
		//Duplicate Connection Request
			//printf("Duplicate\n");
			sendto(sd,ack,3,0,(struct sockaddr *)&cliAddr, cliLen);			
		}else{
			cnt = 0;
			while(buffer[cnt] != '#'){
				seq[cnt] = buffer[cnt];
				cnt++;
			}
			seq[cnt] = '\0';
			int numseq = atoi(seq);							
			if(numseq == partNum){
				//Copy the content
				//printf("%d\n",partNum);
				clearNEWACK();
				sprintf(newack,"%d#%d#",1,partNum);
				//Write to file				
				int filecnt = 0;
				counter = 0;
				cnt++;
				while(counter < 2){
					if(buffer[cnt] == '#') counter++;
					cnt++;
				}
				
				clearFileBuffer();
				while(filecnt<500){
					fileBuffer[filecnt] = buffer[cnt];
					filecnt++;
					cnt++;
				}
				fwrite(fileBuffer,1,filecnt,fp);
				//fileSizeNum -=500;
				//printf("%d\n",fileSizeNum);
				amountcopied += 500;
				//printf("%d\n",amountcopied);
				//printf("Part Number:%d Copied\n",partNum);
				partNum++;				
				sendto(sd,newack,strlen(newack),0,(struct sockaddr *)&cliAddr, cliLen);				
			}else{
				//Out of Order Packet
				sendto(sd,newack,strlen(newack),0,(struct sockaddr *)&cliAddr, cliLen);
			}//if
		}//else
	}else{
		
		n = recvfrom(sd, buffer, (fileSizeNum+12), MSG_WAITALL,(struct sockaddr *) &cliAddr, &cliLen);
		cnt = 0;
		while(buffer[cnt] != '#'){
			seq[cnt] = buffer[cnt];
			cnt++;
		}
		seq[cnt] = '\0';
		int numseq = atoi(seq);							
		if(numseq == partNum){
		//Copy the content
			//printf("Else:%d",partNum);
			clearNEWACK();
			fileSizeNum -= amountcopied;
			//printf("File Size:%d\nAmountCopied%d\n",fileSizeNum, amountcopied);
			sprintf(newack,"%d#%d#",1,partNum);
			//Write to file			
			int filecnt = 0;
			counter = 0;
			cnt++;
			while(counter < 2){
				if(buffer[cnt] == '#') counter++;
				cnt++;
			}
			
			clearFileBuffer();
			while(filecnt<fileSizeNum){
				fileBuffer[filecnt] = buffer[cnt];
				filecnt++;
				cnt++;
			}
			fwrite(fileBuffer,1,filecnt,fp);
			//printf("Part Number:%d Copied\n",partNum);			
			sendto(sd,newack,strlen(newack),0,(struct sockaddr *)&cliAddr, cliLen);				
			break;
		}else{
		//Out of Order Packet
			sendto(sd,newack,strlen(newack),0,(struct sockaddr *)&cliAddr, cliLen);
		}//else
	}//else
  }//while
  fclose(fp);
  char fin[3];
  for(counter = 0; counter<3; counter++){
  	fin[counter] = '\0';
  }

  while(1){
	sendto(sd,newack,strlen(newack),0,(struct sockaddr *)&cliAddr, cliLen);
	n = recvfrom(sd, fin, 3, MSG_WAITALL,(struct sockaddr *) &cliAddr, &cliLen);
        if(fin[0] == '#' || fin[2] == '#' || fin[1] == '1'){
		sendto(sd,fin,3,0,(struct sockaddr *)&cliAddr, cliLen);
		break;
	}//if
  }//while
  printf("Connection terminated successfully\n");
  return 1;
}

