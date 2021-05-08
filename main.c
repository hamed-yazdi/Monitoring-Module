#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <limits.h>
#include<dirent.h>
#include <omp.h>
#include<stdbool.h>
#define MAXCHAR 1000
#define MINCHAR 30





char stateSpaceDB[MAXCHAR]=""; //location of state spaces data base
char eventLogs[MAXCHAR]=""; //location of event log file
char initState[MINCHAR]="-"; //initial state id
char input[MINCHAR]=""; //input message
long start_time;
bool inputState=false; //inputState=false --> no input, inputState=true --> have input
char presentStateArray[10][MINCHAR]; //an array for save current states in each step
int presentSAIndex=0; //index of free location for presentStateArray
char timedStateArray[10][5][MINCHAR]; //an array for save waited transitions
                                      //[x][0][x] refer to waiting time, [x][1][x] refer to present state, [x][2][x] refer to next state
int timedSAIndex=0; //index of free location of timedStateArray
char correctMessage[MAXCHAR][MINCHAR];//this is an array in order to save valid messages
int cMIndex=0; //index of free location of correctMessage array
bool switching; //checking system transition. switching=true --> transition happened. switching=false --> transition not happened

void initDir(); //find directory where program is located and run
void createDataBase(); //create data base from statespace file
void listen(); //listen on input port and get message
void presentStates(); //process input message on current system states
void timedStates(); //process on transitions that include waiting parameter and don't trigger by any input messages
void monitor(char message[MINCHAR], char present_state[MINCHAR]); //process transition rows of each current state from database
void logging(char start_time[MINCHAR], char executionTime[MINCHAR], char present_state[MINCHAR], char message[MINCHAR], char next_state[MINCHAR], char variables[MAXCHAR]); //write output in consul and log file
void monitorNextState(char ns_stateFile[MAXCHAR], long start_time, char executionTime[MINCHAR], char present_state[MINCHAR], char message[MINCHAR], char next_state[MINCHAR]); //process variable and transitions of next states from database
void errorPrint(char input[MINCHAR], char error[MAXCHAR]); //write error output when receive invalid input message

void main()
{
    printf("1\n");
    initDir(stateSpaceDB, eventLogs);
    printf("Create DataBase form spacastate file:\n");
    printf("#############################\n");
    createDataBase();
    printf("data base has been created successfully.\n\n");
    strcpy(presentStateArray[presentSAIndex], initState);
    presentSAIndex++;
    #pragma omp parallel num_threads(3)
    {
        #pragma omp single nowait
        {
            listen();
        }
        #pragma omp single nowait
        {
            presentStates();
        }
        #pragma omp single nowait
        {
            timedStates();
        }
    }
}

void initDir()
{
    char cwd[MAXCHAR];
    getcwd(cwd, sizeof(cwd));
    strcpy(stateSpaceDB, cwd);
    #ifdef _WIN32
        strcat(stateSpaceDB,"\\stateSpaceDB\\");
    #else
        strcat(stateSpaceDB,"/stateSpaceDB/");
    #endif // _WIN32
    DIR *d;
    struct dirent *dir;
    d = opendir(stateSpaceDB);
    while ((dir = readdir(d)) != NULL)
    {
        char file[MAXCHAR]="";
        strcpy(file, stateSpaceDB);
        strcat(file, dir->d_name);
        remove(file);
    }
    closedir(d);
    strcpy(eventLogs, cwd);
    #ifdef _WIN32
        strcat(eventLogs,"\\eventLogs\\event.logs");
    #else
        strcat(eventLogs,"/eventLogs/event.logs");
    #endif // _WIN32
    remove(eventLogs);
}

