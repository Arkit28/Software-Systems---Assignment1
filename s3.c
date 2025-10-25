#include "s3.h"

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
    else if(rc == 0)
    {
        // call child with output redirection
        child_with_output_redirection(args, argsc);
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
        if(line[i] == '>' || line[i] == '<'){
            return 1;//redirection detected
        }
    }
    return 0;//no redirection detected
}


/* TODO: make a function to determine the file which the output should end up in*/

// child's output goes to file:
void child_with_output_redirection(char *args[], int argsc)//e.g ls -l > out.txt
{
    int file = open(args[3], O_CREAT | O_WRONLY | O_TRUNC, 0644);
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
    execvp(args[0], args);
    perror("execvp failed");    // if execvp returns theres an error
    return;
    
}


/* TODO: implement function below AND make a function to determine where the input is coming from*/

void child_with_input_redirection(char* args[], int argsc)//e.g sort < data.txt
{
    int file = open(args[3], O_RDONLY);
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
    execvp(args[0], args);
    perror("execvp failed");    // if execvp returns theres an error
    return;
}