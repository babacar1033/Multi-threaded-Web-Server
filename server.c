#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <sys/time.h>
#include <time.h>
#include "util.h"
#include <stdbool.h>
#include <unistd.h>
#include <signal.h>

#define MAX_THREADS 100
#define MAX_queue_len 100
#define MAX_CE 100
#define INVALID -1
#define BUFF_SIZE 1024
#define ERROR -1

// Global variables because workers and dispatcher threads both need to use them
// Number of requests in the request queue
int totalitems = 0;
// Index of request queue for enqueuing 
int bufin = 0;
// Index of request queue for dequeuing
int bufout = 0;
// Used for request queue
typedef char* buffer_t;
// Size of request queue
int REQUEST_Q_SIZE = 0; 
// File pointer used for opening files
FILE *oStream;
// Used for the web tree root (testing folder)
const char *path;

// Signal handler for SIGINT --> so web server can gracefully terminate
static void forHandler(int val)
{
  // Closing the file stream for webserver_log
  fclose(oStream);
  // Print the number of pending requests in the request queue
  printf("\nThe number of pending requests in the request queue:%d\n", totalitems);
  // To close the program
  exit(val);
}

// structs:
typedef struct request_queue {
  int fd;
  char *request;
} request_t;

// We did not use this as we did not implement caching
typedef struct cache_entry {
  int len;
  char *request;
  char *content;
} cache_entry_t;

// Struct for conditional variables and locks
typedef struct condBuffer {
	request_t *queue;
	pthread_cond_t* sum_content;
  pthread_cond_t* free_slot;
	pthread_mutex_t* mtx;
} cond_buf;

// Struct used for arguments to worker() when worker threads are created
typedef struct args_struct {
  int threadID;
  cond_buf *condbuff;
} args_t;

/* ******************** Dynamic Pool Code  [Extra Credit A] **********************/
// Extra Credit: This function implements the policy to change the worker thread pool dynamically
// depending on the number of requests
/*
void * dynamic_pool_size_update(void *arg) {
}
 

}
*/
/**********************************************************************************/

/* ************************ Cache Code [Extra Credit B] **************************/
/*
// Function to check whether the given request is present in cache
int getCacheIndex(char *request){
  /// return the index if the request is present in the cache
}

// Function to add the request and its file content into the cache
void addIntoCache(char *mybuf, char *memory , int memory_size){memor
  // It should add the request at an index according to the cache replacement policy
  // Make sure to allocate/free memory when adding or replacing cache entries
}

// clear the memory allocated to the cache
void deleteCache(){
  // De-allocate/free the cache memory
}

// Function to initialize the cache
void initCache(){
  // Allocating memory and initializing the cache array
}
*/
/**********************************************************************************/

/* ************************************ Utilities ********************************/

//Within your code you should use one or two sentences to describe each function that you wrote.


// Function to get the content type from the request
// getContentType() takes in a filename (request) and determines what the contentType of a particular file is based upon the filename's file extension.  Then, getContentType() returns said contentType.
char* getContentType(char * mybuf) 
{
  // Should return the content type based on the file type in the request
  char *contentType;
  // Read the string and determine the file extension
  char period = '.';
  char *fileExtension;
  // Return pointer to last occurrence of '.' in filename so we can get file extension
  fileExtension = strchr(mybuf, period);
  if (fileExtension == NULL)
  {
  	printf("This request does not have a file extension.\n");
  }
  // Determine contentType parameter of return_result()
  if ((strcmp(fileExtension, ".html")== 0)||(strcmp(fileExtension, ".htm"))== 0)
  {
    contentType = "text/html";
  }
  else if((strcmp(fileExtension, ".jpg"))== 0)
  {
  	contentType = "image/jpeg";
  }
  else if((strcmp(fileExtension, ".gif"))== 0)
  {
  	contentType = "image/gif";
  }
  else
  {
  	contentType = "text/plain";
  }
  return contentType;
}

