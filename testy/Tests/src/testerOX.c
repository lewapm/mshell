#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>



int 
main(int argc, char** argv)
{
	float fduration=0;
	char * endptr;
	struct timespec ts;
	int res;

	if (argc!=2) {
		printf("Syntax: %s Time_to_wait\n", argv[0]);
		exit(1);
	}
	
	errno=0;
	fduration= strtof(argv[1], &endptr);
	
	if ((*endptr) || (errno)){
		fprintf(stderr,"Cannot parse '%s' as float.\n", argv[1]);
		exit(1);
	}

	int uduration= fduration * 1000000.0;

	if (fduration<0) fduration=0;

	ts.tv_sec=floorf(fduration);
	ts.tv_nsec=fmod(fduration,1)*1000000000;

	res=nanosleep(&ts,&ts);
	

	exit(res);
}
