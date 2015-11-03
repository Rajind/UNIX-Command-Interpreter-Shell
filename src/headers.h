#include <stdio.h>
#include <stdlib.h>                                                               // just for malloc()
#include <unistd.h>                                                               // unix standard lib (fork, dup2 etc.)
#include <string.h>                                                               // string functions
#include <sys/types.h>                                                            // extra data types
#include <signal.h>                                                               // signals
#include <sys/wait.h>                                                             // wait functions
#include <fcntl.h>                                                                // file descriptor operations
#include <termios.h>                                                              // terminal control data structures
#define TRUE 1
#define FALSE !TRUE
typedef int bool;								  //define bool

static const char* SHELL_VERSION = "1.0";

/* definition of user input data structures */
#define BUFFER_MAX_LENGTH 256                                                   // maximum length of the buffer (256 chars)
static char* currentDirectory;                                                  // buffer to contain the current directory
static char userInput = '\0';							// count of commands
static char buffer[BUFFER_MAX_LENGTH];                                    	// buffer for user input line
static int bufferChars = 0;

static char *command[16];							// array of commands with arguments
static int commandC = 0;							// count of the commands 
static int commandConsecutive = 0;						// used to executive consecutive commands
static char *commandArgv[5];                                                    // array of strings that compose the command given
static int commandArgc = 0; 							// count of the words of the command given

bool logsOn = FALSE;								//to turn on and off logs

#define MAXLINE 4096

/* definition of job status */
#define FOREGROUND 'F'
#define BACKGROUND 'B'
#define SUSPENDED 'S'
#define WAITING_INPUT 'W'

/* standard file descriptors */
#define STDIN 1
#define STDOUT 2

/* definition of search type parameters of a job */
#define BY_PROCESS_ID 1
#define BY_JOB_ID 2
#define BY_JOB_STATUS 3

static int numActiveJobs = 0; 							// counts the number of active processes

typedef struct job { 								// struct that defines a job
        int id;
        char *name;
        pid_t pid;
        pid_t pgid; 								// not yet useful
        int status;
        char *descriptor;
        struct job *next;
} t_job;

static t_job* jobsList = NULL; 							// linked list of active processes


/* shell pid, pgid, default terminal, terminal modes */
static pid_t MYSH_PID;
static pid_t MYSH_PGID;
static int MYSH_TERMINAL, MYSH_IS_INTERACTIVE;
static struct termios MYSH_TMODES;

void logMessage(char *message);

/*********************************************************************
 * USER INPUT FUNCTIONS
 *********************************************************************/

/* gets a line of text from user */
void getTextLine();
/*parses set of commands */
void parseCommand();
/* populates commandArgv[] with every word that compose the command line */
void populateCommand();
void populateMultipleCommand(char *commandBuffer);
/* frees the buffer and commandArgv[] */
void destroyCommand();

/*********************************************************************
 * FUNCTIONS REGARDING ACTIVE JOBS
 *********************************************************************/

/*inserts an active process in the linked list*/
t_job * insertJob(pid_t pid, pid_t pgid, char* name, char* descriptor,
                  int status);
/* deletes a no more active process from the linked list */
t_job* delJob(t_job* job);
/* returns a Job object */
t_job* getJob(int searchValue, int searchParameter);
/* prints the active processes launched by the shell */
void printJobs();

/*********************************************************************
* PIPELINING FUNCTION
*********************************************************************/
void pipelining(int);

/*********************************************************************
 * PROMPT AND ERROR FUNCTIONS
 *********************************************************************/

/* displays a welcome screen */
void welcomeScreen();
/* displays a nice prompt for the shell */
void shellPrompt();

/*********************************************************************
 * SHELL FUNCTIONS
 *********************************************************************/

/* handles the command given: is it a built-in command or an external program? */
void handleUserCommand();
/* checks if a user command is a built-in command and executes it */
int checkBuiltInCommands();
/* executes a system command */
void executeCommand(char *command[], char *file, int newDescriptor,
                    int executionMode);
/* creates a child process */
void launchJob(char *command[], char *file, int newDescriptor,
               int executionMode);
/* puts a job in foreground, and continue it if it was suspended */
void putJobForeground(t_job* job, int continueJob);
/* puts a job in background, and continue it if it was suspended */
void putJobBackground(t_job* job, int continueJob);
/* wait for a specific job */
void waitJob(t_job* job);
/* kills a Job given its id */
void killJob(int jobId);
/* changes directory */
void changeDirectory();
/* initializes the shell */
void init();

/*********************************************************************
 * SIGNAL HANDLERS
 *********************************************************************/

/* signal handler for SIGCHLD */
void signalHandler_child(int p);