// Function to open and read the file from the disk into the memory
// Add necessary arguments as needed
// readFromDisk() opens the file with the filename the user specified.  Then, readFromDisk() reads the contents of said file to memory (heap) and returns a pointer to the address where the contents were stored.
char * readFromDisk(char *filename, int fd, int numBytes) 
{
  // Open and read the contents of file given the request
  // File pointer
  FILE *iStream;
  // Error text
  char* string = "Requested file not found.\n";
  // Used to store each character in the requested file
  char oneChar;
  // Points to the location in memory (heap) where the file contents are
  char *pointer;
  // Open the file for reading; the filename is from the request queue
  iStream = fopen(filename, "r");
  // Error handling for file stream
  if (iStream == NULL)
  {
    perror("This file could not be opened");
    numBytes = 0;
    // There is a problem accessing the file
    int error1 = return_error(fd, string);
    // Error handling for return_error()
    if (error1 != 0)
    {
    	printf("Error message could not be sent.\n");
    }
  }
  // Dynamically allocate space in memory only if the file has a nonnegative size
  if (numBytes > 0)
  {
    pointer = malloc(numBytes*sizeof(char));
  }
  // Error handling for dynamic allocation
  if (pointer == NULL) 
  { 
    printf("NumBytes: %d\n",numBytes);
    printf("Memory could not be allocated.\n");    
  }
	// Iterator variable for placing onto pointer
  int i = 0;
  // Non-null file pointer 
  if (iStream != 0x0)
  {
    oneChar = fgetc(iStream);
    // Error handling for fgetc
    while(oneChar != EOF)
    {
      if (numBytes > 0)
      {
        pointer[i] = oneChar;
        // Update the iterator
        i++;
        // Update the file pointer
        oneChar = fgetc(iStream);
      }
    }
    // Close the file stream
    fclose(iStream);
  }    
  // Will de-allocate the memory in the workers() function after the return_result() call 
  // Returns location in memory (heap) where the file contents are
  return pointer;
}

/**********************************************************************************/

// Function to receive the request from the client and add to the queue
// dispatch() accepts the requests and puts it into the request queue.
void * dispatch(void *arg) {
  while (1) {
    // Accept client connection
    int a = accept_connection();
    // Evaluate return value for accept_connection()
    if (a >= 0)
    {
		  // Get request from the client
		  // To store the filename
		  char *file = (char *)malloc(BUFF_SIZE*sizeof(char));
		  int error = get_request(a, file);
		  cond_buf * condBuf = (cond_buf *) arg;
		  // Initialize queue inside dispatcher
		  request_t *queue = condBuf -> queue; 
		  // Error handling for get_request()
		  if (error == 0)
		  {
		    // Error handling for lock() 
		    if ((pthread_mutex_lock(condBuf->mtx)) != 0)
		    {
		     printf("Could not acquire the lock.\n");
		    }
		    // Checking if the request queue is full
		    while (totalitems >= REQUEST_Q_SIZE)
		    {
		      // Error handling for conditional variable wait() 
		      if ((pthread_cond_wait(condBuf->sum_content,condBuf->mtx)) != 0)
		      {
		      	printf("Could not wait for the condition specified to be fulfilled.\n");
		      }
		    }
		    // Add the request into the queue - a bounded buffer
		    // Setting the file descriptor returned from accept_connection()
		    queue[bufin].fd = a;
		    // Setting filename (request)
		    queue[bufin].request = file;
		    // Moving up the index for the queue
		    bufin = (bufin + 1) % REQUEST_Q_SIZE;
		    // Incrementing number of requests in queue
		    totalitems++;
		    // Error handling for conditional variable signal() 
		    if ((pthread_cond_signal(condBuf->free_slot)) != 0)
		    {
		     printf("Could not send a signal to a worker thread.\n");
		    }
		    // Error handling for unlock() 
		    if ((pthread_mutex_unlock(condBuf->mtx)) != 0)
		    {
		     printf("Could not release the lock.\n");
		    }
		  } 
    }
    
  }
  return NULL;
}

/**********************************************************************************/

