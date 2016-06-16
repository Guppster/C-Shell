#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <string.h> 
#include <sys/wait.h> 
#define MAX 256
#define CMD_MAX 10

/*
   This function takes as input the following:
buf: This represents the string for which the tokens are to be determined for  
tokens: This represents the array that the found tokens are tbe put into

The function strtok() is used to find the tokens.  The delimiter used
to distinguish tokens is a space
*/

char history[CMD_MAX][MAX]; 
int start = 0;
int size = 0;
int n = 0;

void getHistory();
int launch(char **args);
int process(char **args);
void writeToHistory(char *element);
void checkForPipe(char **args, int* pipes);

int make_tokenlist(char *buf, char *tokens[])
{
        char input_line[MAX];
        char *line;
        int i,n;

        i = 0;

        line =   buf;
        tokens[i] = strtok(line, " ");
        do
        {
                i++;
                line = NULL;
                tokens[i] = strtok(line, " ");
        } while(tokens[i] != NULL);

        return i;
}

void checkForPipe(char **args, int* pipes)
{
        //element 0 is boolean value indicating pipe or no pipe
        //element 1 is the index of the first char before the pipe 
        //element 2 is the index of the first char after the pipe

        pipes[0] = 0;
        pipes[1] = 0;
        pipes[2] = 0;

        for(int i = 0; i < n; i++)
        {
                if(strcmp(args[i], "|") == 0)
                {
                        pipes[0] = 1;
                        pipes[2] = i + 1;
                }
        }

}

int launch(char **args)
{
        pid_t pid, wpid;
        int fd[2];
        int doPipe[5];
        int i = 0;

        checkForPipe(args, doPipe);

        char* argsA = calloc(n, sizeof(char));
        char* argsB = calloc(n, sizeof(char));
        
        if(doPipe[0] == 1)
        {
                while(i < doPipe[2]-1)
                {
                        argsA[i] = args[i];
                        i++;
                }

                printf("[%s] and [%s]\n", argsA, argsB);
                //strcpy(argsB[0], args[doPipe[2]]);

                if(pipe(fd) < 0) perror("Pipe Error");
        }

        pid = fork();

        //If it is a child process
        if(pid == 0)
        {
                if(doPipe[0] == 1)
                {
                        close(fd[1]);
                        if(dup2(fd[0], STDOUT_FILENO) < 0)
                        {
                                perror("Dup Error");
                                exit(1);
                        }
                }

                if(execvp(argsB[0], argsB) < 0)
                {
                        perror("Exec Problem");
                        exit(1);
                }
        }
        else if(pid < 0)
        {
                perror("Problem Forking");
                exit(-1);
        }
        else //Parent Process
        {

                if(doPipe[0] == 1)
                {
                        close(fd[0]);
                        if(dup2(fd[1], STDOUT_FILENO) < 0)
                        {
                                perror("Dup Error");
                                exit(1);
                        }
                }
                if(execvp(argsA[0], argsA) < 0)
                {
                        perror("Exec Problem");
                        exit(1);
                }
        }

        return 1;
}

int process(char **args)
{
        strtok(args[0], "\n");

        if(strcmp(args[0], "exit") == 0)
        {
                exit(EXIT_SUCCESS);
        }
        else if(strcmp(args[0], "history") == 0)
        {
                getHistory();
                return 0;
        }

        return launch(args);
}

void getHistory()
{
        for(int i = 0; i < size; i++)
        {
                printf("[%s]\n", history[(start+ i) % CMD_MAX]);
        }
}

void writeToHistory(char *element)
{
        strtok(element, "\n");
        if(size < CMD_MAX)
        {
                strcpy(history[size++], element);
        }
        else if(size == CMD_MAX)
        {
                strcpy(history[start++], element);
                start %= CMD_MAX;
        }
}

void loop(void)
{
        char input_line[MAX], *tokens[CMD_MAX];
        int i;

        while(1)
        {
                printf("Singh> ");

                if (fgets(input_line,MAX,stdin) != NULL) 
                {
                        writeToHistory(input_line);
                        n = make_tokenlist(input_line, tokens);
                }          

                process(tokens);
        } 
}

int  main(void) 
{
        loop();
        return EXIT_SUCCESS;
}
