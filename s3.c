#include "s3.h"
#define MAX_ARGS 128

char cwd[MAX_PROMPT_LEN];
char lwd[MAX_PROMPT_LEN];

int redirection_type;
char *shell_argv0 = NULL;


///Simple for now, but will be expanded in a following section
void construct_shell_prompt(char shell_prompt[], char lwd[])
{
    if (getcwd(cwd, MAX_ARGS) == NULL)
    {
        perror("getcwd failed");
        cwd[0] = '\0';
    }

    snprintf(shell_prompt, MAX_PROMPT_LEN+13, "\033[34m[%s]\033[0m$ ", cwd);
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
void run_cd(char* args[], int argsc, char lwd[]){
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
    if(!chdir(args[1])){
        strcpy(lwd, cwd);
        strcpy(cwd, args[1]);
        return;
    }
}

void init_lwd(char lwd[]){
    lwd = getcwd(lwd, MAX_PROMPT_LEN);
}


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
int tokenise_pipe_commands(char* args[], int argsc, char* cmds_piped[MAX_ARGS][MAX_ARGS]) {
    int cmd_index = 0;
    int arg_index = 0;
    int paren_depth = 0;

    for (int i = 0; i < argsc; ++i) {
        char *tok = args[i];

        // split on | at top level not within the subshell
        if (strcmp(tok, "|") == 0 && paren_depth == 0) {
            cmds_piped[cmd_index][arg_index] = NULL;
            cmd_index++;
            arg_index = 0;
            continue;
        }

        // Otherwise, this token is part of the current command
        cmds_piped[cmd_index][arg_index++] = tok;

        // updating the parentheses depth based on characters in this token
        for (char *p = tok; *p != '\0'; ++p) {
            if (*p == '(')
                paren_depth++;
            else if (*p == ')')
                paren_depth--;
        }
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
                if (dup2(prevRead, STDIN_FILENO) == -1) { 
                    perror("dup2 failed"); exit(EXIT_FAILURE); }
                close(prevRead);
            }
            if(i < num_of_commands -1){
                if (dup2(fd[1], STDOUT_FILENO) == -1) {
                    perror("dup2 failed"); exit(EXIT_FAILURE); }
                close(fd[1]);
                close(fd[0]);
            }

            //check if this command is an inner shell

            char temp_cmd[MAX_LINE];
            reconstruct_segment(cmds_piped[i], temp_cmd);// joining the tokens of cmds_piped[i] into one string
            trim_whitespace(temp_cmd);

            char inner[MAX_LINE];

            if (is_subshell_segment(temp_cmd, inner, sizeof(inner))) {
                //run the inner shell
                char *argv_sub[] = { shell_argv0, "-c", inner, NULL };
                execvp(shell_argv0, argv_sub);
                perror("execvp subshell in pipe failed");
                exit(EXIT_FAILURE);
            }

            //not a subshell
            // get number of args for current command
            int local_argc = 0;
            while (local_argc < MAX_ARGS && cmds_piped[i][local_argc] != NULL){
                local_argc++;
            }

            // get redirection type for current command
            int redir = 0;
            for (int k = 0; k < local_argc; ++k) {
                if(strcmp(cmds_piped[i][k], ">") == 0){ 
                    redirection_type = 1; redir = 1; break; 
                }
                if(strcmp(cmds_piped[i][k], ">>") == 0){ 
                    redirection_type = 2; redir = 1; break; 
                }
                if(strcmp(cmds_piped[i][k], "<") == 0){ 
                    redirection_type = 3; redir = 1; break; 
                }
            }

            if (redir){
                if (redirection_type == 3){
                    child_with_input_redirection(cmds_piped[i], local_argc);
                } 
                else {
                    child_with_output_redirection(cmds_piped[i], local_argc);
                }
                //somthings gone wrong :(
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
        }
    }

    // wait for child to finish
    for(int i = 0; i < num_of_commands; ++i){
        waitpid(pids[i], NULL, 0);
    }
}

//batched commands detection
int has_batched_commands(char line[]){
    for(int i = 0; line[i] != '\0'; ++i){
        if(line[i] == ';'){
            return 1;
        }
    }

    return 0;
}

