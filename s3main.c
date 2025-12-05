#include "s3.h"


int main(int argc, char *argv[]){

    //stores programs own path
    shell_argv0 = argv[0];

    // if the user ran  s3 -c command then run only the command part and execute right after
    if (argc > 2 && strcmp(argv[1], "-c") == 0) {

        // copying the subshell command into the line buffer
        char line[MAX_LINE];
        strncpy(line, argv[2], MAX_LINE - 1);
        line[MAX_LINE - 1] = '\0';

        char lwd[MAX_PROMPT_LEN];
        init_lwd(lwd);

        // here we run he single line of command in the subshell
        execute_line(line, lwd);

        return 0;
    }

    ///Stores the command line input
    char line[MAX_LINE];

    ///The last (previous) working directory 
    char lwd[MAX_PROMPT_LEN]; 

    init_lwd(lwd);///Implement this function: initializes lwd with the cwd (using getcwd)

    
    strncpy(dirStack.directories[0], lwd, MAX_LINE - 1);
    dirStack.directories[0][MAX_LINE - 1] = '\0';
    dirStack.top = 0;

    strncpy(dirStack.directories[1], "/mnt/c/projects", MAX_LINE - 1);
    dirStack.directories[1][MAX_LINE - 1] = '\0';
    dirStack.top = 1;

    strncpy(dirStack.directories[2], "/mnt/c/programming/year2", MAX_LINE - 1);
    dirStack.directories[2][MAX_LINE - 1] = '\0';
    dirStack.top = 2;

    while (1) {

        read_command_line(line, lwd);

        execute_line(line,lwd);

    }
}