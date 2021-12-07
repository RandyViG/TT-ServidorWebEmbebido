#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <signal.h>
#include <syslog.h>

void iniciar_demonio(){
    FILE *apArch;
    pid_t pid = 0;
    pid_t sid = 0;
    
	pid = fork();
    if( pid == -1 ){
   	    perror("Error al crear el primer proceso hijo\n");
        exit(EXIT_FAILURE);
    }
   
	if( pid ){
   	    printf("Se termina proceso padre, PID del proceso hijo %d \n", pid);
        exit(0);
    }
	
    umask(0);
	sid = setsid();
   
	if( sid < 0 ){
   	    perror("Error al iniciar sesion");
        exit(EXIT_FAILURE);
    }

    pid = fork( );
    if( pid == -1 ){
        perror("Error al crear el segundo proceso hijo\n");
        exit(EXIT_FAILURE);
    }
    if( pid ){
   	    printf("PID del segundo proceso hijo %d \n", pid);
        apArch = fopen("/var/run/servidor_web_embebido.pid", "w");
        fprintf(apArch, "%d", pid);
        fclose(apArch);

        exit(0);
	}
	
	chdir("/");
	
	close( STDIN_FILENO  );
	close( STDOUT_FILENO );
	close( STDERR_FILENO );
   
	openlog( "servidor_web_embebido", LOG_NDELAY | LOG_PID, LOG_LOCAL0 );
}