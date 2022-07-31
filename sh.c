#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

/* Defining initial variables */

#define INPUT_BUF_SIZE 1024
#define MAX_ARGS 64

char buf[INPUT_BUF_SIZE]; // Buffer to store read input
ssize_t count; // Determine buffer size
pid_t pid; // Process ID

/*--------------------------------------------------------------*/

/* HELPER METHODS */

/*--------------------------------------------------------------*/

/* Parsing char input to generate the command which is stored in a char array, args */

void buf_to_args() {
    buf[count] = '\0';
    char *token = buf;
    arg = args;
    while ((token = strtok(token, " \t\n")) != NULL) { 
        *arg++ = token;
        token = NULL;
    }
}

/* Checking for multiple input/output files by assessing the placement of redirection symbols in the command */

void check_multiple_files() {
    for (int i = 0; i < MAX_ARGS; i++) {
        if (args[i] != NULL) {
            for (int j = i + 1; j < MAX_ARGS; j++) {
                if ((args[j] != NULL)) {
                    if (!strcmp(args[i], ">") ||
                        !strcmp(args[i], ">>")) {
                            if (!strcmp(args[j], ">") ||
                                !strcmp(args[j], ">>")) {
                                    redirection_error = 5;
                                    break;
                            }
                    } else if (!strcmp(args[i], "<")) {
                        if (!strcmp(args[j], "<")) {
                            redirection_error = 6;
                            break;
                        }
                    }
                } else {
                    break;
                }
            }
        } else {
            break;
        }
    }
}

/* Checking for insufficient input/output files by assessing the placement of redirection symbols in the command */

void check_no_files() {
    for (int i = 0; i < MAX_ARGS; i++) {
        if (args[i] != NULL) {
            if (!strcmp(args[i], ">") || !strcmp(args[i], ">>")) {
                if (args[i + 1] != NULL) {
                    if (!strcmp(args[i + 1], ">") ||
                        !strcmp(args[i + 1], ">>")) { // ensure that these symbols are not followed by other ones
                            redirection_error = 4;
                            break;
                        }
                } else {
                    if (redirection_error != 5) {
                        redirection_error = 3;
                    }
                }
            }
        }
    }
}

/* Ensure that the placement of redirection symbols is meaningful */

void check_misplaced_redirection_symbol() {
    for (int i = 0; i < MAX_ARGS; i++) {
        if (args[i] != NULL) {
            if (!strcmp(args[i], "<")) {
                if (args[i + 1] != NULL) {
                    if (!strcmp(args[i + 1], "<")) { // ensures that this symbol is not followed by another one
                        redirection_error = 7;
                        break;
                    }
                }
            }
        }
    }
}

/* If the redirection error flag is nonzero, print the error that occurred */

void print_redirection_error() {
    if (redirection_error > 0) {
        if (redirection_error == 1) {
            fprintf(stderr, "error: redirects with no command\n");
        } else if (redirection_error == 2) {
            fprintf(stderr, "syntax error: no input file\n");
        } else if (redirection_error == 3) {
            fprintf(stderr, "syntax error: no output file\n");
        } else if (redirection_error == 4) {
            fprintf(
                stderr,
                "syntax error: output file is a redirection symbol\n");
        } else if (redirection_error == 5) {
            fprintf(stderr, "syntax error: multiple output files\n");
        } else if (redirection_error == 6) {
            fprintf(stderr, "syntax error: multiple input files\n");
        } else {
            fprintf(
                stderr,
                "syntax error: input file is a redirection symbol\n");
        }
        print_prompt();
    }
}

/* Handling the cd command */

void handle_cd() {
    if (args[1] == NULL) {
        gen_error = 1;
    } else {
        if (chdir(args[1]) == -1) { 
            perror("cd");
        }
        print_prompt();
    }
}

/* Handling the rm command */

void handle_rm() {
    if (args[1] == NULL) {
        gen_error = 2;
    } else {
        if (unlink(args[1]) == -1) {
            perror("rm");
        }
        print_prompt();
    }
}

/* Handling the ln command */

