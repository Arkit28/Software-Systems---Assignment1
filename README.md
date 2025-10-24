<<<<<<< Updated upstream
# Software-Systems---Assignment1
Shell in C
by arhcit and shashank
hi
=======
# Software-Systems---Assignment1
Shell in C yuddddddyyyyyyyy gang


Current Task - Commands with redirection: 
Parse input as normal. If there is ">" then we know that the output of the left command needs to be set as the input of the right command. This is redirection of the output of one command to the next. 
Functions (as of what my brain has clocked right now): 
- child_with_output_redirected : takes the output of child process and sends it to where it needs to go (called from laucnh_program_with_redirection)
- child_with_input_redirected : takes input from a redirected process (called from launch_program_with_redirection)
- redirection_operator_found : parse input to find redirection operators (only considering ">" for now)

Considerations:
- open syscall too access input or output file before redirection using dup2()
- ">>" will need to be implemented too at a later stage
- learn file handing like STDIN, STDOUT, STDERR better before implementing stuff
- Learn dup(), dup2() and execvp() syscalls in depth before implementing stuff 
