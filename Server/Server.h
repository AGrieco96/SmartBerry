#ifndef SERVER_H
	#define SERVER_H

	// Macro to simplify error handling

	#define GENERIC_ERROR_HELPER(cond, errCode, msg) do {           \
        if (cond) {                                                 \
            fprintf(stderr, "%s: %s\n", msg, strerror(errCode));    \
            exit(EXIT_FAILURE);                                     \
        }                                                           \
    } while(0)

	#define ERROR_HELPER(ret, msg)          GENERIC_ERROR_HELPER((ret < 0), errno, msg)
	#define PTHREAD_ERROR_HELPER(ret, msg)  GENERIC_ERROR_HELPER((ret != 0), ret, msg)

    // Configuration parameters 

	#define DEBUG               1   // display debug messages
	#define SERVER_COMMAND      "quit"
	#define SERVER_PORT         2015
    #define MAX_CONN_QUEUE      5

    //Boolean

    #define FALSE               0
    #define TRUE                1

    //Server commands

    #define BLINK               "blink"
    #define BLINK_LEN           strlen(BLINK)
    #define PWMLED              "pwmled"
    #define PWMLED_LEN          strlen(PWMLED)
    #define TEMP                "temperature"
    #define TEMP_LEN            strlen(TEMP)
    #define ULTRASONIC		    "ultrasonic"
    #define ULTRASONIC_LEN	    strlen(ULTRASONIC)

    //Messages

    #define WELCOME_MESS        "Hi! I'm the GNM Server :)\nWhat do you want to do? I will stop if you send me quit \n"
    #define WELLDONE_MESS       "Well Done\n"
    #define NOGOOD_MESS         "Invalid input!\n"
    #define BLINK_MESS          "Led ON or OFF  --0 for OFF & 1 for ON: "
    #define PWMLED_MESS         "Type 0 for stopping PWM or 1 for active PWM: "
    #define TEMP_MESS		    "Umidity is: %d.%d %  Temperature is: %d.%d C ",temperature_array[0],temperature_array[1],temperature_array[2],temperature_array[3]
    #define ULTRASONIC_MESS    	"Type 0 for stopping Alarm system and 1 for active it : \n"
    int controlInput(char* buf , int* value);

#endif
