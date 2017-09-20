#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include "config.h"
#include "siparse.h"
#include "utils.h"
#include "builtins.h"

line * ln;
command *com;
volatile pid_t forePid[MAX_PID];
volatile int pidCounter = 0;
char output[MAX_OUTPUT_LENGTH][100];
volatile int chCounter = 0;
volatile int outputSize = 0;

int ifInForeground(pid_t pid){
	//printf("%d \n", pid);
	int i=0;
	for(i = 0; i < pidCounter; i++){
		if(forePid[i] == pid)
			return 1;
	}
return 0;
}

void handler(int sig_nb){
	//write(1, "jestem handler\n", 15);
	pid_t ch = (pid_t)1;
	int c;
	//printf("%d\n",sig_nb);

	int stat;
	while(ch > 0){
		int* stat_lock;
		ch = waitpid(-1, &stat, WNOHANG);
		/*if(ch == -1){
			if(errno == ECHILD)
				printf("ECHILD\n");
			else{
				if(errno == EINTR)
					printf("EINTR\n");
				else
					printf("EINVAL\n");
			}
		}
		printf("%d\n", ch);*/
		if(ch > 0){
			if(ifInForeground(ch))
				chCounter--;
			else{
				//write(1, "wylaczam\n", 9);
				//printf("%d\n",stat);
				if((c = WIFEXITED(stat))){
						sprintf(output[outputSize], "Background process %d terminated. (excited with status 0)\n", ch);
						outputSize++;
				}
				else{
					if(c = WIFSIGNALED(stat)){
						if(outputSize < MAX_OUTPUT_LENGTH){
							sprintf(output[outputSize], "Background process %d terminated. (killed by signal %d)\n", ch, WTERMSIG(stat));
							//write(1, &output[outputSize], 50);
							outputSize++;
						}
					}
					else if((c = WEXITSTATUS(stat)) != 0){
						if(outputSize < MAX_OUTPUT_LENGTH){
							sprintf(output[outputSize], "Background process %d terminated. (excited with status %d)\n", ch, c);
							outputSize++;
						}
					}
				}
			}
		}
	}
}


int openReFile(char* filename, int fd){
  int k;
  k = close(fd);
  if(k == -1)
    exit(2);
  k = open(filename, O_RDONLY);
  if(k == -1){
    switch(errno){
      case EACCES:  write(2, strcat(filename, ": permission denied\n"), strlen(strcat(filename, ": permission denied\n"))); break;
      case ENOENT:  write(2, strcat(filename, ": no such file or directory\n"), strlen(strcat(filename, ": no such file or directory\n"))); break;
      default: write(2, strcat(filename, ": read error\n"), strlen(strcat(filename, ": read error\n")));
    }
    exit(EXEC_FAILURE);
  }
  if(k != fd)
    exit(2);
return k;
}

int openWrFile(char* filename, int fd){
  int k;
  k = close(fd);
  if(k == -1)
    exit(2);
 // printf("open file %s\n",filename);
  mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
  k = creat(filename, mode);
  //k = open(filename, O_WRONLY|O_CREAT|O_TRUNC);
  if(k == -1){
    switch(errno){
      case EACCES:  write(2, strcat(filename, ": permission denied\n"), strlen(strcat(filename, ": permission denied\n"))); break;
      case ENOENT:  write(2, strcat(filename, ": no such file or directory\n"), strlen(strcat(filename, ": no such file or directory\n"))); break;
      default: write(2, strcat(filename, ": write error\n"), strlen(strcat(filename, ": write error\n")));
    }
    exit(EXEC_FAILURE);
  }
  if(k != fd)
    exit(2);
return k;
}

int openApFile(char* filename, int fd){
  int k;
  k = close(fd);
  if(k == -1)
    exit(2);
  mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
  k = open(filename, O_WRONLY|O_CREAT|O_APPEND, mode);
  if(k == -1){
    switch(errno){
      case EACCES:  write(2, strcat(filename, ": permission denied\n"), strlen(strcat(filename, ": permission denied\n"))); break;
      case ENOENT:  write(2, strcat(filename, ": no such file or directory\n"), strlen(strcat(filename, ": no such file or directory\n"))); break;
      default: write(2, strcat(filename, ": append error\n"), strlen(strcat(filename, ": append error\n"))); 
    }
    exit(EXEC_FAILURE);
  }
  if(k != fd)
    exit(2);
return k;
}

int checkIfNull(line* ln){
  pipeline * p;
  int d = 1, c, e;
  for(p=ln->pipelines; *p; p++){
    if(p != NULL){
      command** pcmd;
      c = 1; e = 0;
      for(pcmd = *p; *pcmd; pcmd++, c++){
        if((*pcmd)->argv[0] == NULL){
          e++;     
        } 
      }
      if(c >= 2 && e > 1){
        //printf("bum%d %d\n", c, e);
        return -1;
      }
    }
  }
return 0;
}

