#include <wiringPi.h>
#include <stdio.h>
#include <sys/time.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include "UltrasonicDetection.h"
#include "Sendmail.h"

#define trigPin 4               // Define the trigger pin
#define echoPin 5               // Define echo pin
#define MAX_DISTANCE 220        // Define the maximum measured distance
#define timeOut MAX_DISTANCE*60 // Calculate timeout according to the maximum measured distance

int TEST;
pthread_t On_thread;

float getSonar(){               // Get the measurement result of ultrasonic module in cm
    long pingTime;
    float distance;
    digitalWrite(trigPin,HIGH); // Send 10us high level to trigPin 
    delayMicroseconds(10);
    digitalWrite(trigPin,LOW);
    pingTime = pulseIn(echoPin,HIGH,timeOut);   // Read plus time of echoPin
    distance = (float)pingTime * 340.0 / 2.0 / 10000.0;  // Calculate distance with sound speed 340m/s
    return distance;
}

int UltrasonicDetection(int On_Off){
    
    int ret;
    TEST=On_Off;
    
    ret=pthread_create(&On_thread,NULL,LoopControl,NULL);       // Create loop thread
	PTHREAD_ERROR_HELPER(ret, "Could not create the thread");
	
	ret = pthread_detach(On_thread);             // Detach for reuse memory
    PTHREAD_ERROR_HELPER(ret, "Could not detach the thread");
}

void* LoopControl(void* arg){
    int i=0;
    wiringPiSetup();
    pinMode(trigPin,OUTPUT);
    pinMode(echoPin,INPUT);
    float distance = getSonar();
    float old_distance;
    while(TEST==1){
        i++;
        old_distance=distance;
        
        distance = getSonar();
    
        if(distance!=old_distance){
            
            puts("ALLARME");
            SendMail();                 // Send an alarm email to the owner
            puts("DONE");
            pthread_exit(NULL);
        }
        delay(100);
    }
    pthread_exit(NULL);
}   
    

int pulseIn(int pin, int level, int timeout){
    
   struct timeval tn, t0, t1;
   long micros;
   gettimeofday(&t0, NULL);
   micros = 0;
   while (digitalRead(pin) != level)
   {
      gettimeofday(&tn, NULL);
      if (tn.tv_sec > t0.tv_sec) micros = 1000000L; else micros = 0;
      micros += (tn.tv_usec - t0.tv_usec);
      if (micros > timeout) return 0;
   }
   gettimeofday(&t1, NULL);
   while (digitalRead(pin) == level)
   {
      gettimeofday(&tn, NULL);
      if (tn.tv_sec > t0.tv_sec) micros = 1000000L; else micros = 0;
      micros = micros + (tn.tv_usec - t0.tv_usec);
      if (micros > timeout) return 0;
   }
   if (tn.tv_sec > t1.tv_sec) micros = 1000000L; else micros = 0;
   micros = micros + (tn.tv_usec - t1.tv_usec);
   return micros;
}