// Function to retrieve the request from the queue, process it and then return a result to the client
// worker() gets the request from the request queue, processes the request, and then prints information about the request to the terminal and the webserver_log file.
void * worker(void *arg) {
  // For return_result()
  char *contentType;
  // Number of requests completed by thread
  int reqNum = 0;
  // Getting args_t struct that is passed into worker
  args_t * arguments = (args_t *) arg;
  // Unique thread ID for thread
  int threadID = (arguments->threadID);
  // For stat() system call
  struct stat statBuf;
  // Number of bytes read
  off_t numBytes;
  
  while (1) {
    // Get the request from the queue
    // Boolean to keep track of whether worker has recieved a request or not
    bool has_request = false;
    // Local char * to hold request
    buffer_t itemp; 
    // Local request queue
    request_t *queue = arguments->condbuff->queue;
    bool read_from_disk = false;
    // Local int to hold file descriptor
    int file_descriptor;
    // Error handling for lock() 
		if ((pthread_mutex_lock(arguments->condbuff->mtx)) != 0)
		{
		 printf("Could not acquire the lock.\n");
		}
    // Lock worker if no requests in queue until signal is recieved from dispatcher
    while (totalitems <= 0)
    {
      // Error handling for conditional variable wait() 
		  if ((pthread_cond_wait(arguments->condbuff->free_slot, arguments->condbuff->mtx)) != 0)
		  {
		   printf("Could not wait for the condition specified to be fulfilled.\n");
		  }
    }
    // Getting filename (request)
    itemp = queue[bufout].request;
    // Getting file descriptor
    file_descriptor = queue[bufout].fd;
    has_request = true;
    // Moving up the index for the queue
    bufout = (bufout + 1) % REQUEST_Q_SIZE;
    // Updating totalitems in queue
    totalitems--;
    // Updating request number
    reqNum++; 
    // Error handling for conditional variable signal() 
		if ((pthread_cond_signal(arguments->condbuff->sum_content)) != 0)
		{
		 printf("Could not send a signal to a worker thread.\n");
		}
    // Error handling for unlock() 
		if ((pthread_mutex_unlock(arguments->condbuff->mtx)) != 0)
		{
		 printf("Could not release the lock.\n");
		}

    if (has_request) 
    {
      // Moving itemp up one to remove '/'
      itemp = itemp + 1;
      int stat_error = stat (itemp,&statBuf);
      // Error handling for stat() system call
      if (stat_error < 0)
      {
      	printf("Could not retrieve information about the file specified.\n");
      }
      numBytes = statBuf.st_size;
      // Reading from disk ang getting pointer to buffer
      char *pointer = readFromDisk(itemp, file_descriptor, numBytes);
      // Error handling for readFromDisk()
      if (pointer == NULL)
      {
      	printf("Unable to read from disk.\n");
      } 
      if (numBytes > 0) 
      {
        read_from_disk = true;
      }
      // Returning result
      contentType = getContentType(itemp);
      int return_num = return_result(file_descriptor, contentType, pointer, numBytes);
      // Error checking for return_result()
      if (return_num != 0)
      {
        printf("Failed to return request.\n");
      }
      // Get the data from the disk or the cache (extra credit B)
      
      // Log the request into the file and terminal
      int sprintf_check, fputs_check, fflush_check;
      // Error handling for lock() 
			if ((pthread_mutex_lock(arguments->condbuff->mtx)) != 0)
			{
			 printf("Could not acquire the lock.\n");
			}
      char stringFromInt[5];
      sprintf_check = sprintf(stringFromInt, "[%d]", threadID);
      // Error checking for sprintf()
      if (sprintf_check < 0) 
      {
      	printf("Could not send formatted output specified to the string.\n");
      }
      printf("%s",stringFromInt);
      fputs_check = fputs(stringFromInt,oStream); 
      // Error checking for fputs()
      if (fputs_check == EOF)
      {
      	printf("Could not write to file stream specified.\n");
      }
      // Getting total number of requests a specific worker thread has handled so far
      sprintf_check = sprintf(stringFromInt, "[%d]", reqNum);
      // Error checking for sprintf()
      if (sprintf_check < 0) 
      {
      	printf("Could not send formatted output specified to the string.\n");
      }
      printf("%s",stringFromInt);
      fputs_check = fputs(stringFromInt,oStream);
      // Error checking for fputs()
      if (fputs_check == EOF)
      {
      	printf("Could not write to file stream specified.\n");
      }
      // Change fd to string type --> fd is of type int which is max 4 bytes in C
      sprintf_check = sprintf(stringFromInt, "[%d]", file_descriptor);
      // Error checking for sprintf()
      if (sprintf_check < 0) 
      {
      	printf("Could not send formatted output specified to the string.\n");
      }
      printf("%s",stringFromInt);
      fputs_check = fputs(stringFromInt,oStream); 
      // Error checking for fputs()
      if (fputs_check == EOF)
      {
      	printf("Could not write to file stream specified.\n");
      }
      // Adding filename(itemp) to string filename_path for stdout
      char filename_path[BUFF_SIZE]; 
      sprintf_check = sprintf(filename_path, "[%s]", itemp);
      // Error checking for sprintf()
      if (sprintf_check < 0) 
      {
      	printf("Could not send formatted output specified to the string.\n");
      }
      printf("%s",filename_path);
      fputs_check = fputs(filename_path,oStream);
      // Error checking for fputs()
      if (fputs_check == EOF)
      {
      	printf("Could not write to file stream specified.\n");
      }
      // For bytes/error field of log request
      if (read_from_disk == false) 
      {
        char * request_error = "[Requested file not found.]\n";
        printf("%s",request_error);
        fputs_check = fputs(request_error,oStream); 
        // Error checking for fputs()
		    if (fputs_check == EOF)
		    {
		    	printf("Could not write to file stream specified.\n");
		    }
      } 
      else 
      {
        sprintf_check = sprintf(stringFromInt, "[%ld]\n", numBytes); 
        // Error checking for sprintf()
		    if (sprintf_check < 0) 
		    {
		    	printf("Could not send formatted output specified to the string.\n");
		    }
        printf("%s",stringFromInt);
        fputs_check = fputs(stringFromInt,oStream);
        // Error checking for fputs()
		    if (fputs_check == EOF)
		    {
		    	printf("Could not write to file stream specified.\n");
		    }
      }
      // Ensures that the log requests written to the log file do not lag behind those written to the terminal
      fflush_check = fflush(oStream);
      // Error handling for fflush()
      if (fflush_check == EOF)
      {
      	printf("Could not flush the output buffer of the stream specified.\n");
      }
      // Error handling for unlock() 
		  if ((pthread_mutex_unlock(arguments->condbuff->mtx)) != 0)
		  {
		   printf("Could not release the lock.\n");
		  }
		  if (numBytes > 0)
		  {
		  	free(pointer); 
		  }
      free(itemp-1);
   	}
   }
  return NULL;
}

