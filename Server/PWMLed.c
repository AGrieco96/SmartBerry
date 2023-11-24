#include <wiringPi.h>
#include <stdio.h>
#include <softPwm.h>
#include <pthread.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "PWMLed.h"

#define ledPin    1 

pthread_t On_thread;

int tester;

void* pwm_go(void* arg){
	
	wiringPiSetup();
	softPwmCreate(ledPin,0,100);
	int i;
	
	while(tester==1){
		
		for(i=0;i<100;i++){               	// Make the led brighter
			softPwmWrite(ledPin, i); 
			delay(10);						// Delay for control speed of blinking
		}
		delay(100);
		for(i=100;i>=0;i--){  				// Make the led darker
			softPwmWrite(ledPin, i);
			delay(10);
		}
		delay(100);							// Delay after a complete cycle
	}
	
	pthread_exit(NULL);						// Exit when tester go to 0
}


void PWMLed(int set){
	
	int ret;
	
	tester=set;
		
	ret=pthread_create(&On_thread,NULL,pwm_go,NULL);			// Crete thread for loop pwm
	PTHREAD_ERROR_HELPER(ret, "Could not create the thread");
	
	ret = pthread_detach(On_thread); 							// Detach for reuse allocated memory
    PTHREAD_ERROR_HELPER(ret, "Could not detach the thread");
	
	
}


	
		
		
		
		



