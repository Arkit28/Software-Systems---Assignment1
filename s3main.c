#include "s3.h"

int main(int argc, char *argv[]){

    ///Stores the command line input
    char line[MAX_LINE];

    ///The last (previous) working directory 
    char lwd[MAX_PROMPT_LEN-6]; 

    init_lwd(lwd);///Implement this function: initializes lwd with the cwd (using getcwd)

    //Stores pointers to command arguments.
    ///The first element of the array is the command name.
    char *args[MAX_ARGS];

    ///Stores the number of arguments
    int argsc;

    //each element points to one whole command string extracted from the input line
    char *commands[MAX_ARGS];

    //counts the number of seperate comands which were found in the batched commands input
    int command_count = 0;


    while (1) {

        read_command_line(line, lwd);


        //check if its a batched command
        if (has_batched_commands(line)) {

            //Spliting the line into commands
            tokenise_batched_commands(line, commands, &command_count);

            //Process each command independently
            for (int i = 0; i < command_count; i++) {

                // Parsing the sub command
                parse_command(commands[i], args, &argsc);

                if (argsc == 0) continue; //this is for an empty command

                //cd
                if (strcmp(args[0], "cd") == 0) {
                    run_cd(args, argsc, lwd);
                    continue;//exit the current iteration of for loop
                }

                //pipes
                if (command_with_pipes_flag(args, argsc)) {
                    launch_program_with_pipes(args, argsc);
                    continue;
                }

                // redirection if no pipes
                else if (command_with_redirection_flag(args, argsc)) {
                    launch_program_with_redirection(args, argsc);
                    continue;
                }

                // Basic command
                launch_program(args, argsc);
            }
        }

        //no batching has taken place so execute the following normal instructions
        else {

            if(command_with_pipes(line)) {
                parse_command(line, args, &argsc);
                launch_program_with_pipes(args, argsc);
            }
            else if(is_cd(line)) {
                parse_command(line, args, &argsc);
                run_cd(args, argsc, lwd);
            }
            else if(command_with_redirection(line)) {
                parse_command(line, args, &argsc);
                launch_program_with_redirection(args, argsc);
                reap();
            }
            else {
                parse_command(line, args, &argsc);
                launch_program(args, argsc);
                reap();
            }
        }
    }
}