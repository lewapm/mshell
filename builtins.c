#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include "builtins.h"
#include <dirent.h>

int echo(char*[]);
int undefined(char *[]);
int lcd(char*[]);
int lls(char*[]);
int lkill(char*[]);
int lexit(char*[]);

builtin_pair builtins_table[]={
	{"exit",	&lexit},
	{"lecho",	&echo},
	{"lcd",		&lcd},
	{"lkill",	&lkill},
	{"lls",		&lls},
	{NULL,NULL}
};

int lexit(char* argv[]){
	exit(0);
}

int echo( char * argv[])
{
	int i =1;
	if (argv[i]) printf("%s", argv[i++]);
	while  (argv[i])
		printf(" %s", argv[i++]);

	//write(1, "\n", 1);
	printf("\n");
	fflush(stdout);
	return 0;
}

int lcd(char * argv[])
{
	if(argv[2]!=NULL)
		return 1;
	if(argv[1])
		return chdir(argv[1]);
	else{
		char *value = getenv("HOME");			
		return chdir(value); 
	}
	return 0;
}

int lls(char*argv[])
{	
	DIR *dir;
	struct dirent *dp;
	if((dir = opendir(".")) == NULL){
		return 1;
	}
	while((dp = readdir(dir)) != NULL){
		if(dp->d_name[0] != '.'){
			write(1, dp->d_name, strlen(dp->d_name));
			write(1, "\n", 1);
		}
	}
	closedir(dir);
return 0;
}

int lkill(char* argv[]){
	char* nptr[100];
	int k=0;
	if(argv[1]==NULL)
		return 1;
	if(argv[2]){
		int x = strtol(argv[1], nptr, 10);
		if(x == 0 && (errno == EINVAL || errno == ERANGE)){
			return 1;
		}
		int y = strtol(argv[2], nptr, 10);
		if(y == 0 && (errno == EINVAL || errno == ERANGE)){
			return 1;
		}
		x = abs(x);
		k = kill(y, x);
	}	
	else{
		int x = strtol(argv[1], nptr, 10);
		if(x == 0 && (errno == EINVAL || errno == ERANGE)){
			return 1;
		}
		k = kill(x, 15);
	}
	return k;
}

int 
undefined(char * argv[])
{
	fprintf(stderr, "Command %s undefined.\n", argv[0]);
	return BUILTIN_ERROR;
}
