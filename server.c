//Waleed Gudah A6 WebServer//
//Alot of help from this tutorial @ http://www.ibm.com/developerworks/systems/library/es-nweb/index.html?ca=dat//
//Credit where credit is due//


//My main process runs as daemon to prevent Zombified children
//If you have trouble stopping my main Process, as it detaches from the terminal, 
//Please use "ps -ef | grep Server" to locate it, then kill -9 pid to end it//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "servertypes.h"
#include "serverfundef.h"
#define BUFSIZE 8096
#define ERROR      42 //the Answer to the Ultimate Question of Life, the Universe, and Everything.
#define NOTFOUND  404

int openFiles = 0;

//root path
char root[200]={0};

struct {
	
	char *ext;
	char *filetype;
	
}

extensions [] = {
	//Our Supported Extensions 
	{"gif", "image/gif" },  
	{"jpg", "image/jpg" }, 
	{"jpeg","image/jpeg"},
	{"png", "image/png" },  
	{"ico", "image/ico" },  
	{"zip", "image/zip" },  
        {"apk", "application/vnd.android"},
	{"gz",  "image/gz"  },  
	{"tar", "image/tar" },  
	{"htm", "text/html" },  
	{"html","text/html" },  
	{0,0}
};


int readfile(char *filepath,downloadstate *state){
    
   FILE *fp;
   fp=fopen(filepath,"rb");  
   if(!fp){
     return 1;
   }
   else{   

     fread(state,sizeof(*state),1,fp);

     fclose(fp);

     return 0;
   }
   /* FILE *fp;
    char ch;

    if((fp=fopen(filepath,"r"))==NULL){
        printf("file can not open \n");
    }

    while((ch=fgetc(fp))!=EOF){
        fputc(ch,stdout);
    }
     
    fclose(fp);
    
    return 0;*/
}

int writefile(char *filepath,downloadstate *state){
    
   FILE *fp;
   fp=fopen(filepath,"wb");
   if(!fp){
     return 1;
   }else{
   
     fwrite(state,sizeof(*state),1,fp);

     fclose(fp);
 
     return 0;
   }
   /* FILE *fp=fopen(filepath,"w");

    fwrite("hello,world",1,strlen("hello,world"),fp);

    fclose(fp);
   
    return 0;*/
}

//write txt file
int writetxtfile(char *filepath,char *writestr){
    
    FILE *fp;
    fp=fopen(filepath,"w");
    if(!fp){
      return 1;
      
    }else{
      
      fwrite(writestr,1,strlen(writestr),fp);
     
      fclose(fp);
      return 0;
    }
    
}

void logger(int type, char *s1, char *s2, int socket_fd){
	
	char logbuffer[BUFSIZE*2];

	switch (type) {
		
	case ERROR: 
			
		(void)sprintf(logbuffer,"HTTP/1.1 500 ERROR"); 
		
		break;
			
	case NOTFOUND: 
			
		(void)write(socket_fd, "HTTP/1.1 404 Not Found\nContent-Length: 136\nConnection: close\n\n<html><head>\n<title>404 Not Found</title>\n</head><body>\n<h1> Error 404: Not Found</h1>\nThe requested URL was not found on this server.\n\n</body></html>\n",224);

		break;
			
	}	
	
}

/* Child WebServer */

