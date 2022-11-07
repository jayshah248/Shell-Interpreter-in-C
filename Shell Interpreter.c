#include <stdio.h>//include library
#include <string.h>//include library
#include <unistd.h>//include library
#include <stdlib.h>//include library
//#include <sys/wait.h>//include library
#include <signal.h>//include library

int extractCommands(char *input, char *commands[]) //function extract total commands from input
{
    int cmdCount = 0; //counter
    char *token; //varaible to store token
    token = strtok(input, ";"); //seperate tokens based on delimiter ";"
    while (token != NULL) //loop to extract all tokens
    {
        commands[cmdCount] = token; //updating the commands array to store every command
        cmdCount++;//increment counter
        token = strtok(NULL, ";");//continue extracting tokens
    }
    return cmdCount;//returning total commands found
}

int extractArguments(char *input, char *arguments[]) //function to extract arguments from a command, the first argument will be the name of the command
{
    int total_arguments = 0;//counter

    char *token;//varaible to store token
    token = strtok(input, " \n"); //seperate tokens based on delimiter " \n"
    while (token != NULL)//loop to extract all tokens
    {
        arguments[total_arguments] = token;//updating the arguments array
        total_arguments++;//increment counter
        token = strtok(NULL, " \n");//continue extracting tokens
    }
    arguments[total_arguments] = NULL;//last argument has to be NULL for the execvp function

    return total_arguments;//return the total number of arguments found
}

int extractPipes(char *input, char *processes[]) //function to extract all the piped processes and return total number of piped processes
{
    int totalPipedProcesses = 0; //counter to count piped processes, it counts processes not pipes

    char *token; //variable to store tokens
    token = strtok(input, "|"); //extracting tokens i.e. processes seperated by "|"(pipes) from the command
    while (token != NULL)//loop to extract all tokens
    {
        processes[totalPipedProcesses] = token; //processes that are piped are stored in processes array
        totalPipedProcesses++;//increment counter
        token = strtok(NULL, "|");//continue extracting tokens
    }
    return totalPipedProcesses;//return total number of piped processes found (it is number of processes, not pipes)
}

int executeProcess(char *arguments[]) //function to execute a process using the execvp function, takes only arguments array as input as first element of the arguments array cotains the name of the program
{

    if(strcmp(arguments[0],"cd") == 0)//checks if the first element of the arguments is cd
    {
        if(chdir(arguments[1]) < 0)//if it is cd then I execute the cd command in c using chdir function
        {
            printf("\nPlease Enter Directory Name Carefully"); //if the chdir function is not sucessfull then print this
        }
        return 0;//exit the function
    }
    else if(strcasecmp(arguments[0], "help") == 0) //checks if user has input help command, I provide them some information regarding this shell interpreter
    {
        printf("\n\n\n\n\nThis is a Shell interpreter written in C by Jay Hemalkumar Shah for this assignment of Advanced System Programming"); //simple print statement
        printf("\nSupported functions are : ");//simple print statement
        printf("\n1. cd ");//simple print statement
        printf("\n2. external bash commands");//simple print statement
        printf("\n3. seperating commands with ';'");//simple print statement
        printf("\n4. pipe commands\n\n\n\n\n");//simple print statement
        return 0;//simple print statement
    }
    int pid; //store process id
    int status; //store status
    if ((pid = fork()) == -1) //fork here
    {
        printf("\nCouldnt fork"); //if cant fork then print this here
    }

    if (pid == 0) // if fork sucessfull and pid is 0 then we are in child process
    {
        if (execvp(arguments[0], arguments) == -1) //execute the process using execvp, arguments array's first element has the name of the process to be executed
        {
            printf("Cant handle this command"); //if execvp fails then print this
        }
        exit(0); //exit here
    }
    else //otherwise we are in parent process
    {
        waitpid(pid, &status, 0); //wait for the child to finish
    }
}

