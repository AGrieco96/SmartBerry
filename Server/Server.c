#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>     //per i segnali
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>  // htons()
#include <netinet/in.h> // struct sockaddr_in
#include <sys/socket.h>
#include <semaphore.h>
#include <dirent.h>
#include <fcntl.h>
#include <limits.h>

#include "Server.h"
#include "Temperature.h"
#include "Blink.h"
#include "PWMLed.h"
#include "UltrasonicDetection.h"


//Semaphore
sem_t Rasputin;

//struct thread arg
typedef struct handler_args_s{
    int socket_desc;
    struct sockaddr_in *client_addr;
}    handler_args_t;



int controlInput(char* buf , int* value){
	char* end;
	int n;
	errno = 0;
	const long sl = strtol(buf , &end , 10);

	
	if (end == buf) {
		if(DEBUG)
    		fprintf(stderr, "%s: not a decimal number\n", buf);
  	}else if ('\0' != *end) {
    	if(DEBUG)
    		fprintf(stderr, "%s: extra characters at end of input: %s\n", buf, end);
  	}else if ((LONG_MIN == sl || LONG_MAX == sl) && ERANGE == errno) {
    	if(DEBUG)
    		fprintf(stderr, "%s out of range of type long\n", buf);
  	}else if (sl > INT_MAX) {
    	if(DEBUG)
    		fprintf(stderr, "%ld greater than INT_MAX\n", sl);
  	}else if (sl < INT_MIN) {
    	if(DEBUG)
    		fprintf(stderr, "%ld less than INT_MIN\n", sl);
  	}else {
    	*value = (int)sl;
    	return TRUE;
    }
    return FALSE;
}



//Connection Handler

