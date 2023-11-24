#ifndef PWMLED_H
	#define PWMLED_H
	
	// Macro to simplify error handling
	
	#define GENERIC_ERROR_HELPER(cond, errCode, msg) do {           \
        if (cond) {                                                 \
            fprintf(stderr, "%s: %s\n", msg, strerror(errCode));    \
            exit(EXIT_FAILURE);                                     \
        }                                                           \
    } while(0)


    #define ERROR_HELPER(ret, msg)          GENERIC_ERROR_HELPER((ret < 0), errno, msg)
    #define PTHREAD_ERROR_HELPER(ret, msg)  GENERIC_ERROR_HELPER((ret != 0), ret, msg)
    
    
	
	void* pwm_go(void* arg);
	void  PWMLed(int set);
	
	
#endif
