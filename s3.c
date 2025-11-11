#include "s3.h"
#define MAX_ARGS 128

char cwd[MAX_PROMPT_LEN];
char lwd[MAX_PROMPT_LEN];
int redirection_type;


///Simple for now, but will be expanded in a following section
void construct_shell_prompt(char shell_prompt[], char lwd[])
{
    if (getcwd(cwd, MAX_ARGS) == NULL)
    {
        perror("getcwd failed");
        cwd[0] = '\0';
    }

    snprintf(shell_prompt, MAX_PROMPT_LEN, "[%.248s]$ ", cwd);
}

///Prints a shell prompt and reads input from the user
void read_command_line(char line[], char lwd[])
{
    char shell_prompt[MAX_PROMPT_LEN];
    construct_shell_prompt(shell_prompt, lwd);
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
        args[(*argsc)++] = strip_quotes(token);
        token = strtok(NULL, " ");
    }
    
    args[*argsc] = NULL; ///args must be null terminated
}

char* strip_quotes(char* s) {
    int len = strlen(s);
    if (len >= 2) {
        if ((s[0] == '"' && s[len-1] == '"') || (s[0] == '\'' && s[len-1] == '\'')) {
            s[len-1] = '\0'; // remove last quote
            return s + 1;     // skip first quote
        }
    }
    return s;
}


// debug function
void print_tokens(char *args[], int argsc)
{
    printf("Parsed %d tokens:\n", argsc);
    for (int i = 0; i < argsc; i++)
    {
        printf("  Token[%d]: '%s'\n", i, args[i]);
    }
}


///Launch related functions
void child(char *args[], int argsc)
{
    // Replace the current process image with the program specified in args.
    // If execvp returns, an error occurred. 

    if (args[0] != NULL && strcmp(args[0], "exit") == 0) {
        exit(0);
    }

    execvp(args[0], args);      //takes a vector input, uses args[0] to find path to direc
    
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
        // Child: execute program 
        child(args, argsc);
        // If child returns, exit with failure
        exit(EXIT_FAILURE);
    } else {
        // Parent: wait for child to finish 
        int status;
        if (waitpid(rc, &status, 0) == -1) {
            perror("waitpid failed");
        }
    }
}


