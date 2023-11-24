#include <stdio.h>
#include <string.h>

#include "Sendmail.h"


void SendMail(){
	
	    char cmd[100];  // To use the command
        char to[] = "typeyourmailhere@test.com"; // Email id of the owner
        char body[] = "            -ATTENTION-\n    Movement revealed by Ultrasonic Alarm!!!";    // Allarm message
        char alarmFile[100];     // Name of alarmFile.

        strcpy(alarmFile,tempnam("/tmp","sendmail")); // Generate alarm file name

        FILE *fp = fopen(alarmFile,"w"); // Open alarmFile for writing
        fprintf(fp,"%s\n",body);        // Write alarm message to it
        fclose(fp);             // Close alarm file

        sprintf(cmd,"sendmail %s < %s",to,alarmFile); // Prepare sendmail command to send email
        system(cmd);     // Execute command on shell
}