/**********************************************************************************/
// Initializes threads, locks, conditional variables, and signal handler.  Also, turns command-line arguments into variables used in the rest of the program.
int main(int argc, char **argv) {

  // Initializing attributes for detachable threads
  pthread_attr_t attr;
  pthread_t dthreads[MAX_THREADS]; 
  pthread_t wthreads[MAX_THREADS];   
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

  // Set up signal handler and modify signal interrupt action 
  struct sigaction act;
  act.sa_handler = forHandler;
  act.sa_flags = 0;

	// Error handling for the signal handler
  if ((sigemptyset(&act.sa_mask) == -1) || (sigaction(SIGINT, &act, NULL) == -1)) 
  {
    perror("Failed to set SIGINT handler");
    return 1;
  }  

  // Error check on number of arguments
  if(argc != 8){
    printf("usage: %s port path num_dispatcher num_workers dynamic_flag queue_length cache_size\n", argv[0]);
    return -1;
  }

	// Set the request queue size from what the user specifies
  char* num_req = argv[6];
  int num_requests;
  // Convert command-line argument (a string) to an integer
  int sscanf_check;
  sscanf_check = sscanf(num_req,"%d",&num_requests);
  // Error handling for sscanf()
  if (sscanf_check == EOF)
  {
  	printf("Input failure before data could be read.\n");
  }
  // Error handling for queue_len command-line argument
  if (num_requests <= 0 || num_requests > 100)
  {
  	printf("The size of the request queue must be greater than 0 and less than or equal to 100.\n");
  	return ERROR;
  } 
  
  REQUEST_Q_SIZE = num_requests;
  // Request queue
  request_t queue[REQUEST_Q_SIZE];
 
  // Open web_server_log file in writing mode
  oStream = fopen("web_server_log", "w");
  // Error handling for fopen()
  if(oStream == NULL) 
  {
		perror("The web_server_log file could not be opened.");
  }
  
  path = argv[2];
  // Change directory to root of web tree
  int atRoot = chdir(path);
  // Error handling for chdir()
  if (atRoot < 0) 
  {
    printf("Could not change directory to root of web tree.");
  }

  // Get number of dispatchers
  char* num_string = argv[3];
  int num_dispatch;
  // Convert command-line argument (a string) to an integer
  
  sscanf_check = sscanf(num_string,"%d",&num_dispatch);
  // Error handling for sscanf()
  if (sscanf_check == EOF)
  {
  	printf("Input failure before data could be read.\n");
  }
  // Error handling for num_dispatch command-line argument
  if (num_dispatch <= 0 || num_dispatch > 100)
  {
  	printf("The number of dispatchers must be greater than 0 and less than or equal to 100.\n");
  	return ERROR;
  } 
  
  // Get number of workers
  num_string = argv[4];
  int num_workers;
  // Convert command-line argument (a string) to an integer
  sscanf_check = sscanf(num_string,"%d",&num_workers);
  // Error handling for sscanf()
  if (sscanf_check == EOF)
  {
  	printf("Input failure before data could be read.\n");
  }
  // Error handling for num_workers command-line argument
  if (num_workers <= 0 || num_workers > 100)
  {
  	printf("The number of workers must be greater than 0 and less than or equal to 100.\n");
  	return ERROR;
  } 

  // Initialize cache (extra credit B)

  // First change the command line argument to int to pass into "init()" function
  char* num_port = argv[1];
  int port;
  // Convert command-line argument (a string) to an integer
  sscanf_check = sscanf(num_port,"%d",&port);
  // Error handling for sscanf()
  if (sscanf_check == EOF)
  {
  	printf("Input failure before data could be read.\n");
  }
  // Error handling for incorrect port number
  if ((port < 1024) ||(port > 65536))
  {
  	printf("The port number specified is not supported by the web server.\n");
  }

  init(port);
  // Arguments to be passed into worker()
  args_t n_args[num_workers];

  // Initializing the conditional buffer
  cond_buf *q = (cond_buf*) malloc(sizeof(cond_buf));
  q->queue = queue;
  // Stop a request to be put into the request queue if the queue is full
  q->sum_content = (pthread_cond_t*) malloc(sizeof(pthread_cond_t));
  // Ensures that there is a request inside the request queue
  q->free_slot = (pthread_cond_t*) malloc(sizeof(pthread_cond_t));
  q->mtx = (pthread_mutex_t*) malloc(sizeof(pthread_mutex_t));
  pthread_cond_init(q->sum_content, NULL);
  pthread_cond_init(q->free_slot, NULL);
  pthread_mutex_init(q->mtx, NULL);

  // Creating dispatcher threads
  for (int i = 0; i < num_dispatch; i++)
  {
  	// Error handling for thread creation
    if ((pthread_create((&dthreads[i]), &attr, dispatch, (void *)q)) != 0)
    {
    	printf("The dispatcher thread could not be created.\n");
    }
  }
  // Creating worker threads
  for (int i = 0; i < num_workers; i++)
  {
    n_args[i].threadID = i;
    n_args[i].condbuff = q;
    // Error handling for thread creation
    if ((pthread_create((&wthreads[i]), &attr, worker, (void *)&n_args[i])) != 0)
    {
    	printf("The worker thread could not be created.\n");
    }
  }
  

  // Create dynamic pool manager thread (extra credit A)
  
  // Terminate server gracefully
  // Print the number of pending requests in the request queue
  
  // close log file
  // Remove cache (extra credit B)
    

  while(1){}
    return 0;

}