//batched commands tokenisation
void tokenise_batched_commands(char line[], char* commands[], int* command_count)
{
    *command_count = 0;
    char *start = line;
    char *p = line;

    int paren_depth = 0;

    while (*p != '\0') {

        if (*p == '(')
            paren_depth++;

        else if (*p == ')')
            paren_depth--;

        else if (*p == ';' && paren_depth == 0) {
            // End of a top-level command
            *p = '\0';
            commands[(*command_count)++] = start;
            start = p + 1;
        }

        p++;
    }

    // last command
    if (*start != '\0')
        commands[(*command_count)++] = start;
}


int command_with_pipes_flag(char* args[], int argsc) {
    for (int i = 0; i < argsc; i++) {
        if (strcmp(args[i], "|") == 0)
            return 1;
    }
    return 0;
}

int command_with_redirection_flag(char* args[], int argsc) {
    for (int i = 0; i < argsc; i++) {
        if (!strcmp(args[i], ">") ||!strcmp(args[i], ">>") ||!strcmp(args[i], "<"))
            return 1;
    }
    return 0;
}



//subshell functions
void trim_whitespace(char *s) {
    
    while (*s == ' ' || *s == '\t') s++;

    char *end = s + strlen(s) - 1;
    while (end > s && (*end == ' ' || *end == '\t'))
        end--;
    *(end + 1) = '\0';
}

//checks if the input command is a subshell
int is_subshell_segment(const char *line, char *inner, int inner_sz) {
    const char *start = line;

    
    while (*start == ' ' || *start == '\t')
        start++;

    // check if the first character is not a whitespace but a opening (
    if (*start != '(')
        return 0;

    // end of the string
    const char *end = line + strlen(line) - 1;

    
    while (end > start && (*end == ' ' || *end == '\t'))
        end--;

    // checks if the last non spaced character of the string is a bracket
    if (*end != ')')
        return 0;

    // extracting content within the subshell command
    start++; 
    int len = end - start;

    if (len <= 0)
        return 0;

    // copying the inner command within the brackets into inner
    if (len >= inner_sz)
        len = inner_sz - 1;

    memcpy(inner, start, len);
    inner[len] = '\0';

    return 1;
}

//run the subshell with execvp
void run_subshell(const char *inner) {
    int rc = fork();

    if (rc < 0) {
        perror("fork failed");
        return;
    }

    if (rc == 0) {
        // run the subshell same path execept now with -c to symbolise its a subshell for rest of the subshell program
        char *argv_sub[] = { shell_argv0, "-c", (char*)inner, NULL };
        execvp(shell_argv0, argv_sub);
        perror("execvp subshell failed");
        _exit(1);
    }

    int status;
    waitpid(rc, &status, 0);
}