void launch_program_with_redirection(char* args[], int argsc)
{
    int rc = fork();

    if(rc < 0)
    {
        perror("fork failed, terminating process\n");
        return;
    }
    else if(rc == 0)        //child process launch
    {
        // call the specifc child function for redirection
        if(redirection_type == 3){
            child_with_input_redirection(args, argsc);
        }
        else{
            child_with_output_redirection(args, argsc);
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

// checks for redirection operator, stores type in redirection_type
int command_with_redirection(char line[]){
    //scan the input string for < or >
    for(int i = 0; line[i] != '\0'; i++){
        if(line[i] == '>'){
            if(line[i+1] == '>'){
                redirection_type = 2;
                return 2;               //child_with_output_redirection needs calling
            }
            redirection_type=1;
            return 1;                   //child_with_output_redirection needs calling
          
        }
        else if (line[i] == '<'){
            redirection_type=3;
            return 3;       //child_with_input_redirection needs calling
        }
    }
    return 0;       //no redirection detected
}




// child's output goes to file:
// handles "command > file"
void child_with_output_redirection(char *args[], int argsc)   //e.g ls -l > out.txt
{   
    char* file_path = redirection_file(args, argsc);
    int flags = choose_flags(redirection_type);
    
    int file = open(file_path, flags, 0644);
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
    perror("execvp failed");        // if execvp returns theres an error
    return;
    
}

// handles "command < file"
void child_with_input_redirection(char* args[], int argsc)//e.g sort < data.txt
{
    char* file_path = redirection_file(args, argsc);
    int file = open(file_path, O_RDONLY, 0444);

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


//function to get the file path 
char *redirection_file(char *args[], int argsc)
{
    for (int i = 0; i < argsc - 1; i++) {
        if (strcmp(args[i], "<") == 0 || strcmp(args[i], ">") == 0 || strcmp(args[i], ">>") == 0) {
            return args[i + 1];  // returning the filename after the symbol
        }
    }
    return NULL; // no redirection symbol found
}


//function to get the command line arguments for execution
char **redir_exec_args(char *args[], int argsc)
{
    static char *exec_args[MAX_ARGS];
    int count = 0;

    //counting tokens before redirection
    for(int i=0; i<argsc; i++)
    {
        if(strcmp(args[i], "<") == 0 || strcmp(args[i], ">") == 0 || strcmp(args[i], ">>")==0)
        {
            break;    //exit the for loop before redirection
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

//Check if the command is 'cd'
int is_cd(char args[])
{
    for(int i = 0; i < MAX_LINE; ++i){
        if(args[i] == 'c' && args[i+1] == 'd')
        {
            return 1;
        }
    }
    return 0;
}

// change directory to the one specified 
void run_cd(char* args[], int argsc, char lwd[])
{
    const char* home = getenv("HOME");
    //case 1: '' go to home directory
    if(args[1] == NULL)
    {
        printf("no dir: %d\n", chdir(home));
        return;
    }

    //case2: '-' go to lwd, if lwd is not set then bash returns an error
    if(strcmp(args[1], "-") == 0)
    {   
        chdir(lwd);
        char tmp[MAX_PROMPT_LEN];
        strcpy(tmp, cwd);
        strcpy(cwd, lwd);
        strcpy(lwd, tmp);

        return;
    }

    //case3: "FILEPATH" change directory to normal path
    if(chdir(args[1])){
        lwd = cwd;
        strcpy(cwd, args[1]);
        return;
    }
}

void init_lwd(char lwd[])
{
    lwd = getcwd(lwd, MAX_PROMPT_LEN);
}

char directory_stack[128][MAX_LINE];  
int top = -1;
void pushd(const char* dir){}
void popd(){}
void dirs(){}

//check whether input has pipes
int command_with_pipes(char line[]){
    for(int i = 0; line[i] != '\0'; ++i){
        if(line[i] == '|'){
            return 1;
        }
    }

    return 0;
}

//split piped commands 
int tokenise_pipe_commands(char* args[], int argsc, char* cmds_piped[MAX_ARGS][MAX_ARGS]){
    int cmd_index = 0;
    int arg_index = 0;

    for(int i = 0; i < argsc; ++i){
        if(strcmp(args[i], "|") == 0){
            //current command has ended
            cmds_piped[cmd_index][arg_index] = NULL;
            cmd_index++;
            arg_index = 0;
            continue;
        }
        cmds_piped[cmd_index][arg_index++] = args[i];
    }

    cmds_piped[cmd_index][arg_index] = NULL;
    return cmd_index + 1;

}

void print_piped_tokens(char* args[], int argsc){
    char *cmds_piped[MAX_ARGS][MAX_ARGS];

    int num_cmds = tokenise_pipe_commands(args, argsc, cmds_piped);

    printf("Found %d commands:\n", num_cmds);
    for (int i = 0; i < num_cmds; i++) {
        printf("Command %d: ", i);
        for (int j = 0; cmds_piped[i][j] != NULL; j++) {
            printf("%s ", cmds_piped[i][j]);
        }
        printf("\n");
    }
}

// launch commands, connecting previous command output to next command input
void launch_program_with_pipes(char* args[], int argsc){
    char* cmds_piped[MAX_ARGS][MAX_ARGS];
    int num_of_commands = tokenise_pipe_commands(args, argsc, cmds_piped);
    pid_t pids[MAX_ARGS];

    int prevRead = -1;
    int fd[2];
    for(int i = 0; i < num_of_commands; ++i){
        if(i < num_of_commands - 1){
            if (pipe(fd) == -1) {
                perror("pipe failed");
                return;
            }
        }
        
        pids[i] = fork();
        if(pids[i] < 0){
            perror("fork failed");
            return;
        }
        else if (pids[i] == 0){
            if(prevRead != -1){
                if (dup2(prevRead, STDIN_FILENO) == -1) { perror("dup2 failed"); exit(EXIT_FAILURE); }
                close(prevRead);
            }
            if(i < num_of_commands -1){
                if (dup2(fd[1], STDOUT_FILENO) == -1) { perror("dup2 failed"); exit(EXIT_FAILURE); }
                close(fd[1]);
                close(fd[0]);
            }

            // get number of args for current command
            int local_argc = 0;
            while (local_argc < MAX_ARGS && cmds_piped[i][local_argc] != NULL){
                local_argc++;
            }

            // get redirection type for current command
            int redir = 0;
            for (int k = 0; k < local_argc; ++k) {
                if (strcmp(cmds_piped[i][k], ">") == 0) { redirection_type = 1; redir = 1; break; }
                if (strcmp(cmds_piped[i][k], ">>") == 0) { redirection_type = 2; redir = 1; break; }
                if (strcmp(cmds_piped[i][k], "<") == 0) { redirection_type = 3; redir = 1; break; }
            }

            if (redir) {

                if (redirection_type == 3) {
                    child_with_input_redirection(cmds_piped[i], local_argc);
                } else {
                    child_with_output_redirection(cmds_piped[i], local_argc);
                }
                /* If we reach here, exec failed inside the redirection helper */
                perror("execvp (redirection) failed");
                exit(EXIT_FAILURE);

            } 
            else {
                execvp(cmds_piped[i][0], cmds_piped[i]);
                perror("execvp failed");
                exit(EXIT_FAILURE);
            }

        } 
        else{
            // parent closes pipe and preps for next pipe

            if (prevRead != -1) close(prevRead);
            if (i < num_of_commands - 1) {
                close(fd[1]);
                prevRead = fd[0];
            }
            // wait for child to finish
            for(int i = 0; i < num_of_commands; ++i){
                waitpid(pids[i], NULL, 0);
            }
        }

    }
}