int checkIfInBuiltins(command* coma){
  //write(1, coma->argv[0], 10);
  int i = 0, res;
  char tmp[40] ;
  while(builtins_table[i].name != NULL){
    if(strcmp(builtins_table[i].name, coma->argv[0]) == 0){ 
      res = builtins_table[i].fun(coma->argv);
      if(res != 0){
      	sprintf(tmp, "Builtin %s error.\n", builtins_table[i].name);
      	write(2, tmp, strlen(tmp));
      
      }
      return res;
    }
    i++;
  }
return 150;
}



int children(command* com, int fdread, int fdwrite, int notUsedRead){
  //write(1, com->argv[0], 2);
 // printf("%d %d %d\n", fdread, fdwrite, notUsedRead);
  sigset_t mask;
  sigemptyset(&mask);
  sigaddset(&mask, SIGINT);
  sigprocmask(SIG_UNBLOCK, &mask, NULL);
  int p = 0, k;
  if(fdread != 0){
    close(0); // zamykam stdin, i w jego miejsce wstawiam fdread
    dup2(fdread, 0);
    close(fdread);
  }
  if(fdwrite != 1){
    close(1); // zamykam stdout i  w jego miejsce wstawiam fdwrite
    dup2(fdwrite, 1);
    close(fdwrite); // zamykam pliki fdread i fdwrite w procesie dziecka
  }
  if(notUsedRead != -1){
  	close(notUsedRead);
  }
  while(com->redirs[p] != NULL){
    if(IS_RIN(com->redirs[p]->flags)){
      k = openReFile(com->redirs[p]->filename, 0); //where have I opened
    }
    else{
      if(IS_ROUT(com->redirs[p]->flags)){
        k = openWrFile(com->redirs[p]->filename, 1);
      }
      else{
        if(IS_RAPPEND(com->redirs[p]->flags))
          k = openApFile(com->redirs[p]->filename, 1);
      }
    }
    p++;
  }
  execvp(com->argv[0], com->argv);
  if(errno == ENOENT){ 
      strcat(com->argv[0], ": no such file or directory\n");
      write(2, com->argv[0], strlen(com->argv[0]));              
      _exit(EXEC_FAILURE);
  }
  else{
      if(errno == EACCES){
        strcat(com->argv[0], ": permission denied\n");
          write(2, com->argv[0], strlen(com->argv[0]));
          _exit(EXEC_FAILURE);
      }
      else{
        strcat(com->argv[0], ": exec error\n");
        write(2, com->argv[0], strlen(com->argv[0]));
         _exit(EXEC_FAILURE);
      }
  }
}

void exec(char* line){
	int b;
	for(b=0; b < MAX_PID; ++b)
		forePid[b] = (pid_t)0;

	int ifBack = 0; // 0 - jezeli czytamy na foreground
	ln = parseline(line);
	if(ln == NULL){
	    exit(1);
	}
	int d = checkIfNull(ln);
	if(d == -1){
		write(2, "syntax error\n", 13);
	    //fprintf(stderr, "syntax error\n");
	}
	  //printf("ale jazda\n");
	//  printparsedline(ln);
	if((ln->flags && (1<<LINBACKGROUND)) != 0){
	  	//printf("udalo sie\n");
		ifBack = 1;
	}
	 // printparsedline(ln);
	pipeline * p;
	int c, e, i, res;
	int fd[2];
	 /* for(i = 0; i < 4; i++){
	    pipe(fd[i]);
  }*/

  	for(p=ln->pipelines; *p; p++){
	    command ** pcmd;
	    chCounter = 0;
	    c=0;
	    for(pcmd = *p; *pcmd; pcmd++, c++){
	        continue;
	    }
	    sigset_t mask, mask1;
	    sigemptyset(&mask);
    	sigaddset(&mask, SIGCHLD);
   		sigprocmask(SIG_BLOCK, &mask, &mask1);
	    if(c == 1){
	     //   write(1, "tylko 1 komenda\n", 16);
	    	chCounter = c;
	    	pcmd = *p;
	        if((*pcmd)->argv[0] != NULL){
	            res = 150;
	            if(ifBack == 0)
	            	res = checkIfInBuiltins(*pcmd);
	            if(res == 150){
		            int cPID = fork();
		            if(cPID){
		            	if(ifBack == 0){
		            		pidCounter = 0;
		            		forePid[pidCounter] = cPID;
		            		pidCounter++;
		            		//printf("ala ma kota %d\n", cPID);
		            	//	write(1, "bum\n", 4);
		              	}
		            }
		            else{ // dziecko
		            	if(ifBack)
		            		setsid();
		            	//printf("ale supers\n");
		              	children(*pcmd, 0, 1, -1);
		              	//printf("czyzby koniec\n");
		              	exit(0);
		            }
		            if(ifBack == 0){
		            	//printf("czekamy\n");
					    while(chCounter){
					      sigsuspend(&mask1);
				    	}
			    	}
	          	}
	        }
	    }
      	else{
     //   write(1, "wiecej command\n",10);
    //	printf("%d\n", ifBack);
        	pid_t pid;
        	int i, tmp = 0;
	        pcmd = *p;
	    //    printf("%d %d\n", c,chCounter);
	        chCounter = c;
	     //   printf("%d %d\n", c,chCounter);
	        if(ifBack == 0)
	        	pidCounter = 0;
	        for(i = 0; i < c; i++){
	          pipe(fd);
	          pid = fork();
	          if(pid != 0){
	          	if(pid < 0)
	          		exit(1);
	          	else if(ifBack == 0 && pidCounter < MAX_PID){
	          		forePid[pidCounter] = pid;
	          		pidCounter++;
	          	}
	          }
	          else if(pid == 0){
	          	if(ifBack == 1)
	          		setsid();
	            if(i + 1 == c){
	              close(fd[1]);
	              children(*(pcmd + i), tmp, 1, fd[0]);
	              exit(0);
	            }
	            else{
	              children(*(pcmd+i), tmp, fd[1], fd[0]);
	              exit(0);
	            }
	          }
	         // printf(":)%d %d\n", fd[0], fd[1]);
	          close(fd[1]);
	          if(tmp != 0)
	            close(tmp);
	          tmp = fd[0]; 
	        }
	        close(fd[0]);
	        //close(fd[1]);

	      //  printf("koniec pętli %d\n", end);
	        if(ifBack == 0){
			    while(chCounter){
			      sigsuspend(&mask1);
		    	}
	    	}
		}
		sigprocmask(SIG_SETMASK, &mask1, NULL);
  	}
}

