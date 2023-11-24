#include <wiringPi.h>
#include <stdio.h>

#define  ledPin    0	                // Define the led pin number

void Blink(int ret){

	
	wiringPiSetup();	                //Initialize wiringPi
	
	pinMode(ledPin, OUTPUT);            //Set the pin mode
	
    if(ret==0){
        digitalWrite(ledPin,HIGH);
        printf("LED TURNED ON \n");
    }
    else if(ret==1){
        digitalWrite(ledPin,LOW);
        printf("LED turned OFF \n");

    }
    
}