void createDataBase()
{
    char cwd[MAXCHAR];
    getcwd(cwd, sizeof(cwd));
    char stateSpaceDirectory[MAXCHAR]="";
    strcpy(stateSpaceDirectory, cwd);
    #ifdef _WIN32
        strcat(stateSpaceDirectory, "\\stateSpace\\");
    #else
        strcat(stateSpaceDirectory, "/stateSpace/");
    #endif
    char stateSpaceFile[MAXCHAR]="";
    DIR *d;
    struct dirent *dir;
    d = opendir(stateSpaceDirectory);
    if (d)
    {
        int i = 0;
        while ((dir = readdir(d)) != NULL)
        {
            i++;
            if(strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0)
            {
                continue;
            }
            strcpy(stateSpaceFile,stateSpaceDirectory);
            strcat(stateSpaceFile,dir->d_name);
        }
        closedir(d);
    }
    FILE *fstate;
    FILE *fp;
    char str[MAXCHAR];
    fp = fopen(stateSpaceFile, "r");
    if (fp == NULL)
    {
        printf("Could not open file statespace file");
    }
    int initStateIndex=1;
    while (fgets(str, MAXCHAR, fp) != NULL)
    {
        char delim[] = " \"\t<>";
        char *str_ptr = strtok(str, delim);
        if (strcmp(str_ptr, "state") == 0)
        {
                char state_id[MINCHAR]="-";
                str_ptr = strtok(NULL, delim);
                while(str_ptr != NULL)
                    {
                        if (strcmp(str_ptr, "id=") == 0)
                        {
                            strcpy(state_id, strtok(NULL, delim));
                            if (initStateIndex == 1)
                            {
                                strcpy(initState, state_id);
                                initStateIndex--;
                            }
                            break;
                        }
                        str_ptr = strtok(NULL, delim);
                    }
                getcwd(cwd, sizeof(cwd));
                char stateSpaceDB[MAXCHAR]="";
                strcpy(stateSpaceDB, cwd);
                #ifdef _WIN32
                    strcat(stateSpaceDB,"\\stateSpaceDB\\");
                #else
                    strcat(stateSpaceDB,"/stateSpaceDB/");
                #endif
                strcat(stateSpaceDB, state_id);
                fstate = fopen(stateSpaceDB, "a+");
                fprintf(fstate, "variables\t");
        }
       else if (strcmp(str_ptr, "/state") == 0)
       {
           fclose(fstate);

       }
       else if (strcmp(str_ptr, "variable") == 0)
       {
           char variable_name[MAXCHAR]="-";
           char value[MINCHAR]="-";
           while(str_ptr != NULL)
            {
                str_ptr = strtok(NULL, delim);
                if (strcmp(str_ptr, "name=") == 0)
                {
                    strcpy(variable_name, strtok(NULL, delim));
                    continue;
                }
                else if (strcmp(str_ptr, "type=") == 0)
                {
                    str_ptr = strtok(NULL, delim);
                    strcpy(value, strtok(NULL, delim));
                    break;
                }
            }
            strcat(strcat(variable_name, ":"), value);
            fprintf(fstate, "%s, ", variable_name);
       }
       else if (strcmp(str_ptr, "transition") == 0)
       {
           char destination[MINCHAR]="-";
           char executionTime[MINCHAR]="-";
           char title[MINCHAR]="";
           char timeValue[MINCHAR]="-";
           while(str_ptr != NULL)
            {
                str_ptr = strtok(NULL, delim);
                if (strcmp(str_ptr, "source=") == 0)
                {
                    char *state_id="-";
                    state_id = strtok(NULL, delim);
                    getcwd(cwd, sizeof(cwd));
                    char stateSpaceDB[MAXCHAR]="";
                    strcpy(stateSpaceDB, cwd);
                    #ifdef _WIN32
                        strcat(stateSpaceDB,"\\stateSpaceDB\\");
                    #else
                        strcat(stateSpaceDB,"/stateSpaceDB/");
                    #endif // _WIN32
                    strcat(stateSpaceDB, state_id);
                    fstate = fopen(stateSpaceDB, "a+");
                    fprintf(fstate, "\ntransition\t");
                }
                else if (strcmp(str_ptr, "destination=") == 0)
                {
                    strcpy(destination, strtok(NULL, delim));
                }
                else if (strcmp(str_ptr, "executionTime=") == 0)
                {
                    strcpy(executionTime, strtok(NULL, delim));
                }
                else if (strcmp(str_ptr, "title=") == 0)
                {
                    strcpy(title, strtok(NULL, delim));
                    if (strcmp(title, "tau=") == 0)
                    {
                        strcpy(title, strtok(NULL, delim));
                    }
                    int checker=0;
                    for(int i=0; i<cMIndex; i++)
                    {
                        if(strcmp(title, correctMessage[i]) == 0)
                           {
                               checker=1;
                               break;
                           }
                    }
                    if(checker == 0)
                    {
                        strcpy(correctMessage[cMIndex], title);
                        cMIndex++;
                    }
                }
                else if (strcmp(str_ptr, "value=") == 0)
                {
                    strcpy(title, "-");
                    strcpy(timeValue, strtok(NULL, delim));
                }
                else if (strcmp(str_ptr, "/transition") == 0)
                {
                    fprintf(fstate, "title:%s destination:%s executionTime:%s timeValue:%s  ", title, destination, executionTime, timeValue);
                    fclose(fstate);
                    break;
                }
            }

       }
    }
    fclose(fp);
}