void handle_ln() {
    if (args[2] == NULL) {
        gen_error = 3;
    } else {
        if (link(args[1], args[2]) == -1) {
            perror("ln");
        }
        print_prompt();
    }
}

/* Handling a correctly placed redirection symbol, opening the inputted files */

void handle_redirection(char *args) {
    for (int i = 0; i < (MAX_ARGS - 2); i++) {
        if (args[i] != NULL) {
            if (!strcmp(args[i], ">")) {
                close(1);
                if (open(args[i + 1], 
                        O_WRONLY | O_CREAT | O_TRUNC, // file permissions
                        0600) == -1) {
                    perror(args[i + 1]);
                    exit(1);
                }
                args[i] = args[i + 2];
            } else if (!strcmp(args[i], ">>")) {
                close(1);
                if (open(args[i + 1],
                        O_WRONLY | O_CREAT | O_APPEND,
                        0600) == -1) {
                    perror(args[i + 1]);
                    exit(1);
                }
                args[i] = args[i + 2];
            } else if (!strcmp(args[i], "<")) {
                close(0);
                if (open(args[i + 1], O_RDONLY, 0600) == -1) {
                    perror(args[i + 1]);
                    exit(1);
                }
                args[i] = args[i + 2];
            }
        }
    }
}

/* Exec the child process by getting the path to the file to be executed, and a null-terminated argument vector argv */

void exec_child_process() {
    int argc;
    for (int i = 0; i < MAX_ARGS; i++) {
        if (args[i] == NULL) {
            argc = i + 1; // determine length of argv and ensure it is null-terminated
            break;
        }
    }
    char *argv[argc];
    for (int i = 0; i < argc; i++) {
        argv[i] = args[i]; // getting argument vector from original args array
    }
    if (!strncmp(argv[0], "/bin/", 5)) {
        argv[0] = strtok(argv[0], "/bin/"); // remove possible /bin/ from argv before execing 
    }
    if (execv(args[0], argv) == -1) { // passing in the filepath and argument argv
        perror("execv");
    }
    exit(0);
}

/* If the general error flag is nonzero, print the error that occurred */

void print_gen_error() {
    if (gen_error == 1) {
        fprintf(stderr, "cd: syntax error\n");
    } else if (gen_error == 2) {
        fprintf(stderr, "rm: syntax error\n");
    } else if (gen_error == 3) {
        fprintf(stderr, "ln: syntax error\n");
    }
    print_prompt();
}

/*--------------------------------------------------------------*/

/* MAIN METHOD: Utilizing a recursive method print_prompt() to implement a REPL that reprints the prompt after the command has been executed. Another approach would be to use a continuous while loop. */

/*--------------------------------------------------------------*/

void print_prompt() {
#ifdef PROMPT
    if (printf("33sh> ") < 0) {
        fprintf(stderr, "printf");
        exit(1);
    }
    if (fflush(stdout) < 0) {
        fprintf(stderr, "fflush");
    }
#endif
    char *args[MAX_ARGS] = {NULL};
    char **arg;

    /* Flags to designate redirection errors. The flags' values are read before commands are interpreted. */

    int redirection_error = 0; 
    int gen_error = 0;

    /* Reading string input and converting it to an array */

    if ((count = read(0, buf, INPUT_BUF_SIZE)) == -1) {
        perror("read");
        exit(1);
    } else {
        buf_to_args()
        if (args[0] == NULL) {  // Checking for non-null argument
            print_prompt();
        } else { // Checking for errors and printing error messages
            check_multiple_output()
            check_no_output()
            check_misplaced_redirection_symbol()
            print_redirection_error() // Handling commands
            if (!strcmp(args[0], "cd")) {
                handle_cd()
            } else if (!strcmp(args[0], "rm")) {
                handle_rm()
            } else if (!strcmp(args[0], "ln")) {
                handle_ln()
            } else if (!strcmp(args[0], "exit")) {
                exit(0);
            } else {
                if ((pid = fork()) == 0) { // Handling child processes
                    handle_redirection()
                    exec_child_process()
                }
                wait(0);
                print_prompt();
            }
            if (gen_error > 0) {
                print_gen_error()
            }
        }
    }
}

int main() {
    print_prompt();
    return 0;
}
