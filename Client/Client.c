#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>  // htons() and inet_addr()
#include <netinet/in.h> // struct sockaddr_in
#include <sys/socket.h>
#include <fcntl.h>
#include <ctype.h>
#include <signal.h>

#include "Client.h"

// Variables

int control = FALSE;
int socket_desc;
int fd;

//Signal Handler function

void signal_handler(int sig){
    int ret;
    ret = close(socket_desc);
    ERROR_HELPER(ret, "Cannot close socket");

    if (DEBUG){ 
        ret = close(fd);
        ERROR_HELPER(ret , "Could not close error.log");
    }
    printf("Exiting...\n");
    exit(EXIT_SUCCESS);
}

//MAIN FUNCTION

int main(int argc, char const *argv[]){
	
    int ret;
    (void) signal(SIGINT, signal_handler);

    //Error Log file handling
    
	if(DEBUG){
		fd = open("error.log" , O_CREAT|O_WRONLY|O_TRUNC);
		ERROR_HELPER(fd , "Could not create error.log");
		ret = dup2(fd,2);
		ERROR_HELPER(ret , "Could not redirect stderr");
	}

    // Variables for handling a socket

    struct sockaddr_in server_addr = {0}; 

    // Create a socket


    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    ERROR_HELPER(socket_desc, "Could not create socket");


    // Set up parameters for the connection

    server_addr.sin_addr.s_addr = inet_addr(SERVER_ADDRESS);
    server_addr.sin_family      = AF_INET;
    server_addr.sin_port        = htons(SERVER_PORT); 
    
    
    // Initiate a connection on the socket
    ret = connect(socket_desc, (struct sockaddr*) &server_addr, sizeof(struct sockaddr_in));
    ERROR_HELPER(ret, "Could not create connection");
    if (DEBUG) 
        fprintf(stderr, "Connection established!\n");
     
    // Useful variables

    char buf[1024];
    size_t buf_len = sizeof(buf);
    size_t msg_len;
    int i;

    // Quit command

    char* quit_command = SERVER_COMMAND;
    size_t quit_command_len = strlen(quit_command);

    // Clear buf

    memset(buf , 0 , buf_len);
    if(DEBUG)
    	fprintf(stderr, "buf: %s\n", buf);

    // Receive welcome message from server

    while ( (msg_len = recv(socket_desc, buf, buf_len - 1, 0)) < 0 ) {
        if (errno == EINTR) continue;
        ERROR_HELPER(-1, "Cannot read from socket");
    }

    buf[msg_len] = '\0';

    // Display welcome message

    printf("%s", buf);

    //MAIN LOOP

    while(1){
        do{ 
    		// Printf option

            if(control)
                printf("Ops... \nInsert valid option, please.\n");
            control = FALSE;
            printf("-Blink\n-PwmLed\n-Temperature\n-Ultrasonic\n-Help\n-Quit\nInsert option: ");   // <----- INSERIRE FUNZIONI RASPY!!!!!
            
            // Clear buf
    		
            memset(buf , 0 , buf_len);
    		if(DEBUG)
    			fprintf(stderr, "buf: %s\n", buf);
    		
            // Read from stdin(Keyboard Input)

    		if (fgets(buf, sizeof(buf), stdin) != (char*)buf) {
                fprintf(stderr, "Error while reading from stdin, exiting...\n");
                exit(EXIT_FAILURE);
            }
            msg_len = strlen(buf);
            buf[--msg_len] = '\0';              // Remove '\n' from the end of the message for checking
            if(DEBUG)
            	fprintf(stderr, "buf: %s\n", buf);
           
            // To lower case 

            for ( i = 0; i < msg_len; ++i){
            	buf[i] = tolower(buf[i]);
            }
            control = TRUE;
            
            // MAIN OPTION WHILE
         
        }while(!(msg_len == quit_command_len && !memcmp(buf, quit_command, quit_command_len))  &&  //not QUIT
                    !(msg_len == BLINK_LEN && !memcmp(buf, BLINK,BLINK_LEN))                   &&
                    !(msg_len == PWMLED_LEN && !memcmp(buf,PWMLED,PWMLED_LEN))                 &&
                    !(msg_len == TEMP_LEN && !memcmp(buf,TEMP,TEMP_LEN))                       &&
                    !(msg_len == HELP_LEN && !memcmp(buf,HELP,HELP_LEN))                       &&    //  <---INSERIRE QUI FUNZIONI RASPY
                    !(msg_len == ULTRASONIC_LEN && !memcmp(buf,ULTRASONIC,ULTRASONIC_LEN))

                );		// Checking request
        
        control = FALSE;

        // Send option to server
        
        while ( (ret = send(socket_desc, buf, msg_len, 0)) < 0) {
            if (errno == EINTR) continue;
            ERROR_HELPER(-1, "Cannot write to socket");
        }

        

        // Quit command control

        if (msg_len == quit_command_len && !memcmp(buf, quit_command, quit_command_len)) break;


        //         INSERIRE QUI LE FUNZIONI RASPY!!!!!!!!

        //If Blink

        else if (msg_len == BLINK_LEN && !memcmp(buf, BLINK, BLINK_LEN)){
        	
        		//Clear buf

        		memset(buf , 0 , buf_len);
        		if(DEBUG)
        			fprintf(stderr, "buf: %s\n", buf);

                //Receive Blink Mess
                
                while ( (msg_len = recv(socket_desc, buf, buf_len, 0)) < 0 ) {
                    if (errno == EINTR) continue;
                    ERROR_HELPER(-1, "Cannot read from socket");
            }
              
            printf("%s", buf);
            //Clear buf
            memset(buf , 0 , buf_len);
            if(DEBUG)
            	fprintf(stderr, "buf: %s\n", buf);

            memset(buf , 0 , buf_len);
            if (fgets(buf, sizeof(buf), stdin) != (char*)buf) {
                fprintf(stderr, "Error while reading from stdin, exiting...\n");
                exit(EXIT_FAILURE);
            }

            msg_len = strlen(buf);
            buf[--msg_len] = '\0'; // remove '\n' from the end of the message
            if(DEBUG)
            	fprintf(stderr, "buf: %s\n", buf);

            // Send BlinkONOFF to server
            while ( (ret = send(socket_desc, buf, msg_len, 0)) < 0) {
                if (errno == EINTR) continue;
                ERROR_HELPER(-1, "Cannot write to socket");
            }

            //Clear buf

            memset(buf , 0 , buf_len);
            if(DEBUG)
            	fprintf(stderr, "buf: %s\n", buf);

            // waiting for "Well done" message

            memset(buf , 0 , buf_len);
            while ( (msg_len = recv(socket_desc, buf, buf_len, 0)) < 0 ) {
                if (errno == EINTR) continue;
                ERROR_HELPER(-1, "Cannot read from socket");
            }
            printf("%s \n", buf); // no need to insert '\0'
        }               //BLINK DONE

                        //PWMLED      !!!!!!!!!

        else if (msg_len == PWMLED_LEN && !memcmp(buf, PWMLED, PWMLED_LEN)){
        	
        		//Clear buf

        		memset(buf , 0 , buf_len);
        		if(DEBUG)
        			fprintf(stderr, "buf: %s\n", buf);

                //Receive Blink Mess
                
                while ( (msg_len = recv(socket_desc, buf, buf_len, 0)) < 0 ) {
                    if (errno == EINTR) continue;
                    ERROR_HELPER(-1, "Cannot read from socket");
            }
              
            printf("%s", buf);
            //Clear buf
            memset(buf , 0 , buf_len);
            if(DEBUG)
            	fprintf(stderr, "buf: %s\n", buf);

            memset(buf , 0 , buf_len);
            if (fgets(buf, sizeof(buf), stdin) != (char*)buf) {
                fprintf(stderr, "Error while reading from stdin, exiting...\n");
                exit(EXIT_FAILURE);
            }

            msg_len = strlen(buf);
            buf[--msg_len] = '\0'; // remove '\n' from the end of the message
            if(DEBUG)
            	fprintf(stderr, "buf: %s\n", buf);

            // Send PWM_ONOFF to server
            while ( (ret = send(socket_desc, buf, msg_len, 0)) < 0) {
                if (errno == EINTR) continue;
                ERROR_HELPER(-1, "Cannot write to socket");
            }

            //Clear buf

            memset(buf , 0 , buf_len);
            if(DEBUG)
            	fprintf(stderr, "buf: %s\n", buf);

            // waiting for "Well done" message

            memset(buf , 0 , buf_len);
            while ( (msg_len = recv(socket_desc, buf, buf_len, 0)) < 0 ) {
                if (errno == EINTR) continue;
                ERROR_HELPER(-1, "Cannot read from socket");
            }
            printf("%s \n", buf); // no need to insert '\0'
        }

            // TEMP FUNCTION !!!!!!

        else if (msg_len == TEMP_LEN && !memcmp(buf, TEMP, TEMP_LEN)){

                
        	
        		//Clear buf

        		memset(buf , 0 , buf_len);
        		if(DEBUG)
        			fprintf(stderr, "buf: %s\n", buf);

                //Receive TEMP Mess
                
                while ( (msg_len = recv(socket_desc, buf, buf_len, 0)) < 0 ) {
                    if (errno == EINTR) continue;
                    ERROR_HELPER(-1, "Cannot read from socket");
            }
            printf("%s\n", buf);
        }

        // Help Command

        else if (msg_len == HELP_LEN && !memcmp(buf, HELP, HELP_LEN)){

                printf(" Need help for what? \n-Blink\n-PWMLed\n-Temperature\nInsert Option: ");

                //Clear buf

        		memset(buf , 0 , buf_len);
        		if(DEBUG)
        			fprintf(stderr, "buf: %s\n", buf);

                if (fgets(buf, sizeof(buf), stdin) != (char*)buf) {
                    fprintf(stderr, "Error while reading from stdin, exiting...\n");
                    exit(EXIT_FAILURE);
                }
                msg_len = strlen(buf);
                buf[--msg_len] = '\0';              // Remove '\n' from the end of the message for checking
                if(DEBUG)
            	    fprintf(stderr, "buf: %s\n", buf);
           
                // To lower case 

                for ( i = 0; i < msg_len; ++i){
            	    buf[i] = tolower(buf[i]);
                 }

                
                
                if (msg_len == BLINK_LEN && !memcmp(buf,BLINK,BLINK_LEN)){
                    printf("---Blink function can set On or Off a led connected at Raspberry---\n");
                }
                else if (msg_len == PWMLED_LEN && !memcmp(buf,PWMLED,PWMLED_LEN)){
                    printf("---PWMLED function is used to set On/Off a led Blinking continuosly---\n");
                }
                else if (msg_len == TEMP_LEN && !memcmp(buf,TEMP,TEMP_LEN)){
                    printf("---Temperature function is used to get temperature and umidity---\n");
                }

        }
        
        else if (msg_len == ULTRASONIC_LEN && !memcmp(buf, ULTRASONIC, ULTRASONIC_LEN)){
        	
        		//Clear buf
                
                

        		memset(buf , 0 , buf_len);
        		if(DEBUG)
        			fprintf(stderr, "buf: %s\n", buf);

                //Receive ultrasonic Mess
                
                while ( (msg_len = recv(socket_desc, buf, buf_len, 0)) < 0 ) {
                    if (errno == EINTR) continue;
                    ERROR_HELPER(-1, "Cannot read from socket");
            }
              
            printf("%s", buf);
            //Clear buf
            memset(buf , 0 , buf_len);
            if(DEBUG)
            	fprintf(stderr, "buf: %s\n", buf);

            memset(buf , 0 , buf_len);
            if (fgets(buf, sizeof(buf), stdin) != (char*)buf) {
                fprintf(stderr, "Error while reading from stdin, exiting...\n");
                exit(EXIT_FAILURE);
            }

            msg_len = strlen(buf);
            buf[--msg_len] = '\0'; // remove '\n' from the end of the message
            if(DEBUG)
            	fprintf(stderr, "buf: %s\n", buf);

            // Send ULTRASONICON_OFF to server
            while ( (ret = send(socket_desc, buf, msg_len, 0)) < 0) {
                if (errno == EINTR) continue;
                ERROR_HELPER(-1, "Cannot write to socket");
            }

            //Clear buf

            memset(buf , 0 , buf_len);
            if(DEBUG)
            	fprintf(stderr, "buf: %s\n", buf);

            // waiting for "Well done" message

            memset(buf , 0 , buf_len);
            while ( (msg_len = recv(socket_desc, buf, buf_len, 0)) < 0 ) {
                if (errno == EINTR) continue;
                ERROR_HELPER(-1, "Cannot read from socket");
            }
            printf("%s \n", buf); // no need to insert '\0'
        }        
        
        


    

    // Close the socket
    }
    ret = close(socket_desc);
    ERROR_HELPER(ret, "Cannot close socket");

    if (DEBUG){ 
    	ret = close(fd);
    	ERROR_HELPER(ret , "Could not close error.log");
	}

	printf("Exiting...\n");

    exit(EXIT_SUCCESS);
}