//executed in the while loop
void execute_line(char *line, char lwd[]){
    // the subshell variables to store
    char *args[MAX_ARGS];
    int argsc = 0;

    char *commands[MAX_ARGS];
    int command_count = 0;

    //detection and running of subshells
    char inner[MAX_LINE];
    if (is_subshell_segment(line, inner, sizeof(inner))) {
        run_subshell(inner);
        return;
    }

    //handling batched commands
    if (has_batched_commands(line)) {

        //tokenising the input commands
        tokenise_batched_commands(line, commands, &command_count);

        // process each command within the batch independently
        for (int i = 0; i < command_count; i++) {
            trim_whitespace(commands[i]);

            // detect subshell as a command within the batch and if so run as subshell
            char inner[MAX_LINE];
            if (is_subshell_segment(commands[i], inner, sizeof(inner))) {
                run_subshell(inner);
                continue; 
            }

            // Parse the sub command
            parse_command(commands[i], args, &argsc);
            
            //if there was no argument passed
            if (argsc == 0){
                continue;
            }

            // cd
            if (strcmp(args[0], "cd") == 0) {
                run_cd(args, argsc, lwd);
                continue;
            }

            if(strcmp(args[0], "popd") == 0 || strcmp(args[0], "pushd") == 0 || strcmp(args[0], "dirs") == 0){
                if(strcmp(args[0], "pushd") == 0){
                    printf("Pushing directory onto stack...\n");
                    pushd(args, argsc);
                }
                else if(strcmp(args[0], "popd") == 0){
                    printf("Popping directory from stack...\n");
                    popd();
                }
                else if(strcmp(args[0], "dirs") == 0){
                    printf("Displaying directory stack...\n");
                    dirs();
                }
                continue;
            }

            // pipes
            if (command_with_pipes_flag(args, argsc)) {
                launch_program_with_pipes(args, argsc);
                continue;
            }

            // redirection if no pipes
            if (command_with_redirection_flag(args, argsc)) {
                launch_program_with_redirection(args, argsc);
                continue;
            }

            // Basic command
            launch_program(args, argsc);
        }

        return;
    }

   
    //single commands handled here
  
    if (command_with_pipes(line)) {
        parse_command(line, args, &argsc);
        launch_program_with_pipes(args, argsc);
        return;
    }

    if (is_cd(line)) {
        parse_command(line, args, &argsc);
        run_cd(args, argsc, lwd);
        return;
    }

    if (command_with_redirection(line)) {
        parse_command(line, args, &argsc);
        launch_program_with_redirection(args, argsc);
        reap();
        return;
    }

    if(is_directory_command(line)){
        parse_command(line, args, &argsc);
        if(strcmp(args[0], "pushd") == 0){
            //print_tokens(args, argsc);
            printf("Pushing directory onto stack...\n");
            pushd(args, argsc);
        }
        else if(strcmp(args[0], "popd") == 0){
            printf("Popping directory from stack...\n");
            popd();
        }
        else if(strcmp(args[0], "dirs") == 0){
            printf("Displaying directory stack...\n");
            dirs();
        }
        return;
    }

    // plain command
    parse_command(line, args, &argsc);
    if (argsc > 0) {
        launch_program(args, argsc);
        reap();
    }
}

//reconstruct segment
void reconstruct_segment(char *cmd[], char *outbuf) {
    outbuf[0] = '\0';
    for (int i = 0; cmd[i] != NULL; i++) {
        strcat(outbuf, cmd[i]);
        strcat(outbuf, " ");
    }
    int len = strlen(outbuf);
    if (len > 0 && outbuf[len - 1] == ' ') {
        outbuf[len - 1] = '\0';
    }
}


DirStack dirStack = { .top = -1 };

int is_directory_command(const char* dir){
    if(dir == NULL){
        return 0;
    }
    if(strncmp(dir, "pushd", 4) == 0 || strncmp(dir, "popd", 4) == 0 || strncmp(dir, "dirs", 4) == 0){
        return 1;
    }
    return 0;
}

void init_directory_stack()
{
    char current_dir[MAX_PROMPT_LEN];
    if (getcwd(current_dir, sizeof(current_dir)) != NULL) {
        strncpy(dirStack.directories[0], current_dir, MAX_LINE - 1);
        dirStack.directories[0][MAX_LINE - 1] = '\0';
        dirStack.top = 0;
    } else {
        perror("getcwd failed in init_directory_stack");
        dirStack.top = -1;
    }
}

int directory_exists(const char* path){
    struct stat info;
    if(stat(path, &info) != 0){
        return 0;
    }
    else if(info.st_mode & S_IFDIR){
        return 1;
    }
    return 0;
}

void pushd(char* args[], int argsc)
{
    
    if(dirStack.top >= 127){
        printf("pushd: Directory stack full!\n");
        return;
    }       

    if(argsc < 2){
        printf("pushd: No directory specified!\n");
        return;
    }

    if(!directory_exists(args[1])){
        printf("pushd: Directory '%s' aint real cuzzy\n", args[1]);
        return;
    }

    chdir(args[1]);

    dirStack.top++;
    strncpy(dirStack.directories[dirStack.top], args[1], MAX_LINE - 1);
    dirStack.directories[dirStack.top][MAX_LINE - 1] = '\0';
    printf("directory top: %s\n", dirStack.directories[dirStack.top]);

    dirs();
    return;
    
}

void popd()
{
    if(dirStack.top <= 0){
        printf("popd : Directory stack empty!\n");
        return;
    }

    dirStack.top--;
    dirStack.directories[dirStack.top + 1][0] = '\0';
    dirs();
    return;
}

void dirs()
{    
    for(int i = dirStack.top; i >=0; --i){
            printf("%s\n", dirStack.directories[i]);
    }
}
















