#ifndef _S3_H_
#define _S3_H_

///See reference for what these libraries provide
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
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

extern char cwd[MAX_PROMPT_LEN];
extern char lwd[MAX_PROMPT_LEN];

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

static inline int choose_flags(int redirection_type){
    return (redirection_type == 1) ?
        O_WRONLY | O_CREAT | O_TRUNC : 
        O_WRONLY | O_CREAT | O_APPEND;
}

///Shell I/O and related functions (add more as appropriate)
void read_command_line(char line[], char lwd[]);
void construct_shell_prompt(char shell_prompt[], char lwd[]);
void parse_command(char line[], char *args[], int *argsc);

///Child functions (add more as appropriate)
void child(char *args[], int argsc);
void child_with_output_redirection(char *args[], int argsc);        //redirection output
void child_with_input_redirection(char* args[], int argsc);         //redirection input

///Program launching functions (add more as appropriate)
void launch_program(char *args[], int argsc);
void launch_program_with_redirection(char *args[], int argsc);  // Executes a command with input/output redirection by using dup2() to map STDIN/STDOUT to the target file before execvp(), then waits for the child to finish.
void launch_program_with_pipes(char *args[], int argsc);        //executes commands with pipes


//redirection helper functions
int command_with_redirection(char line[]);        //to check if command contains redirection
char *redirection_file(char *args[], int argsc);    //getting the file name to create and dup
char **redir_exec_args(char *args[], int argsc);    //pointer to array of pointerts to strings of the command


//directory functions:
int is_cd(char args[]);                             
void run_cd(char* args[], int argsc, char lwd[]);
void init_lwd(char lwd[]);                          //set lwd = current working directory on shell start

//EXTENSION: directory stack using a 2d array, with basic functions
extern char directory_stack[128][MAX_LINE];
extern int top;
void pushd(const char* dir);
void popd();
void dirs();


// pipeline functions:
int command_with_pipes(char line[]);
int tokenise_pipe_commands(char* args[], int argsc, char* cmds_piped[MAX_ARGS][MAX_ARGS]);
char* strip_quotes(char* str);

// batched command functions:
int has_batched_commands(char line[]);
void tokenise_batched_commands(char line[], char* commands[], int* command_count);


//debug functions:
void print_tokens(char* args[], int argsc);
void print_piped_tokens(char* args[], int argsc);
#endif