void web(int fd, int hit){
	
        //printf("%d",strlen(root));
            
    
	int file_fd, buflen;
	long i, ret, len;
	char * fstr;
	static char buffer[BUFSIZE+1]; /* static so zero filled */

	ret =read(fd,buffer,BUFSIZE); 	/* read Web request in one go */

	for(i=4;i<BUFSIZE;i++) { /* null terminate after the second space to ignore extra stuff */
		
		if(buffer[i] == ' ') { /* string is "GET URL " +lots of other stuff */
			
			buffer[i] = 0;
			
		break;
			
		}
		
	}
	
	 /* convert no filename to index file */
	if( !strncmp(&buffer[0],"GET /\0",6) || !strncmp(&buffer[0],"get /\0",6) ){
		
		(void)strcpy(buffer,"GET /index.html");

	}

	/* work out the file type and check we support it */
	buflen=strlen(buffer);
	
	fstr = (char *)0;
	
	for(i=0;extensions[i].ext != 0;i++) {
		
		len = strlen(extensions[i].ext);
		
		if( !strncmp(&buffer[buflen-len], extensions[i].ext, len)) {
			
			fstr =extensions[i].filetype;
			
			break;
			
		}
		
	}

        //read the txt and the counter +1
        if(strcmp("application/vnd.android",fstr)==0){
           char wrpath[200]={0};
           char wrtxtpath[200]={0};

           strcat(wrpath,root);
           strcat(wrpath,"counter.dat");

           strcat(wrtxtpath,root);
           strcat(wrtxtpath,"counter.txt");
       
           printf("%s\n",wrpath);
           printf("%s\n",wrtxtpath);


           downloadstate counter;

           //init the counter number if read the file failed 
           //or the first time to read that the counter.dat not exist
           //this value will be 0
           counter.downloadtimes = 0;

           if(readfile(wrpath,&counter)==1){
             printf("read error or the first time to read\n");
             printf("maybe the file counter.dat not exist!\n");
           }

           counter.downloadtimes+=1;

           if(writefile(wrpath,&counter)==1){
               printf("write data error!\n");
            }
     
            char txtstr[200]={0};
            char countstr[200]={0};
            sprintf(countstr,":%d\n",counter.downloadtimes);
            strcat(txtstr,&buffer[5]);
            strcat(txtstr,countstr);          

            if(writetxtfile(wrtxtpath,txtstr)==1){
               printf("write text error!\n");
            }
            
            printf("times:%d\n",counter.downloadtimes);

            //printf("%s",fstr);
        }

        //connect root path and file path
        strcat(root,&buffer[5]);

	/* open the file for reading */
	//if(( file_fd = open(&buffer[5],O_RDONLY)) == -1){

        if(( file_fd = open(root,O_RDONLY)) == -1){
		
		logger(NOTFOUND, "404 Not Found!\n\n","404",fd);
		
	}else{
		
	len = (long)lseek(file_fd, (off_t)0, SEEK_END); /* lseek to the file end to find the length */
	      (void)lseek(file_fd, (off_t)0, SEEK_SET); /* lseek back to the file start ready for reading */
	      (void)sprintf(buffer,"HTTP/1.1 200 OK\nServer: WAL33D'z WebServer/\nContent-Length: %ld\nConnection: close\nContent-Type: %s\n\n", len, fstr); /* Header + a blank line */ 		    		      (void)write(fd,buffer,strlen(buffer));

		  openFiles++;
		
		// send file like a packet, in blocks//
	while ((ret = read(file_fd, buffer, BUFSIZE)) > 0 ) {
		
		(void)write(fd,buffer,ret);
		
		}

	}


	close(fd);
	
	close(file_fd);
	
	sleep(1);	
	
	exit(1);
	
}

int main(int argc, char **argv){
        //downloadstate s;
        //printf("%d",writefile("abc",&s));
	int pid, listenfd, socketfd, hit;
	socklen_t length;
	static struct sockaddr_in cli_addr; /* static = initialised to zeros */
	static struct sockaddr_in serv_addr; /* static = initialised to zeros */
	
	if( argc < 3  || argc > 3) {

	(void)printf("\t\n\nHINT: Server Port-Number Top-Directory\t\nExample: ./Server 8181 .\t\n\n\n");
				
		exit(0);

	} else {

	(void)printf("\t\nServer started.\t\nListening on Port-Number %s\t\n\n", argv[1]);
		
	}


	/* Become deamon + unstopable and no zombies children (= no wait()) */
	if(fork() != 0){
		
		return 0; /* parent returns OK to shell */
		
	}
		
        (void)signal(SIGCHLD, SIG_IGN); 
	
	(void)signal(SIGHUP, SIG_IGN); 
	
	(void)setpgrp();		
	
	
	/* break away from process group */
	
	/* setup the network socket */
	if((listenfd = socket(AF_INET, SOCK_STREAM,0)) <0){
		
		logger(ERROR,"SOCKET ERROR","bind",0);

		exit(1);
		
	}
	
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(atoi(argv[1]));
        //copy root path string
        strcpy(root,argv[2]);
	
	if(bind(listenfd, (struct sockaddr *)&serv_addr,sizeof(serv_addr)) <0){

		logger(ERROR,"COULD NOT BIND","bind",0);
	
		exit(1);
		
	}
	
	if( listen(listenfd,64) <0){
		
		logger(ERROR,"COULD NOT LISTEN","listen",0);
		
		exit(1);
		
	}
	
	for(hit=1; ;hit++) {
		
		length = sizeof(cli_addr);
		
		socketfd = accept(listenfd, (struct sockaddr *)&cli_addr, &length);
		
		if((pid = fork()) < 0) {
			
			logger(ERROR,"FORK ERROR","fork",0);
			
		exit(1);
		
		} else {
			
			if(pid == 0) { 	/* child */
				
				(void)close(listenfd);
				
				web(socketfd,hit); /* never returns */
				
			} else { 	/* parent */
				
				(void)close(socketfd);
				
			}
		}
	}
}
