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
    execvp(args[0], args);
    perror("execvp failed");//outputs process failed
    exit(EXIT_FAILURE);
}

void launch_program(char *args[], int argsc)
{
    /* no input, do nothing */
    if (argsc == 0) {
        return;
    }

    /* If the user typed 'exit', exit the shell (current process). */
    if (strcmp(args[0], "exit") == 0) {
        exit(0);
    }

    // forking child process
    pid_t rc = fork();
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
