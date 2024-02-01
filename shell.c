#include "lexer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_INPUT_SIZE 1024

int main() {
    while (1) {
        char *user = getenv("USER"); //gets user info
        char *machine = getenv("MACHINE"); //gets machine info
        char cd[MAX_INPUT_SIZE]; //creates an array cd to store current working dir

        //gets cwd size and current working dir and checks to see if there are any
        if (getcwd(cd, sizeof(cd)) == NULL) {
            perror("getcwd");
            return 1;
        }

        //The prompt of the shell shows the user@machine:~cwd(currentdir)>
        //ex. johnGD@linprog.cs.fsu1:/User/Home/dir>
        printf("%s@%s:~%s> ", user, machine, cd);

        //gets input
        char *input = get_input();
        //turns input into token
        tokenlist *tokens = get_tokens(input);

        //Allows user to exit the function by going through tokens 
        for (int i = 0; i < tokens->size;++i) {
            if (strcmp(tokens->items[i], "exit") == 0) {
                printf("Now exiting shell...\n");
                free(input);
                free_tokens(tokens);
                return 0;
            }
        }

        //creates a child process and stores the fork() id process in cpid (child process)
        pid_t cpid = fork();

        //if child process address is -1 meaning there is no child process
        if (cpid == -1) {
            printf("fork() failed!");
            return 1;
        } 
        //if cpid is 0 then there is a child process and to proceed
        else if (cpid == 0) {

            //sets the args to the size of MAX_INPUT_SIZE
            char* args[MAX_INPUT_SIZE];
            //counts the number of arguments
            int args_count = 0;

            //for iterates through token
            for (int i = 0; i < tokens->size; i++) {
                //if the first token is $ then it proceeds to call getenv and skips the $ sign to see what is called
                if (tokens->items[i][0] == '$') {
                    char *env = getenv(tokens->items[i] + 1);
                    //if there is more than just the $ then tokens are freed and strdup(env) is called
                    if (env != NULL) {
                        free(tokens->items[i]);
                        tokens->items[i] = strdup(env);
                    }
                    //prints the desired environmental variable
                    printf("%s\n", tokens->items[i]);
                }
                //if token[0] is equal to ~ then it checks to see if it has a ~/ or a null character
                else if (tokens->items[i][0] == '~') {
                    if (tokens->items[i][1] == '/' || tokens->items[i][1] == '\0') {
                        //Gets home environment
                        char *home = getenv("HOME");
                        //gets the length of environment home
                        size_t homelen = strlen(home);
                        //gets the remaining length of the token
                        size_t remaininglen = strlen(tokens->items[i] + 1);
                        //adds the two to get the newlength
                        size_t newlen = homelen + remaininglen + 1;
                        char *newpath = (char *)malloc(newlen);
                        //prints the path
                        snprintf(newpath, newlen + 1, "%s%s", home, tokens->items[i] + 1);
                        printf("%s\n",  newpath);
                    }
                }
                else 
                {
                    args[args_count++] = tokens->items[i];
                }
            }



            args[args_count] = NULL;

            if (execvp(args[0], args) == -1) {
                printf("%s: Command not found.\n", input);
                return 1;
            }
        }
        
        else {
            int status;
            waitpid(cpid, &status, 0);
        }

        free(input);
        free_tokens(tokens);
    }

}


char *get_input(void) {
    char *buffer = NULL;
    int bufsize = 0;
    char line[5];
    while (fgets(line, 5, stdin) != NULL)
    {
        int addby = 0;
        char *newln = strchr(line, '\n');
        if (newln != NULL)
            addby = newln - line;
        else
            addby = 5 - 1;
        buffer = (char *)realloc(buffer, bufsize + addby);
        memcpy(&buffer[bufsize], line, addby);
        bufsize += addby;
        if (newln != NULL)
            break;
    }
    buffer = (char *)realloc(buffer, bufsize + 1);
    buffer[bufsize] = 0;
    return buffer;
}

tokenlist *new_tokenlist(void) {
    tokenlist *tokens = (tokenlist *)malloc(sizeof(tokenlist));
    tokens->size = 0;
    tokens->items = (char **)malloc(sizeof(char *));
    tokens->items[0] = NULL; /* make NULL terminated */
    return tokens;
}

void add_token(tokenlist *tokens, char *item) {
    int i = tokens->size;

    tokens->items = (char **)realloc(tokens->items, (i + 2) * sizeof(char *));
    tokens->items[i] = (char *)malloc(strlen(item) + 1);
    tokens->items[i + 1] = NULL;
    strcpy(tokens->items[i], item);

    tokens->size += 1;
}

tokenlist *get_tokens(char *input) {
    char *buf = (char *)malloc(strlen(input) + 1);
    strcpy(buf, input);
    tokenlist *tokens = new_tokenlist();
    char *tok = strtok(buf, " ");
    while (tok != NULL)
    {
        add_token(tokens, tok);
        tok = strtok(NULL, " ");
    }
    free(buf);
    return tokens;
}

void free_tokens(tokenlist *tokens) {
    for (int i = 0; i < tokens->size; i++)
        free(tokens->items[i]);
    free(tokens->items);
    free(tokens);
}
