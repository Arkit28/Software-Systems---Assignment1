#ifndef _S3_H_
#define _S3_H_

///See reference for what these libraries provide
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

///Constants for array sizes, defined for clarity and code readability
#define MAX_LINE 1024
#define MAX_ARGS 128
#define MAX_PROMPT_LEN 256

//global variables

extern int redirection_type;  // 0 = none, 1 = >, 2 = >>, 3 = < stores the type of redirection once command_with_redirection runs


///Enum for readable argument indices (use where required)
enum ArgIndex
{
    ARG_PROGNAME,
    ARG_1,
    ARG_2,
    ARG_3,
};

///With inline functions, the compiler replaces the function call 
///with the actual function code;
///inline improves speed and readability; meant for short functions (a few lines).
///the static here avoids linker errors from multiple definitions (needed with inline).
static inline void reap()
{
    wait(NULL);
}

///Shell I/O and related functions (add more as appropriate)
void read_command_line(char line[]);
void construct_shell_prompt(char shell_prompt[]);
void parse_command(char line[], char *args[], int *argsc);

///Child functions (add more as appropriate)
void child(char *args[], int argsc);


///Program launching functions (add more as appropriate)
void launch_program(char *args[], int argsc);

int command_with_redirection(char line[]);//to check if command contains redirection

void launch_program_with_redirection(char *args[], int argsc)// Executes a command with input/output redirection by using dup2() to map STDIN/STDOUT to the target file before execvp(), then waits for the child to finish.

//redirection different types
void child_with_output_redirection_overwrite(char *args[], int argsc)
void child_with_input_redirection(char* args[], int argsc)
void child_with_output_redirection_appending(char *args[], int argsc)

//redirectionhelper functions
char *redirection_file(char *args[], int argsc)//getting the file name to create and dup
char **redir_exec_args(char *args[], int argsc)//pointer to array of pointerts to strings of the command

#endif