void listen()
{
printf("listen!!\n");
    for(int i=0; i<cMIndex; i++)
    {
        printf("%s ", correctMessage[i]);
    }
    printf("\n");
    while(1)
    {
        if(inputState == 0)
        {
            printf("please enter a message ! \n");
            scanf("%s", input);
            int checker=0;
            for(int i=0; i<cMIndex; i++)
            {
                if(strcmp(correctMessage[i], input) == 0)
                    {
                        checker=1;
                        break;
                    }
            }
            if (checker == 0)
            {
                errorPrint(input, "input is not valid");
                continue;
            }
            start_time = time(NULL);
            inputState=1;
        }
    }

}

void presentStates()
{
printf("present state!!\n");

    while(1)
    {
        if (inputState == 1)
        {
            int pStateCounter = presentSAIndex;
            switching=false;
            char currentStatus[10][MAXCHAR];
            for (int i=0; i<pStateCounter; i++)
            {
                strcpy(currentStatus[i], presentStateArray[i]);
            }
            presentSAIndex=0;
            #pragma omp parallel
            {
                #pragma omp for nowait
                for(int i=0; i<pStateCounter; i++)
                {
                    char stateID[MINCHAR];
                    strcpy(stateID, presentStateArray[i]);
                    strcpy(presentStateArray[i], "");
                    monitor(input, stateID);
                }
            }
            if (switching == false)
            {
                for (int i=0; i<pStateCounter; i++)
                {
                    strcpy(presentStateArray[i], currentStatus[i]);
                    presentSAIndex++;
                    errorPrint(input, "system no transition");
                }
            }
            inputState=0;
        }
    }
}

 void monitor(char message[MINCHAR], char present_state[MINCHAR])
{
    char stateFile[MAXCHAR]="";
    snprintf(stateFile, sizeof(stateFile), "%s%s", stateSpaceDB, present_state);
    FILE *fp;
    fp = fopen(stateFile, "r");
    char str[MAXCHAR];
    char next_state[MINCHAR]="-";
    char executionTime[MINCHAR]="-";
    char title[MINCHAR]="";
    char timeValue[MINCHAR]="-";
    while (fgets(str, MAXCHAR, fp) != NULL)
    {
        char delim[] = " \t\:";
        char *str_ptr = strtok(str, delim);
        if ((strcmp(str_ptr, "transition") == 0))
        {
            str_ptr = strtok(NULL, delim);
            str_ptr = strtok(NULL, delim);
            if (strcmp(str_ptr, message) == 0)
            {
                switching=true;
                char ns_stateFile[MAXCHAR]="";
                strcpy(title, message);
                str_ptr = strtok(NULL, delim);
                strcpy(next_state, strtok(NULL, delim));
                str_ptr = strtok(NULL, delim);
                strcpy(executionTime, strtok(NULL, delim));
                snprintf(ns_stateFile, sizeof(ns_stateFile), "%s%s", stateSpaceDB, next_state);
                monitorNextState(ns_stateFile, start_time, executionTime, present_state, message, next_state);
            }
        }
    }
    fclose(fp);
}