int executePipedProcess(char *processes[], char *arguments[], int totalProcesses) //function to execute all the piped processes
{
    int j; //counter for looping

    if (fork() == 0) //fork the process here
    {
        for (j = 0; j < totalProcesses - 1; j++) //loop for j's value from 0 to totalProcesses-1 as totalProcesses is the total processes in the command, not the total pipes
        {
            //this loop creates pipes for all the piped processes and forks and maps the output of child to input for parent for all processes
            int pipeDescriptor[2];//this is the pipe descriptor
            if((pipe(pipeDescriptor)) == -1) //open pipe
            {
                printf("\n Pipe failed");//print this if pipe is not sucessful
                return 0;
            }

            if (fork() == 0)    //fork the process here
            {
                dup2(pipeDescriptor[1], STDOUT_FILENO); // map standard output to pipeDescriptor output and remap out back to parent
                arguments[0] = NULL; //reset the argument array
                extractArguments(processes[j], arguments); //extract argument from the process
                if((execvp(arguments[0], arguments)) == -1) //run the process
                {
                    printf("Cant handle this command"); //if execvp fails then print this
                }         
                abort();//abort the process
            }            
            dup2(pipeDescriptor[0], STDIN_FILENO); // remap output from previous child to input
            close(pipeDescriptor[1]);//close pipeDescriptor
        }
        //loop works till the last second process and the final piped process is handled here
        arguments[0] = NULL; //reset arguments
        extractArguments(processes[j], arguments);//extract arguments
        if((execvp(arguments[0], arguments)) == -1)//run the process
        {
             printf("Cant handle this command"); //if execvp fails then print this
        }
        abort();
    }
    else
    {
        wait(NULL);//main parent wait till all the piped processes are executed
    }
}

int main()
{
    char userInput[255];//string to store user's input
    char *arguments[255];//array to store arguments of a command
    char *commands[255];//array to store all the input commands from the user
    char *processes[255];//array to store all the piped processes
    char cwd[255];//string to store current working directory

    printf("\n\n\n\n\n\n\t\t\t***********************************************************\n"); //simple print statement
    printf("\t\t\t** Jay Hemalkumar Shah -- 110070829 -- SHELL Interpreter **\n"); //simple print statement
    printf("\t\t\t***********************************************************\n"); //simple print statement
    printf("\t\t\t                     Welcome %s \n", getenv("USER")); //simple print statement
    printf("\t\t\t***********************************************************\n"); //simple print statement
    printf("\t\t\t*  Type help for details regarding this SHELL Interpreter *\n\n\n\n\n\n"); //simple print statement
    while (1)
    {
        printf("\n%s @ %s >>> ", getenv("USER"), getcwd(cwd,255));//simple print statement which also shows current user and current working directory
        fgets(userInput, 255, stdin); //get user's input from standard input

        int cmdCount = extractCommands(userInput, commands); //get the total number of commands in the user's input and store those commands in commands array
        for (int i = 0; i < cmdCount; i++)//loop through all the commands
        {
            if (strcasestr(commands[i], "exit"))//if the command has exit in it then exit the program
            {
                printf("\nEXITING PROGRAM .....\n");
                exit(0); //exit here
            }
            processes[0] = NULL;//reset the piped processes array for each command
            int totalProcesses = extractPipes(commands[i], processes); //get total number of piped processes, as extractPipes function gives total piped processes not total pipes, there will always be one process
            if (totalProcesses > 1)  //if pipes are present in a command
            {
                executePipedProcess(processes, arguments, totalProcesses); //all the function to execute all the piped processes and pass the arrays processes and arguments and integer totalProcesses as parameters
            }
            else //if there are no pipes
            {
                arguments[0] = NULL; //reset the arguments array
                extractArguments(commands[i], arguments); //extract arguments from the command
                executeProcess(arguments);//pass the arguments array to executeProcess function, arguments[0] has the process name to be executed
            }
        }
    }
    return 0; //return here
}