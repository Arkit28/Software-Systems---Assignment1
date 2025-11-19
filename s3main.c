#include "s3.h"



int main(int argc, char *argv[]){

    // Store the path of the shell executable for later use inside subshells.
    // (argv[0] always contains the path used to launch this program.)
    // We save it globally so run_subshell() knows what binary to exec.
    shell_argv0 = argv[0];
    // If the user ran:  s3 -c commands
    // then run ONLY that command and exit immediately.
    // argv[1] = "-c"
    // argv[2] = the string of commands to execute.
    if (argc > 2 && strcmp(argv[1], "-c") == 0) {

        // Copy the command string safely into a local buffer.
        // "line" will be passed to execute_line() exactly as if
        // the user typed it in interactive mode.
        char line[MAX_LINE];
        strncpy(line, argv[2], MAX_LINE - 1);
        line[MAX_LINE - 1] = '\0';

        char lwd[MAX_PROMPT_LEN];
        init_lwd(lwd);

        // Run the single line of commands.
        execute_line(line, lwd);

        return 0;
    }

    ///Stores the command line input
    char line[MAX_LINE];

    ///The last (previous) working directory 
    char lwd[MAX_PROMPT_LEN]; 

    init_lwd(lwd);///Implement this function: initializes lwd with the cwd (using getcwd)



    while (1) {

        read_command_line(line, lwd);

        execute_line(line,lwd);

    }
}