void monitorNextState(char ns_stateFile[MAXCHAR], long start_time, char executionTime[MINCHAR], char present_state[MINCHAR], char message[MINCHAR], char next_state[MINCHAR])
{
    char ns_str[MAXCHAR];
    char variables[MAXCHAR]="-";
    FILE *ns_fp;
    ns_fp = fopen(ns_stateFile, "r");
    while (fgets(ns_str, MAXCHAR, ns_fp) != NULL)
    {
        char delim[] = "\t";
        char *ns_str_ptr = strtok(ns_str, delim);
        if ((strcmp(ns_str_ptr, "variables") == 0))
        {
            strcpy(variables, strtok(NULL, delim));
            int size = strlen(variables);
            variables[size - 3] = '\0';
            logging(start_time, executionTime, present_state, message, next_state, variables);
            strcpy(presentStateArray[presentSAIndex], next_state);
            presentSAIndex++;
            continue;
        }
        else if ((strcmp(ns_str_ptr, "transition") == 0))
        {
            strcpy(ns_str_ptr, strtok(NULL, delim));
            char next_state1[MINCHAR]="-";
            char executionTime1[MINCHAR]="-";
            char title1[MINCHAR]="";
            char delim1[] = " \:\t";
            char *ns_str_ptr1 = strtok(ns_str_ptr, delim1);
            while (ns_str_ptr1 != NULL)
            {
                if (strcmp(ns_str_ptr1, "title") == 0)
                {
                    strcpy(title1, strtok(NULL, delim1));
                }
                else if (strcmp(ns_str_ptr1, "destination") == 0)
                {
                    strcpy(next_state1, strtok(NULL, delim1));
                }
                else if (strcmp(ns_str_ptr1, "executionTime") == 0)
                {
                    strcpy(executionTime1, strtok(NULL, delim1));
                }
                else if (strcmp(ns_str_ptr1, "timeValue") == 0)
                {
                    strcpy(ns_str_ptr1, strtok(NULL, delim1));
                    if (strcmp(ns_str_ptr1, "-") != 0)
                    {

                        strcpy(timedStateArray[timedSAIndex][0],ns_str_ptr1);
                        strcpy(timedStateArray[timedSAIndex][1],next_state);
                        strcpy(timedStateArray[timedSAIndex][2],next_state1);
                        strcpy(timedStateArray[timedSAIndex][3],title1);
                        strcpy(timedStateArray[timedSAIndex][4],executionTime);
                        timedSAIndex++;
                    }
                    break;
                }
                strcpy(ns_str_ptr1, strtok(NULL, delim1));
            }
        }
    }
    fclose(ns_fp);
}

void timedStates()
{
printf("timedstate!!\n");

    while(1)
    {
        if(timedSAIndex > 0)
        {
            timedSAIndex--;
            int timeValue = atoi(timedStateArray[timedSAIndex][0]);
            char present_state[MINCHAR];
            strcpy(present_state, timedStateArray[timedSAIndex][1]);
            char next_state[MINCHAR];
            strcpy(next_state, timedStateArray[timedSAIndex][2]);
            char message[MINCHAR];
            strcpy(message, timedStateArray[timedSAIndex][3]);
            char executionTime[MINCHAR];
            strcpy(executionTime, timedStateArray[timedSAIndex][4]);
            sleep(timeValue);
            char stateFile[MAXCHAR];
            long current_time = time(NULL);
            snprintf(stateFile, sizeof(stateFile), "%s%s", stateSpaceDB, next_state);
            monitorNextState(stateFile, current_time, executionTime, present_state, message, next_state);
        }
    }
}

void errorPrint(char input[MINCHAR], char error[MAXCHAR])
{
    printf("input:%s error:%s\n", input, error);
    FILE *ep;
    ep = fopen(eventLogs, "a+");
    fprintf(ep, "input:%s error:%s\n",input, error);
    fclose(ep);
}
void logging(char start_time[MINCHAR], char executionTime[MINCHAR], char present_state[MINCHAR], char message[MINCHAR], char next_state[MINCHAR], char variables[MAXCHAR])
{
    printf("%ld %s %s %s %s %s\n",start_time, executionTime, present_state, message, next_state, variables);
    FILE *ep;
    ep = fopen(eventLogs, "a+");
    fprintf(ep, "%ld %s %s %s %s %s\n",start_time, executionTime, present_state, message, next_state, variables);
    fclose(ep);
}