void* connection_handler(void* arg){
    //Variables

    int ret;
    int i;
    int index;
    int recv_bytes;
    int blinkON_OFF;
    int PWMON_OFF;
    int* temperature_array[4];
    int ULTRASONICON_OFF;


    //Thread Args

    handler_args_t *args=(handler_args_t*) arg;
    int socket_desc = args->socket_desc;
    struct sockaddr_in *client_addr = args->client_addr;

    //parse client IP address and port

    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(client_addr->sin_addr), client_ip, INET_ADDRSTRLEN);
    uint16_t client_port = ntohs(client_addr->sin_port); 

    //Local Variables

    char buf[1024];
    size_t buf_len=sizeof(buf);
    size_t msg_len;
    char* quit_command=SERVER_COMMAND;
    size_t quit_command_len=strlen(quit_command);

    //Control

    int control;

    //Initialize Welcome Messagge

    memset(buf,0,buf_len);
    if(DEBUG)
        fprintf(stderr,"buf:%s \n",buf);
    sprintf(buf , WELCOME_MESS);
    msg_len = strlen(buf);
    if(DEBUG) 
    	fprintf(stderr, "buf: %s\n", buf);

    //Send Welcome Message

    while(ret=(send(socket_desc,buf,msg_len,0)) <0 ){
        if(errno==EINTR) continue;
        ret=close(socket_desc);

        //Free Buffer

        free(args->socket_desc);
        free(args);

        //Exit

        pthread_exit(NULL);
    }

    //MAIN LOOP

    while(1){
        control = TRUE;
    	if(DEBUG)
    		fprintf(stderr, "Waiting options...\n");

    	//Clear buf

    	memset(buf , 0 , buf_len);
    	if(DEBUG) 
    		fprintf(stderr, "buf: %s\n", buf);

        //Receive option from client

    	while ((recv_bytes = recv(socket_desc, buf, buf_len, 0)) < 0){
            if (errno == EINTR) continue;
            ret = close(socket_desc);

        	//Free buffer

    		free(args->client_addr); 
    		free(args);

    		//Exit

    		pthread_exit(NULL);
        }
        //Setting last= '\0'

        buf[recv_bytes] = '\0';
        if(DEBUG) 
    		fprintf(stderr, "buf: %s\n", buf);

        // Check for quit message

        if (recv_bytes == 0) break;
        if (recv_bytes == quit_command_len && !memcmp(buf, quit_command, quit_command_len)) break;

        //Blinking Led 


        if(recv_bytes==BLINK_LEN && !memcmp(buf,BLINK,BLINK_LEN)){

            
            // Clear buf

            memset(buf,0,buf_len);
            if(DEBUG)
                fprintf(stderr,"buf: %s\n",buf);

            // Initialize buf
           
            sprintf(buf,BLINK_MESS);
            msg_len=strlen(buf);
            if(DEBUG)
                fprintf(stderr,"buf: %s\n",buf);
            
            // Send Blink mess
    		while ((ret = send(socket_desc, buf, msg_len, 0)) < 0){
        		if (errno == EINTR) continue;
        		ret = close(socket_desc);
        		// Free buffer
    			free(args->client_addr); 
    			free(args);
    			//Exit
    			pthread_exit(NULL);
    		}

            //Clear buf
    		memset(buf , 0 , buf_len);
    		if(DEBUG) 
    			fprintf(stderr, "buf: %s\n", buf);

            //Receive ON/OFF option

            while ((recv_bytes = recv(socket_desc, buf, buf_len, 0)) < 0){
            	if (errno == EINTR) continue;
            	ret = close(socket_desc);
        		//Free buffer
    			free(args->client_addr); 
    			free(args);
    			//Exit
    			pthread_exit(NULL);
        	}

            buf[recv_bytes] = '\0';
        	if(DEBUG) 
    			fprintf(stderr, "buf: %s\n", buf);
            
            // Input control

    		control = controlInput(buf , &blinkON_OFF);
    		if(control && DEBUG)                                    // AGGIUSTARE QUI
    			fprintf(stderr, "On/OFF: %d\n", blinkON_OFF);
            if(control && (blinkON_OFF<0 || blinkON_OFF>1))
    			control = FALSE;

            if(control){

    			// Wait on semaphore
    		    ret = sem_wait(&Rasputin);
	            ERROR_HELPER(ret, "Wait on semaphore Rasputin");
	            if(DEBUG)
	            	fprintf(stderr , "ON Semaphore\n");
	            /************************
	            *                       *
	            *   Blinking Function   *
	            *                       *
	            ************************/
				Blink(blinkON_OFF);

	            //Post on semaphore

	            ret = sem_post(&Rasputin);
	            ERROR_HELPER(ret, "Wait on semaphore Rasputin");
    		}

            // Clear buf
    		memset(buf , 0 , buf_len);
    		if(DEBUG) 
    			fprintf(stderr, "buf: %s\n", buf);
    		//Initialize welldone/nogood mess
    		if(control)
    			sprintf(buf , WELLDONE_MESS);
    		else
    			sprintf(buf , NOGOOD_MESS);
    		msg_len = strlen(buf);
    		if(DEBUG) 
    			fprintf(stderr, "buf: %s\n", buf);
    		// Send weel done mess
    		while ((ret = send(socket_desc, buf, msg_len, 0)) < 0){
        		if (errno == EINTR) continue;
        		ret = close(socket_desc);
        		//free buffer
    			free(args->client_addr); 
    			free(args);
    			//exit
    			pthread_exit(NULL);
    		}
    		control = TRUE;
        }

            //BLINK END             
            

            //PWM LED FUNCTION       

            else if(recv_bytes==PWMLED_LEN && !memcmp(buf,PWMLED,PWMLED_LEN)){

            
            // Clear buf

            memset(buf,0,buf_len);
            if(DEBUG)
                fprintf(stderr,"buf: %s\n",buf);

            // Initialize buf
           
            sprintf(buf,PWMLED_MESS);
            msg_len=strlen(buf);
            if(DEBUG)
                fprintf(stderr,"buf: %s\n",buf);
            
            // Send PWM mess
    		while ((ret = send(socket_desc, buf, msg_len, 0)) < 0){
        		if (errno == EINTR) continue;
        		ret = close(socket_desc);
        		// Free buffer
    			free(args->client_addr); 
    			free(args);
    			//Exit
    			pthread_exit(NULL);
    		}

            //Clear buf
    		memset(buf , 0 , buf_len);
    		if(DEBUG) 
    			fprintf(stderr, "buf: %s\n", buf);

            //Receive ON/OFF option

            while ((recv_bytes = recv(socket_desc, buf, buf_len, 0)) < 0){
            	if (errno == EINTR) continue;
            	ret = close(socket_desc);
        		//Free buffer
    			free(args->client_addr); 
    			free(args);
    			//Exit
    			pthread_exit(NULL);
        	}

            buf[recv_bytes] = '\0';
        	if(DEBUG) 
    			fprintf(stderr, "buf: %s\n", buf);
            
            // Input control

    		control = controlInput(buf , &PWMON_OFF);
    		if(control && DEBUG)                                    // AGGIUSTARE QUI
    			fprintf(stderr, "On/OFF: %d\n", PWMON_OFF);
            if(control && (PWMON_OFF<0 || PWMON_OFF>1))
    			control = FALSE;

            if(control){

    			// Wait on semaphore
    		    ret = sem_wait(&Rasputin);
	            ERROR_HELPER(ret, "Wait on semaphore Rasputin");
	            if(DEBUG)
	            	fprintf(stderr , "ON Semaphore\n");
	            /************************
	            *                       *
	            *   PWM FUNCTION        *
	            *                       *
	            ************************/
				PWMLed(PWMON_OFF);

	            //Post on semaphore

	            ret = sem_post(&Rasputin);
	            ERROR_HELPER(ret, "Wait on semaphore Rasputin");
    		}

            // Clear buf
    		memset(buf , 0 , buf_len);
    		if(DEBUG) 
    			fprintf(stderr, "buf: %s\n", buf);
    		//Initialize welldone/nogood mess
    		if(control)
    			sprintf(buf , WELLDONE_MESS);
    		else
    			sprintf(buf , NOGOOD_MESS);
    		msg_len = strlen(buf);
    		if(DEBUG) 
    			fprintf(stderr, "buf: %s\n", buf);
    		// Send well done mess
    		while ((ret = send(socket_desc, buf, msg_len, 0)) < 0){
        		if (errno == EINTR) continue;
        		ret = close(socket_desc);
        		//free buffer
    			free(args->client_addr); 
    			free(args);
    			//exit
    			pthread_exit(NULL);
    		}
    		control = TRUE;
        }



        //         TEMPERATURE FUNCTION


         else if(recv_bytes==TEMP_LEN && !memcmp(buf,TEMP,TEMP_LEN)){

             puts("SONO IN TEMP");
            
            // Clear buf

            memset(buf,0,buf_len);
            if(DEBUG)
                fprintf(stderr,"buf: %s\n",buf);

            // Wait on semaphore
    		ret = sem_wait(&Rasputin);
	        ERROR_HELPER(ret, "Wait on semaphore Rasputin");
	        if(DEBUG)
	            fprintf(stderr , "ON Semaphore\n");
	        /************************
	        *                       *
	        *   TEMPERATURE FUNCTION        *
	        *                       *
	        ************************/
			temperature(temperature_array);

	        //Post on semaphore

	        ret = sem_post(&Rasputin);
	        ERROR_HELPER(ret, "Wait on semaphore Rasputin");

            // Initialize buf
           
            sprintf(buf,TEMP_MESS);
            msg_len=strlen(buf);
            if(DEBUG)
                fprintf(stderr,"buf: %s\n",buf);
            
            // Send TEMP mess
    		while ((ret = send(socket_desc, buf, msg_len, 0)) < 0){
        		if (errno == EINTR) continue;
        		ret = close(socket_desc);
        		// Free buffer
    			free(args->client_addr); 
    			free(args);
    			//Exit
    			pthread_exit(NULL);
    		}
            control=TRUE;
         }
         
         //      UltrasonicDetection 
         
         else if(recv_bytes==ULTRASONIC_LEN && !memcmp(buf,ULTRASONIC,ULTRASONIC_LEN)){
             
            
            // Clear buf

            memset(buf,0,buf_len);
            if(DEBUG)
                fprintf(stderr,"buf: %s\n",buf);

            // Initialize buf
           
            sprintf(buf,ULTRASONIC_MESS);
            msg_len=strlen(buf);
            if(DEBUG)
                fprintf(stderr,"buf: %s\n",buf);
            
            // Send ultrasonic mess
    		while ((ret = send(socket_desc, buf, msg_len, 0)) < 0){
        		if (errno == EINTR) continue;
        		ret = close(socket_desc);
        		// Free buffer
    			free(args->client_addr); 
    			free(args);
    			//Exit
    			pthread_exit(NULL);
    		}
            

            //Clear buf
    		memset(buf , 0 , buf_len);
    		if(DEBUG) 
    			fprintf(stderr, "buf: %s\n", buf);

            //Receive ON/OFF option

            while ((recv_bytes = recv(socket_desc, buf, buf_len, 0)) < 0){
            	if (errno == EINTR) continue;
            	ret = close(socket_desc);
        		//Free buffer
    			free(args->client_addr); 
    			free(args);
    			//Exit
    			pthread_exit(NULL);
        	}

            buf[recv_bytes] = '\0';
        	if(DEBUG) 
    			fprintf(stderr, "buf: %s\n", buf);
            
            // Input control

    		control = controlInput(buf , &ULTRASONICON_OFF);
    		if(control && DEBUG)                                    
    			fprintf(stderr, "On/OFF: %d\n", ULTRASONICON_OFF);
            if(control && (ULTRASONICON_OFF<0 || ULTRASONICON_OFF>1))
    			control = FALSE;

            if(control){

    			// Wait on semaphore
    		    ret = sem_wait(&Rasputin);
	            ERROR_HELPER(ret, "Wait on semaphore Rasputin");
	            if(DEBUG)
	            	fprintf(stderr , "ON Semaphore\n");
	            /************************
	            *                       *
	            *   ULTRASONIC FUNCTION *
	            *                       *
	            ************************/
                
              
				UltrasonicDetection(ULTRASONICON_OFF);

	            //Post on semaphore

	            ret = sem_post(&Rasputin);
	            ERROR_HELPER(ret, "Wait on semaphore Rasputin");
    		}

            // Clear buf
    		memset(buf , 0 , buf_len);
    		if(DEBUG) 
    			fprintf(stderr, "buf: %s\n", buf);
    		//Initialize welldone/nogood mess
    		if(control)
    			sprintf(buf , WELLDONE_MESS);
    		else
    			sprintf(buf , NOGOOD_MESS);
    		msg_len = strlen(buf);
    		if(DEBUG) 
    			fprintf(stderr, "buf: %s\n", buf);
    		// Send well done mess
    		while ((ret = send(socket_desc, buf, msg_len, 0)) < 0){
        		if (errno == EINTR) continue;
        		ret = close(socket_desc);
        		//free buffer
    			free(args->client_addr); 
    			free(args);
    			//exit
    			pthread_exit(NULL);
    		}
    		control = TRUE;
        }
		//Add here other Raspy function!
    }

    // Close socket

    ret = close(socket_desc);
    ERROR_HELPER(ret, "Cannot close socket for incoming connection");
    if (DEBUG)
        fprintf(stderr, "Thread created to handle the request has completed.\n");

    //Free buffer
    
    free(args->client_addr); 
    free(args);
    
    //Exit
    
    pthread_exit(NULL);
}       //END OF CONNECTION_HANDLER FUNCTION



