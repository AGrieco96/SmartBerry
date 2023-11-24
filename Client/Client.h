#ifndef CLIENT_H
	#define CLIENT_H

	// Macro to simplify error handling


	#define GENERIC_ERROR_HELPER(cond, errCode, msg) do {               \
        if (cond) {                                                 \
            fprintf(stderr, "%s: %s\n", msg, strerror(errCode));    \
            exit(EXIT_FAILURE);                                     \
        }                                                           \
    } while(0)

	#define ERROR_HELPER(ret, msg)          GENERIC_ERROR_HELPER((ret < 0), errno, msg)
	#define PTHREAD_ERROR_HELPER(ret, msg)  GENERIC_ERROR_HELPER((ret != 0), ret, msg)

    // Configuration parameters 

	#define DEBUG           1   
	#define SERVER_ADDRESS  "192.168.1.6"
	#define SERVER_COMMAND  "quit"
	#define SERVER_PORT     2015

    #define BLINK			"blink"
    #define BLINK_LEN 		strlen(BLINK)
    #define PWMLED          "pwmled"
    #define PWMLED_LEN      strlen(PWMLED)
    #define TEMP            "temperature"
    #define TEMP_LEN        strlen(TEMP)
    #define HELP            "help"
    #define HELP_LEN        strlen(HELP)
    #define ULTRASONIC	    "ultrasonic"
    #define ULTRASONIC_LEN  strlen(ULTRASONIC)		

    // Boolean

    #define FALSE 			0
    #define TRUE 			1

    

    
#endif
