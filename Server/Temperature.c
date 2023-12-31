#include <wiringPi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "Temperature.h"
#define MAXTIMINGS	85
#define DHTPIN		0
int dht11_dat[5] = { 0, 0, 0, 0, 0 };
int ret[4];
 
void read_dht11_dat(){
	
	uint8_t laststate	= HIGH;
	uint8_t counter		= 0;
	uint8_t j		= 0, i;
	
	
 
	dht11_dat[0] = dht11_dat[1] = dht11_dat[2] = dht11_dat[3] = dht11_dat[4] = 0;
	
		//Setting pins
 
	pinMode( DHTPIN, OUTPUT );
	digitalWrite( DHTPIN, LOW );
	delay( 18 );
	digitalWrite( DHTPIN, HIGH );
	delayMicroseconds( 40 );
	pinMode( DHTPIN, INPUT );
 
			// Cycle for get data from DHT11
			
	for ( i = 0; i < MAXTIMINGS; i++ )
	{
		counter = 0;
		while ( digitalRead( DHTPIN ) == laststate )
		{
			counter++;
			delayMicroseconds( 1 );
			if ( counter == 255 )
			{
				break;
			}
		}
		laststate = digitalRead( DHTPIN );
 
		if ( counter == 255 )
			break;
 
		if ( (i >= 4) && (i % 2 == 0) )
		{
			dht11_dat[j / 8] <<= 1;
			if ( counter > 50 )
				dht11_dat[j / 8] |= 1;
			j++;
		}
	}
 
	if ( (j >= 40) &&
	     (dht11_dat[4] == ( (dht11_dat[0] + dht11_dat[1] + dht11_dat[2] + dht11_dat[3]) & 0xFF) ) )
	{
		
	
			for(i=0;i<4;i++){					//Insert data in ret array
				ret[i]=dht11_dat[i];
			}
				
	}else  {
		printf( "Wait a few seconds please...I'm reading data...\n" );
	}
}
 
void* temperature(int*array){
	
	printf( "Reading Temperature...\n" );
	int i;
 
	if ( wiringPiSetup() == -1 )
		exit( 1 );
 
	while ( 1 )
	{
		read_dht11_dat();		//Read data from DHT11
		
		if(ret[0] !=0 && ret[2] !=0) {
			printf( "Humidity = %d.%d %% Temperature = %d.%d C \n",ret[0],ret[1],ret[2],ret[3]);		//Check result
			for(i=0;i<4;i++){
				array[i]=ret[i];				//Filling result array
			}
				
			break;
		}
			
		delay( 1000 ); 
		
	}
 
}