//MAIN FUNCTION

int main(int argc,char* argv[]){
    //Variables
    int ret;
    int socket_desc, client_desc;
    struct sockaddr_in server_addr = {0};
    int sockaddr_len = sizeof(struct sockaddr_in);

    //Semaphore initialized
    
    ret=sem_init(&Rasputin,0,1);
    ERROR_HELPER(ret,"Failed to inizialize semaphore");

    // Initialize socket for listening

    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    ERROR_HELPER(socket_desc, "Could not create socket");

    server_addr.sin_addr.s_addr = INADDR_ANY; // Want to accept connections from any interface
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);


    // Enable SO_REUSEADDR to quickly restart server after a crash

    int reuseaddr_opt = 1;
    ret = setsockopt(socket_desc, SOL_SOCKET, SO_REUSEADDR, &reuseaddr_opt, sizeof(reuseaddr_opt));
    ERROR_HELPER(ret, "Cannot set SO_REUSEADDR option");

    // Bind address to socket
    ret = bind(socket_desc, (struct sockaddr *)&server_addr, sockaddr_len);
    ERROR_HELPER(ret, "Cannot bind address to socket");

    // Start listening
    ret = listen(socket_desc, MAX_CONN_QUEUE);
    ERROR_HELPER(ret, "Cannot listen on socket");

    // Allocate client_addr dynamically and initialize it to zero
    struct sockaddr_in *client_addr = calloc(1, sizeof(struct sockaddr_in));

    // Loop to manage incoming connections spawning handler threads

    while (1){
        // Accept incoming connection

        client_desc = accept(socket_desc, (struct sockaddr *)client_addr, (socklen_t *)&sockaddr_len);
        if (client_desc == -1 && errno == EINTR) continue;           // Check for interruption by signals
        ERROR_HELPER(client_desc, "Cannot open socket for incoming connection");

        if (DEBUG)
            fprintf(stderr, "Incoming connection accepted...\n");
        
        pthread_t thread;

        // Put arguments for the new thread into a buffer
        handler_args_t *thread_args = malloc(sizeof(handler_args_t));
        thread_args->socket_desc = client_desc;
        thread_args->client_addr = client_addr;

        ret = pthread_create(&thread, NULL, connection_handler, (void *)thread_args);
        PTHREAD_ERROR_HELPER(ret, "Could not create a new thread");

        if (DEBUG)
            fprintf(stderr, "New thread created to handle the request!\n");
        
        ret = pthread_detach(thread); // I won't phtread_join() on this thread
        PTHREAD_ERROR_HELPER(ret, "Could not detach the thread");

        // Can't just reset fields: we need a new buffer for client_addr!

        client_addr = calloc(1, sizeof(struct sockaddr_in));
    }

    exit(EXIT_SUCCESS); // this will never be executed


}
