/*
 * File: rw.c
 * Author: Andy Sayler
 * Project: CSCI 3753 Programming Assignment 3
 * Create Date: 2012/03/19
 * Modify Date: 2012/03/20
 * Description: A small i/o bound program to copy N bytes from an input
 *              file to an output file. May read the input file multiple
 *              times if N is larger than the size of the input file.
 */

/* Include Flags */
#define _GNU_SOURCE

/* System Includes */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sched.h>

/* Local Defines */
#define MAXFILENAMELENGTH 80
#define DEFAULT_INPUTFILENAME "rwinput"
#define DEFAULT_OUTPUTFILENAMEBASE "rwoutput"
#define DEFAULT_BLOCKSIZE 1024
#define DEFAULT_TRANSFERSIZE 1024*100

int main(int argc, char* argv[]){

    int rv;
    int inputFD;
    int outputFD;
    char inputFilename[MAXFILENAMELENGTH];
    char outputFilename[MAXFILENAMELENGTH];
    char outputFilenameBase[MAXFILENAMELENGTH];

    //Adding Scheduler Variables:
    struct sched_param param;
    int policy = SCHED_OTHER;

    //Adding Code for Fork:
    pid_t pid;
    pid_t parent = getpid();
    int processes;
    int p;

    ssize_t transfersize = 0;
    ssize_t blocksize = 0; 
    char* transferBuffer = NULL;
    ssize_t buffersize;

    ssize_t bytesRead = 0;
    ssize_t totalBytesRead = 0;
    int totalReads = 0;
    ssize_t bytesWritten = 0;
    ssize_t totalBytesWritten = 0;
    int totalWrites = 0;
    int inputFileResets = 0;
    
    /* Process program arguments to select run-time parameters */
    /* Set supplied transfer size or default if not supplied */
    if(argc < 2){
	transfersize = DEFAULT_TRANSFERSIZE;
    }
    else{
	transfersize = atol(argv[1]);
	if(transfersize < 1){
	    fprintf(stderr, "Bad transfersize value\n");
	    exit(EXIT_FAILURE);
	}
    }
    /* Set supplied block size or default if not supplied */
    if(argc < 3){
	blocksize = DEFAULT_BLOCKSIZE;
    }
    else{
	blocksize = atol(argv[2]);
	if(blocksize < 1){
	    fprintf(stderr, "Bad blocksize value\n");
	    exit(EXIT_FAILURE);
	}
    }
    /* Set supplied input filename or default if not supplied */
    if(argc < 4){
	if(strnlen(DEFAULT_INPUTFILENAME, MAXFILENAMELENGTH) >= MAXFILENAMELENGTH){
	    fprintf(stderr, "Default input filename too long\n");
	    exit(EXIT_FAILURE);
	}
	strncpy(inputFilename, DEFAULT_INPUTFILENAME, MAXFILENAMELENGTH);
    }
    else{
	if(strnlen(argv[3], MAXFILENAMELENGTH) >= MAXFILENAMELENGTH){
	    fprintf(stderr, "Input filename too long\n");
	    exit(EXIT_FAILURE);
	}
	strncpy(inputFilename, argv[3], MAXFILENAMELENGTH);
    }
    /* Set supplied output filename base or default if not supplied */
    if(argc < 5){
	if(strnlen(DEFAULT_OUTPUTFILENAMEBASE, MAXFILENAMELENGTH) >= MAXFILENAMELENGTH){
	    fprintf(stderr, "Default output filename base too long\n");
	    exit(EXIT_FAILURE);
	}
	strncpy(outputFilenameBase, DEFAULT_OUTPUTFILENAMEBASE, MAXFILENAMELENGTH);
    }
    else{
	if(strnlen(argv[4], MAXFILENAMELENGTH) >= MAXFILENAMELENGTH){
	    fprintf(stderr, "Output filename base is too long\n");
	    exit(EXIT_FAILURE);
	}
	strncpy(outputFilenameBase, argv[4], MAXFILENAMELENGTH);
    }


    //Default SCHED_OTHER
    if(argv[5])
    {
		if(!strcmp(argv[5], "SCHED_FIFO"))
		{
	        policy = SCHED_FIFO;
	    }
		else if(!strcmp(argv[5], "SCHED_RR"))
		{
	        policy = SCHED_RR;
	    }
	    else printf("SCHED_OTHER by Default / Selection\n");
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

    /* Confirm blocksize is multiple of and less than transfersize*/
    if(blocksize > transfersize){
	fprintf(stderr, "blocksize can not exceed transfersize\n");
	exit(EXIT_FAILURE);
    }
    if(transfersize % blocksize){
	fprintf(stderr, "blocksize must be multiple of transfersize\n");
	exit(EXIT_FAILURE);
    }

    //Enable Forking:
    if(argv[6]) processes = atoi(argv[6]);
    else processes = 1;
    
    for(p = 0; p < processes; p++) 
    {
		
		pid = fork();

        if (pid == -1) return 1;

        else if(pid == 0)
        {
			//printf("I'm a child!\n");
			
			char processNumber[MAXFILENAMELENGTH];
			sprintf(processNumber, "%d", p);
			
			//printf("processNumber is: %s\n", processNumber);
			
			char local_inputFilename[MAXFILENAMELENGTH];
            char local_outputFilename[MAXFILENAMELENGTH];
            char local_outputFilenameBase[MAXFILENAMELENGTH];
			
			char blah[MAXFILENAMELENGTH];
			char blah1[MAXFILENAMELENGTH];
			//strncpy(blah, "blah.txt", MAXFILENAMELENGTH);
			//strncpy(blah1, "blah1.txt", MAXFILENAMELENGTH);
			
			//fprintf(stdout, "the blahs %s %s\n", blah, blah1);
			
			strncpy(local_inputFilename, inputFilename, MAXFILENAMELENGTH);
			strncpy(local_outputFilename, outputFilename, MAXFILENAMELENGTH);
			strncpy(local_outputFilenameBase, outputFilenameBase, MAXFILENAMELENGTH);
			strncat(local_inputFilename, processNumber, MAXFILENAMELENGTH);
			strncat(local_outputFilename, processNumber, MAXFILENAMELENGTH);
			strncat(local_outputFilenameBase, processNumber, MAXFILENAMELENGTH);
			
			//local_inputFilename = inputFilename;
			//local_outputFilename = outputFilename + processNumber;
			//local_outputFilenameBase = outputFilenameBase + processNumber;
			
			fprintf(stdout, "inputFilename: %s and local_inputFilename: %s\n", inputFilename, local_inputFilename);
			
            /* Allocate buffer space */
            buffersize = blocksize;
            if(!(transferBuffer = malloc(buffersize*sizeof(*transferBuffer)))){
	            perror("Failed to allocate transfer buffer");
	            exit(EXIT_FAILURE);
            }
            
            //char lsString[MAXFILENAMELENGTH];
            //strncpy(lsString, "ls ", MAXFILENAMELENGTH);
            //strncat(lsString, local_inputFilename, MAXFILENAMELENGTH);
            //strncat(lsString, " > output.log", MAXFILENAMELENGTH);
            //int systemLS = system(lsString);
            
            //fprintf(stderr, "systemLS: %d", systemLS);
            
            //Creates File if it Does Not Exist:
            //**NOTE** DOES NOT SCRUB FOR LEGIMATE DATA:
            char execinputString[MAXFILENAMELENGTH];
            char execoutputString[MAXFILENAMELENGTH];
            char execoutputbaseString[MAXFILENAMELENGTH];
            
            //Convert to a Function (running out of time, haha) do later:
            strncpy(execinputString, "/bin/ls ", MAXFILENAMELENGTH);
            strncat(execinputString, local_inputFilename, MAXFILENAMELENGTH);
            strncat(execinputString, " || cp ", MAXFILENAMELENGTH);
            strncat(execinputString, inputFilename, MAXFILENAMELENGTH);
            strncat(execinputString, " ", MAXFILENAMELENGTH);
            strncat(execinputString, local_inputFilename, MAXFILENAMELENGTH);
            
            strncpy(execoutputString, "/bin/ls ", MAXFILENAMELENGTH);
            strncat(execoutputString, local_outputFilename, MAXFILENAMELENGTH);
            strncat(execoutputString, " || cp ", MAXFILENAMELENGTH);
            strncat(execoutputString, outputFilename, MAXFILENAMELENGTH);
            strncat(execoutputString, " ", MAXFILENAMELENGTH);
            strncat(execoutputString, local_outputFilename, MAXFILENAMELENGTH);
            
            strncpy(execoutputbaseString, "/bin/ls ", MAXFILENAMELENGTH);
            strncat(execoutputbaseString, local_outputFilenameBase, MAXFILENAMELENGTH);
            strncat(execoutputbaseString, " || cp ", MAXFILENAMELENGTH);
            strncat(execoutputbaseString, outputFilenameBase, MAXFILENAMELENGTH);
            strncat(execoutputbaseString, " ", MAXFILENAMELENGTH);
            strncat(execoutputbaseString, local_outputFilenameBase, MAXFILENAMELENGTH);
            
            int system_find_input = system(execinputString);
            //int system_find_output = system(execoutputString);
            
            //Makes Copy:
            //int systemCopy = execlp("/bin/cp", "/bin/cp", inputFilename, local_inputFilename, NULL);
            //int systemCopy = execlp("/usr/bin/find", "/usr/bin/find", "-name", local_inputFilename, "|| cp", inputFilename, local_inputFilename, NULL);
         
         
            //fprintf(stdout, "systemCopy = %d\n", systemCopy);
            
            /* Open Input File Descriptor in Read Only mode */
            if((inputFD = open(local_inputFilename, O_RDONLY | O_SYNC)) < 0){
	            perror("Failed to open input file");
	        exit(EXIT_FAILURE);
	           //char inputPath[100] = "files/inputFile/";
	           //strncat(inputPath, local_inputFilename, 100);
	            //printf("inputPath: %s\n", inputPath);
	            
	            //execlp("cp", inputFilename, local_inputFilename);
            }
            
            else
            {
				
			}

            /* Open Output File Descriptor in Write Only mode with standard permissions*/
            rv = snprintf(local_outputFilename, MAXFILENAMELENGTH, "%s-%d",
	    	      local_outputFilenameBase, getpid());    
            if(rv > MAXFILENAMELENGTH){
    	    fprintf(stderr, "Output filenmae length exceeds limit of %d characters.\n",
	    	    MAXFILENAMELENGTH);
	        exit(EXIT_FAILURE);
            }
            else if(rv < 0){
	        perror("Failed to generate output filename");
	        exit(EXIT_FAILURE);
            }
            if((outputFD =
	        open(local_outputFilename,
	             O_WRONLY | O_CREAT | O_TRUNC | O_SYNC,
	             S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH)) < 0){
    	    perror("Failed to open output file");
	        exit(EXIT_FAILURE);
            }

            /* Print Status */
            fprintf(stdout, "Reading from %s and writing to %s\n",
	            local_inputFilename, local_outputFilename);

            /* Read from input file and write to output file*/
            do{
        	/* Read transfersize bytes from input file*/
        	bytesRead = read(inputFD, transferBuffer, buffersize);
	        if(bytesRead < 0){
	            perror("Error reading input file");
	            exit(EXIT_FAILURE);
	        }
	        else{
	            totalBytesRead += bytesRead;
	            totalReads++;
	        }
	
	        /* If all bytes were read, write to output file*/
        	if(bytesRead == blocksize){
	            bytesWritten = write(outputFD, transferBuffer, bytesRead);
	            if(bytesWritten < 0){
		        perror("Error writing output file");
	    	    exit(EXIT_FAILURE);
	            }
	            else{
	    	    totalBytesWritten += bytesWritten;
	    	    totalWrites++;
	            }
	        }
	        /* Otherwise assume we have reached the end of the input file and reset */
	        else{
	            if(lseek(inputFD, 0, SEEK_SET)){
	    	    perror("Error resetting to beginning of file");
	    	    exit(EXIT_FAILURE);
	            }
	            inputFileResets++;
        	}
	
            }while(totalBytesWritten < transfersize);

            /* Output some possibly helpfull info to make it seem like we were doing stuff */
            fprintf(stdout, "Read:    %zd bytes in %d reads\n",
	            totalBytesRead, totalReads);
            fprintf(stdout, "Written: %zd bytes in %d writes\n",
	            totalBytesWritten, totalWrites);
            fprintf(stdout, "Read input file in %d pass%s\n",
	            (inputFileResets + 1), (inputFileResets ? "es" : ""));
            fprintf(stdout, "Processed %zd bytes in blocks of %zd bytes\n",
	            transfersize, blocksize);
	
            /* Free Buffer */
            free(transferBuffer);

            /* Close Output File Descriptor */
            if(close(outputFD)){
	        perror("Failed to close output file");
	        exit(EXIT_FAILURE);
            }

            /* Close Input File Descriptor */
            if(close(inputFD)){
	        perror("Failed to close input file");
	        exit(EXIT_FAILURE);
            }
    
        break;
    }
    
    else if(pid > 0)
    {
		int status;
		 waitpid(pid, &status, 0);
	}
    
    }

    return EXIT_SUCCESS;
}
