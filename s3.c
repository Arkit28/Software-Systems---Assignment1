#include "s3.h"
#include <string.h>
#include <sys/wait.h>
#define MAX_ARGS 128



///Simple for now, but will be expanded in a following section
void construct_shell_prompt(char shell_prompt[])
{
    strcpy(shell_prompt, "[s3]$ ");
}

///Prints a shell prompt and reads input from the user
void read_command_line(char line[])
{
    char shell_prompt[MAX_PROMPT_LEN];
    construct_shell_prompt(shell_prompt);
    printf("%s", shell_prompt);

    ///See man page of fgets(...)
    if (fgets(line, MAX_LINE, stdin) == NULL)
    {
        perror("fgets failed");
        exit(1);
    }
    ///Remove newline (enter)
    line[strlen(line) - 1] = '\0';
}

void parse_command(char line[], char *args[], int *argsc)
{
    ///Implements simple tokenization (space delimited)
    ///Note: strtok puts '\0' (null) characters within the existing storage, 
    ///to split it into logical cstrings.
    ///There is no dynamic allocation.

    ///See the man page of strtok(...)
    char *token = strtok(line, " ");
    *argsc = 0;
    while (token != NULL && *argsc < MAX_ARGS - 1)
    {
        args[(*argsc)++] = token;
        token = strtok(NULL, " ");
    }
    
    args[*argsc] = NULL; ///args must be null terminated
}

///Launch related functions
void child(char *args[], int argsc)
{
    /* Replace the current process image with the program specified in args.
       If execvp returns, an error occurred. */

    if(args[0] == "exit"){
        exit(0);
    }

    execvp(args[0], args);//takes a vector input, uses args[0] to find path to direc
    
    perror("execvp failed");
    exit(EXIT_FAILURE);
}

void launch_program(char *args[], int argsc)
{

    if(args[0] != NULL && strcmp(args[0], "exit") == 0){
        printf("Exiting Shell...\n");
        exit(0);
    }
    int rc = fork();

    if (rc < 0) {
        perror("fork failed");
        return;
    } else if (rc == 0) {
        /* Child: execute program */
        child(args, argsc);
        /* If child returns, exit with failure */
        exit(EXIT_FAILURE);
    } else {
        /* Parent: wait for child to finish */
        int status;
        if (waitpid(rc, &status, 0) == -1) {
            perror("waitpid failed");
        }
    }
}

//task2//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* TODO: Make function below accept < and >> operators too */

void launch_program_with_redirection(char* args[], int argsc)
{
    int rc = fork();

    if(rc < 0)
    {
        perror("fork failed, terminating process\n");
        return;
    }
    else if(rc == 0)//i am the childe process
    {
        // call the specifc child function for redirection
        switch (redirection_type){
            case 1://>
                child_with_output_redirection_overwrite(args, argsc);
                break;
            case 2://>>
                child_with_output_redirection_appending(args, argsc);
                break;
            case 3://<
                child_with_input_redirection(args, argsc);
                break;
            default:
                perror("invalid redirection_type");
                exit(EXIT_FAILURE);
        }
        // if child returns theres an error
        perror("Child flopped");
        exit(EXIT_FAILURE);
    }
    else
    {
        //parent waiting for child to complete
        int status;
        if (waitpid(rc, &status, 0) == -1) {
            perror("waitpid failed");
        }
    }

}


int command_with_redirection(char line[]){//checks if a redirection exists detecting < or >
    //scan the input string for < or >
    for(int i = 0; line[i] != '\0'; i++){
        if(line[i] == '>'){
            redirection_type=1;
            return 1;//child_with_output_redirection needs calling
        }else if ( line[i] == '>' && line[i+1] == '>'){
            redirection_type=2;
            return 2;//child_with_input_redirection_appending needs calling
        }else if ( line[i] == '<'){
            redirection_type=3;
            return 3;//child_with_input_redirection needs calling
        }
    }
    return 0;//no redirection detected
}


/* TODO: make a function to determine the file which the output should end up in*/

// child's output goes to file:
// handles "command > file"
void child_with_output_redirection_overwrite(char *args[], int argsc)//e.g ls -l > out.txt
{
    int file = open(redirection_file(args,argsc), O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if(file == -1)
    {
        perror("open failed");
        return;
    }

    if(dup2(file, STDOUT_FILENO) == -1)
    {
        perror("dup2 failed");
        close(file);
        return;
    }
    close(file);

    //if all checks are fine, execute command
    execvp(args[0], redir_exec_args(args, argsc));
    perror("execvp failed");// if execvp returns theres an error
    return;
    
}


/* TODO: implement function below AND make a function to determine where the input is coming from*/


// handles "command < file"
void child_with_input_redirection(char* args[], int argsc)//e.g sort < data.txt
{
    int file = open(redirection_file(args,argsc), O_RDONLY);
    if(file == -1)
    {
        perror("open failed");
        return;
    }

    if(dup2(file, STDIN_FILENO) == -1)
    {
        perror("dup2 failed");
        close(file);
        return;
    }
    close(file);

        //if all checks are fine, execute command
    execvp(args[0], redir_exec_args(args, argsc));
    perror("execvp failed");// if execvp returns theres an error
    return;
}


// handles "command >> file"
void child_with_output_redirection_appending(char *args[], int argsc)//echo "second" >> file.txt
{//same as child_with_output_redirection bu flags changed
    int file = open(redirection_file(args,argsc), O_CREAT | O_WRONLY | O_APPEND, 0644);
    if (file == -1) {
        perror("open failed");
        return;
    }

    if (dup2(file, STDOUT_FILENO) == -1) {
        perror("dup2 failed");
        close(file);
        return;
    }
    close(file);

    execvp(args[0], redir_exec_args(args, argsc));
    perror("execvp failed");
    return;
}

//making a function to get the arguments for the child redirections for the open function
char *redirection_file(char *args[], int argsc)
{
    for (int i = 0; i < argsc - 1; i++) {
        if (strcmp(args[i], "<") == 0 || strcmp(args[i], ">") == 0 || strcmp(args[i], ">>") == 0) {
            return args[i + 1];  // returning the filename after the symbol
        }
    }
    return NULL; // no redirection symbol found
}

//making a function to get the arguments for the child redirections for the execvp function
char **redir_exec_args(char *args[], int argsc)
{
    static char *exec_args[MAX_ARGS];
    int count = 0;

    //counting tokens before redirection
    for(int i=0; i<argsc; i++)
    {
        if(strcmp(args[i], "<") == 0 || strcmp(args[i], ">") == 0 || strcmp(args[i], ">>")==0)
        {
            break;//exit the for loop before redirection
        }else
        {
            exec_args[count]=args[i];
            count++;
        }
    }
    //adding last part of the array, the NULL
    exec_args[count]=NULL;
    return exec_args;
}










