CC = gcc

THREAD = -lpthread

WIRINGPI= -lwiringPi


all:		Server Blink Temperature PWMLed UltrasonicDetection Sendmail final


Server: 	 Server.c Server.h
	$(CC) -c Server.c

Temperature: Temperature.c Temperature.h 
	$(CC) -c Temperature.c 

Blink: 	     Blink.c Blink.h	
	$(CC) -c Blink.c 

PWMLed: 	 PWMLed.c PWMLed.h
	$(CC) -c PWMLed.c 
	
UltrasonicDetection:	UltrasonicDetection.c UltrasonicDetection.h
	$(CC) -c UltrasonicDetection.c
	
Sendmail:    Sendmail.c Sendmail.h
	$(CC) -c Sendmail.c

final: 		
	$(CC) -o Server Server.o Temperature.o PWMLed.o Blink.o UltrasonicDetection.o Sendmail.o  $(THREAD) $(WIRINGPI)

.PHONY:clean

clean:
	rm -f Server *.o 

run:
	./Server
