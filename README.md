# shell
Implemented a Shell from scratch using C. Running in a Linux virtual machine, the Shell parses terminal input and executes commands by running scripts and forking and handling child processes. It also performs extensive error checking for Syscalls and bad user input.

Visit sh.c to view my code.

How to compile: make clean all

My program runs recursively using a single function, print_prompt(), with many if-clauses within. Here is how it runs: 

1. Checks if the -PROMPT flag is included. If it is, the program prints 33sh> as a prompt. 

2. Defines the args array, a pointer to a single arg, and the redirection_error and gen_error ints, which will be used to tell the program which error to throw. 

3. Reads input from file descriptor 0 (keyboard input) to the buffer.

4. Uses strtok to divide every piece of input into a separate argument, which are all placed in the args array. 

5. Checks if the first argument entered is null, in which case the program just re-runs and prompts again. 

6. Checks for doubly entered >, >>, and <

7. Checks for arguments to redirection symbols that are redirection symbols.

8. Prints redirection errors based on the number that was assigned in error checking. 

9. Cd, rm, ln, and exit. Throwing errors accordingly. 

10. Forking. Checks for redirection symbols and opens files accordingly, removing the redirection symbols and the following file from the argument. Then executing the argument. 

11. General errors assigned from cd, ln, rm. 
