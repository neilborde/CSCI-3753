/*
Warren Payne
wapa1274@colorado.edu
*/

//Includes
    #include <pthread.h>
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <sys/types.h>
    #include <sys/syscall.h>
    #include "util.h"
    #include "multi-lookup.h"
    #include "sys/time.h"
//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
//Structs
    struct thread_data{
        char *ptr_to_fileArray;
    };
    struct mutexes {
        pthread_mutex_t lock[10];
        pthread_mutex_t sharedBufferLock;
        pthread_mutex_t serviceLock;
        pthread_mutex_t resultsLock;  
    };
    struct various_variables {
        char sharedBuffer[2000][20];
        char files_to_service[20][20];
        char results[20];
        char serviced[20];
        int GLOBALFILECOUNT; //set in main
        int bufferIndex;
    };
    struct globals {
        struct thread_data *array;
        struct various_variables var;
        struct mutexes mut;
    };

/* ______________________________________________________________________________________

    functions

    ____________________________________________________________________________________ */

void *requester(void *thd){
	// process id
	pid_t tid = syscall(SYS_gettid);
	struct globals *req = thd;

	for(int i = 0; i < req->var.GLOBALFILECOUNT; i++) {
		// if thread currently doesn't have lock, it now will
		if(!pthread_mutex_trylock(&req->mut.lock[i])){
			char file_tobe_serviced[20];
			// had to get help for this, still don't quite get it
			strcpy(file_tobe_serviced, (req->array->ptr_to_fileArray + (i*20)));
			// fetching the thread id
			pid_t tid = syscall(SYS_gettid);
			printf("\n%d is servicing %s\n", tid, file_tobe_serviced);
			pthread_mutex_lock(&req->mut.serviceLock); //lock the service mutex before servicing
			// appending to serviced struct
				FILE *f = fopen(req->var.serviced, "a"); // serviced texts

				if(f == NULL) {
					printf("could not open file to be serviced\n");
					exit(1);
				}
				//fprintf to print to file rather than stdout
				fprintf(f, "thread: %d is servicing %s\n", tid, file_tobe_serviced);
				fclose(f);
			pthread_mutex_unlock(&req->mut.serviceLock); //unlock service lock
			FILE* inputFile = fopen(file_tobe_serviced, "r"); //just reading
			if(!inputFile) {
				// error
				printf("Error: input file\n");
			}
			char domainName[20];
			// fgets(dest, n-1 characters to be read, src stream)
			while(fgets(domainName, sizeof(domainName)+1, inputFile)){
				// fetch domains and put them in buffer
				domainName[strlen(domainName)-1] = '\0';
				strcpy(req->var.sharedBuffer[req->var.bufferIndex], domainName);
				req->var.bufferIndex++; //next line in buffer
			}
			pthread_mutex_destroy(&req->mut.lock[i]);
		}


	}
	return 0;
}


void *resolver(void *thd){
	struct globals *req = thd;
	if(!pthread_mutex_trylock(&req->mut.sharedBufferLock)) {
		// if true, currently unlocked, gtg
		for(int i = 0; i < req->var.bufferIndex; i++){ //each line in the buffer
			char domain[20];
			char ip[20];
			// copying contents of buffer into domainName array
			strcpy(domain, req->var.sharedBuffer[i]);
			printf("%s, ", domain); //print each domain
			int flag = 0; // to check for validity of host names
			if(dnslookup(domain, ip, sizeof(ip))){
				fprintf(stderr, "invalid hostname: %s \n", domain);
				flag = 1;		// must have invalid hostname
				printf("\n");
			}
			else{
				printf("%s\n", ip); // give ip
			}

			pthread_mutex_lock(&req->mut.resultsLock); //locking results lock
			// appending newly found ip
				FILE *f = fopen(req->var.results, "a");
				if(f == NULL) {
					printf("error: output file issues yo \n");
					exit(1);
				}
				if(!flag){
					//if not valid hostname
					fprintf(f, "%s, %s\n", domain, ip);
				}
				else {
					fprintf(f, "%s, \n", domain);

				}
				fclose(f);
			pthread_mutex_unlock(&req->mut.resultsLock);
		}
		// so we need to destroy the lock?
	pthread_mutex_destroy(&req->mut.sharedBufferLock);
	}	

}


int main(int argc, char *argv[]) {
	// begin time
	clock_t begin, finish;
	begin = clock();
	// printf("num arguments %d", argc);
	// to call the global struct (good way to not write real globals)
	struct globals gl;
	// nothing in buffer yet
	gl.var.bufferIndex = 0;

	// going to write into servided / results.txt
	strcpy(gl.var.serviced, argv[4]);
	strcpy(gl.var.results, argv[3]);


	// for requester threads
	// strtol is a way to grab the number from a string, where 10 is the base
	// pulling 
	int num_req_threads = strtol(argv[1], NULL, 10);
	// array of request threads
	pthread_t reqThreads[num_req_threads];
	struct thread_data thread_data_array[num_req_threads];
	// setting shared array
	gl.array = thread_data_array;

	// for resolver threads
	int num_res_threads = strtol(argv[2], NULL, 10);
	// array of resolver threads
	pthread_t resThreads[num_res_threads];
	char f[20][20];
	// number of input files
	int fileCount = argc-5;
	gl.var.GLOBALFILECOUNT = fileCount;

	for(int i = 0; i < fileCount; i++){
		// copying the contents of each input file into f
		strcpy(f[i], argv[i+5]);
	}
	char inputFiles[20][20];

	// initialize locks to default value
	// NULL argument sets locks to default values
	pthread_mutex_init(&gl.mut.resultsLock, NULL);
	pthread_mutex_init(&gl.mut.serviceLock, NULL);
	pthread_mutex_init(&gl.mut.sharedBufferLock, NULL);
	// initialize all locks
	for(int i = 0; i < 10; i++){
		pthread_mutex_init(&gl.mut.lock[i], NULL);
	}

	// populating thread data and input files to be serviced
	for(int i = 0; i < num_req_threads; i++){
		thread_data_array[i].ptr_to_fileArray = &f;
	}

	// create requester threads
	for(int i =0; i < num_req_threads; i++){
		pthread_create(&reqThreads[i], NULL, requester, &gl);
		// creating a thread (thread, attr, start routine, arg)
	}

	for(int i = 0; i < num_req_threads; i++){
		pthread_join(reqThreads[i], NULL);
		// calling threads waits for the ending of the target thread
	}

	// create resolver threads
	for(int i = 0; i < num_res_threads; i++){
		pthread_create(&resThreads[i], NULL, resolver, &gl);
	}
	// join resolver threads

	for (int i = 0; i < num_res_threads; i++){
		pthread_join(resThreads[i], NULL);

	}

	// end time
	finish = clock();
	printf("Time elapsed (in ticks) = %d \n", finish-begin);
}


// strncpy would be optimal.... will see in interviews