int
main(int argc, char *argv[])
{
	sigset_t maska, maska1, maska2;
	struct sigaction act;
	
	act.sa_handler = handler;
	act.sa_flags = SA_RESTART;
	sigemptyset(&act.sa_mask);
	sigaction(SIGCHLD, &act, NULL);
	
	sigemptyset(&maska);
    sigaddset(&maska, SIGINT);
    sigprocmask(SIG_SETMASK, &maska, NULL);

    sigemptyset(&maska1);
    sigaddset(&maska1, SIGCHLD);

  	struct stat ist;       
	int status;   
	char line[MAX_LINE_LENGTH+1];

    if(fstat(0, &ist) == 0){          
	    status = S_ISCHR(ist.st_mode);
	}
	else
	    exit(1);
    char* wpocz = line;
    char* wob = line;
    char* wkon = line;
    int bol = 0;
 	while(1){
	    if(status){
	    	sigprocmask(SIG_BLOCK, &maska1, &maska2);
	    	int lw = 0, i;
	    	for(i=0; i < outputSize; ++i){
	    		lw = write(1, output[i], strlen(output[i]));
	    		if(lw <= 0)
	    			exit(1);
	    	}
	    	outputSize = 0;
	    	lw = write(1, PROMPT_STR, strlen(PROMPT_STR));
	  		if(lw <= 0)
	    		exit(1);
	    	sigprocmask(SIG_SETMASK, &maska2, NULL);
	    }
	    int le = read(0, wkon, MAX_LINE_LENGTH - (wkon - line));
	    if(le < 0){
	    	exit(1);
	    }     
	   // printf("osiem\n");
	    if(le == 0){
	        if(wpocz == wkon){               
	            exit(1);
	        }
	        *(wkon+1)='\n';
	    }
	    else{
	        wkon = wkon + le;
	    }   
	    while(wob < wkon){
	        while(*wob != '\n' && wob < wkon){
	            wob++;
	        }
	        if(*wob == '\n')
	        {
	            if(bol == 0){                
	                *wob = 0;
	                exec(wpocz);
	                wpocz = wob + 1;
	            }
	            else{                   
	                bol = 0;
	                wpocz = wob + 1;
	            }
	            wob++;
	        }
	    }    
   // printf("koniec pliku %d\n", (int)(wkon - line));    
	    if((int)(wob - wpocz) == MAX_LINE_LENGTH){
	        if(bol == 0){
	            write(2, "Syntax error.\n", 14);
	        }
	        bol=1;
	        wkon = line;
	        wob = line;
	        wpocz = line;
	    }
	    else{
	        if((line+MAX_LINE_LENGTH) - wkon < 100){
	            int i = 0;            
	            for(i = 0; i <= wob - wpocz; i++){
	                line[i] = *(wpocz+i);
	            }
	            wkon = line + (wob - wpocz);           
	            wob = wkon;            
	            wpocz = line;
	        }     
	    }      
    	if(le == 0)
        	exit(1);
	}
}