/*
 * File: hybrid-sched.c
 * Author: Andy Sayler
 * Project: CSCI 3753 Programming Assignment 3
 * Create Date: 2012/03/07
 * Modify Date: 2012/03/09
 * Description:
 * 	This file contains a simple program for statistically
 *      calculating pi using a specific scheduling policy.
 */

//Referenced: stack overflow 19099663/how-to-correctly-use-fork-exec-wait

/* Local Includes */
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <errno.h>
#include <sched.h>
#include <sys/wait.h>

#define DEFAULT_ITERATIONS 1000000
#define RADIUS (RAND_MAX / 2)

inline double dist(double x0, double y0, double x1, double y1){
    return sqrt(pow((x1-x0),2) + pow((y1-y0),2));
}

inline double zeroDist(double x, double y){
    return dist(0, 0, x, y);
}

int main(int argc, char* argv[]){

    long i;
    long iterations;
    struct sched_param param;
    int policy;
    double x, y;
    double inCircle = 0.0;
    double inSquare = 0.0;
    double pCircle = 0.0;
    double piCalc = 0.0;
    int processes;
    int p = 0;
    pid_t pid;
    pid_t parent = getpid();

    /* Process program arguments to select iterations and policy */
    /* Set default iterations if not supplied */
    if(argc < 2){
	iterations = DEFAULT_ITERATIONS;
    }
    /* Set default policy if not supplied */
    if(argc < 3){
	policy = SCHED_OTHER;
    }
    /* Set iterations if supplied */
    if(argc > 1){
	iterations = atol(argv[1]);
	if(iterations < 1){
	    fprintf(stderr, "Bad iterations value\n");
	    exit(EXIT_FAILURE);
	}
    }
    /* Set policy if supplied */
    if(argc > 2){
	if(!strcmp(argv[2], "SCHED_OTHER")){
	    policy = SCHED_OTHER;
	}
	else if(!strcmp(argv[2], "SCHED_FIFO")){
	    policy = SCHED_FIFO;
	}
	else if(!strcmp(argv[2], "SCHED_RR")){
	    policy = SCHED_RR;
	}
	else{
	    fprintf(stderr, "Unhandeled scheduling policy\n");
	    exit(EXIT_FAILURE);
	}
    } 
   
    /* Set process to max prioty for given scheduler */
    param.sched_priority = sched_get_priority_max(policy);
    
    /* Set new scheduler policy */
    fprintf(stdout, "Current Scheduling Policy: %d\n", sched_getscheduler(0));
    fprintf(stdout, "Setting Scheduling Policy to: %d\n", policy);
    if(sched_setscheduler(0, policy, &param)){
	perror("Error setting scheduler policy");
	exit(EXIT_FAILURE);
    }
    fprintf(stdout, "New Scheduling Policy: %d\n", sched_getscheduler(0));

    //Enable Forking:
    if(argv[3]) processes = atoi(argv[3]);
    else processes = 1;
    
    char outputName[80];
    
    //char inputName[80] = argv[4];
    if(argv[4]) strncpy(outputName, argv[4], 80);
    else 
    {
		printf("Missing Output File in Argument\n");
		return 1; 
	}
	
    //char outputName[80] = argv[4];
    
    fprintf(stdout, "My Proccess Count is: %d\n", processes);

    for(p = 0; p < processes; p++) 
    { 
		pid = fork();

        if (pid == -1) return 1;

        else if(pid == 0)
        {
			char processNumber[80];
			
			//char local_inputFilename[80];
            char local_outputFilename[80];
			
			//char execinputString[80];
            //char execoutputString[80];
            //char execoutputbaseString[80];
            
            //FILE * inputFile;
            FILE * outputFile;
            
			sprintf(processNumber, "%d", p);
			
			//strncpy(local_inputFilename, inputFilename, 80);
			strncpy(local_outputFilename, outputName, 80);
			//strncat(local_inputFilename, processNumber, 80);
			strncat(local_outputFilename, processNumber, 80);
			
			            
            //Convert to a Function (running out of time, haha) do later:
            //strncpy(execinputString, "/bin/ls ", 80);
            //strncat(execinputString, local_inputFilename, 80);
            //strncat(execinputString, " || cp ", 80);
            //strncat(execinputString, inputFilename, 80);
            //strncat(execinputString, " ", 80);
            //strncat(execinputString, local_inputFilename, 80);
            
            
            //strncpy(execoutputString, "/bin/ls ", 80);
            //strncat(execoutputString, local_outputFilename, 80);
            //strncat(execoutputString, " || cp ", 80);
            //strncat(execoutputString, outputFilename, 80);
            //strncat(execoutputString, " ", 80);
            //strncat(execoutputString, local_outputFilename, 80);		
			
	        //fprintf(stdout, "I am pid: %d\n", pid);
	    
            /* Calculate pi using statistical methode across all iterations*/
            for(i=0; i<iterations; i++){
	            x = (random() % (RADIUS * 2)) - RADIUS;
	            y = (random() % (RADIUS * 2)) - RADIUS;
	        if(zeroDist(x,y) < RADIUS){
	        inCircle++;
	                }
	    inSquare++;
            }

        /* Finish calculation */
        pCircle = inCircle/inSquare;
        piCalc = pCircle * 4.0;

        outputFile = fopen(local_outputFilename, "w");
        
        if(outputFile != NULL)
        {
			//fputs(piCalc, outputFile);
			fprintf(outputFile, "pi = %f\n", piCalc);
			fclose(outputFile);
		}
		
        /* Print result */
        fprintf(stdout, "pi = %f\n", piCalc);
        break;
        }
        
        else if(pid > 1)
        {
	        //fprintf(stdout, "I should be parent with pid: %d\n", pid);
            int status;
            waitpid(pid, &status, 0);
        }

}

    return 0;
}
