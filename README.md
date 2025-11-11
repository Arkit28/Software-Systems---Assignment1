<<<<<<< Updated upstream
# Software-Systems---Assignment1
Shell in C
by arhcit and shashank
hi
=======
# Software-Systems---Assignment1
Shell in C yuddddddyyyyyyyy gang


CURRENT TASK: 
Task 5 - Batched commands:
Execution of multiple commands independently in a sequential manner, regardless of whether previous commands were successful. It uses a semicolon to indicate the end of a command line.

To implement:
- detect batched commands in the input line
- split batched commands into their own commands, then execute them separately in order

Consideration:
- ensure shell continues to execute commands whether or not previous ones succeeded
- consider whether commands have pipes, redirection etc



COMPLETED TASKS:
Task 4 - Commands with pipes:
Interrprocess communication i.e. commands sending data to each other. To implement will need to use the pipe() function alongside dup2(). Refer to the assignment page for a walkthrough example of how it should work from a high level

To implement:
- detect whether a pipe is in the command line
- tokenize the piped commands
- functions to launch piped commands

Considerations:
- read up on pipes properly
- redirection within piped commands
- main is not included so consider what changes will need to be made to enable piped commands
- tokenise by pipes perhaps



Task 3 - Implementing cd:
To implement the cd command, the chdir() syscall needs to be used. In summary the chdir() takes in the inputs we would expect to pass to cd. There are special . and .. directories which can be passed directly into chdir().
"cd -" goes to the previous directory, so some method of storage will be needed.

To implement:
- ability to change directory
- ability to go to previous directory, like "cd -"
    - This only one variable to store the previous directory, so its really simple imo
- print current working directory (cwd) for each prompt, to look something like [/projects/s3/ s3]$
- EXTENSION: consider implmenting pushd, popd, and dirs (these are just stack push, pop and printing the stack respectively) for directory management (ONGOING)

Considerations:
- method of storing previous directory, I think a stack is best
- read chdir() and getcwd() man pages



Task 2 - Commands with redirection: 
Parse input as normal. If there is ">" then we know that the output of the left command needs to be set as the input of the right command. This is redirection of the output of one command to the next. 
Functions (tbc): 
- child_with_output_redirected : takes the output of child process and sends it to where it needs to go (called from laucnh_program_with_redirection)
- child_with_input_redirected : takes input from a redirected process (called from launch_program_with_redirection)
- redirection_operator_found : parse input to find redirection operators (only considering ">" for now)

Considerations:
- open syscall too access input or output file before redirection using dup2()
- ">>" will need to be implemented too at a later stage
- learn file handing like STDIN, STDOUT, STDERR better before implementing stuff
- Learn dup(), dup2() and execvp() syscalls in depth before implementing stuff 



