#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <limits.h>
#include<dirent.h>
#define MAXCHAR 1000

char initState[10]="-";

void createDataBase();

int main()
{
    char cwd[MAXCHAR];
    getcwd(cwd, sizeof(cwd));
    char stateSpaceDB[MAXCHAR]="";
    strcpy(stateSpaceDB, cwd);
    strcat(stateSpaceDB,"\\stateSpaceDB\\");
    DIR *d;
    struct dirent *dir;
    d = opendir(stateSpaceDB);
    while ((dir = readdir(d)) != NULL)
    {
        char file[MAXCHAR]="";
        strcpy(file, stateSpaceDB);
        strcat(file, dir->d_name);
        remove(file);
        free(file);
    }
    closedir(d);
    char eventLogs[MAXCHAR]="";
    strcpy(eventLogs, cwd);
    strcat(eventLogs,"\\eventLogs\\event.logs");
    remove(eventLogs);
    printf("Create DataBase form spacastate file:\n");
    printf("#############################\n");
    createDataBase();
    printf("data base has been created successfully.\n\n");
    char present_state[10]= "-";
    strcpy(present_state, initState);
    char stateFile[MAXCHAR]="";
    while(1)
    {
        printf("please enter a message ! \n");
        char message[100];
        scanf("%s", message);
        clock_t start_time = time(NULL);
        snprintf(stateFile, sizeof(stateFile), "%s%s", stateSpaceDB, present_state);
        FILE *fp;
        fp = fopen(stateFile, "r");
        char str[MAXCHAR];
        char next_state[10]="-";
        char executionTime[10]="-";
        char title[20]="";
        char timeValue[10]="-";
        char variables[MAXCHAR]="-";
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
                    strcpy(title, str_ptr);
                    str_ptr = strtok(NULL, delim);
                    strcpy(next_state, strtok(NULL, delim));
                    str_ptr = strtok(NULL, delim);
                    strcpy(executionTime, strtok(NULL, delim));
                    fclose(fp);
                    break;
                }
                continue;
            }
            free(delim);
            free(str_ptr);
        }
        if(strcmp(title, "") == 0)
        {
            printf("No Match Transition! %s \n", message);
            FILE *ep;
            ep = fopen(eventLogs, "a+");
            fprintf(ep, "No Match Transition! %s\n", message);
            fclose(ep);
            continue;
        }
        snprintf(stateFile, sizeof(stateFile), "%s%s", stateSpaceDB, next_state);
        fp = fopen(stateFile, "r");
        while (fgets(str, MAXCHAR, fp) != NULL)
        {
            char delim[] = "\t";
            char *str_ptr = strtok(str, delim);
            if ((strcmp(str_ptr, "variables") == 0))
            {
                strcpy(variables, strtok(NULL, delim));
                int size = strlen(variables);
                variables[size - 3] = '\0';
                //free(size);
                continue;
            }
            else if ((strcmp(str_ptr, "transition") == 0))
            {
                char next_state1[10]="-";
                char executionTime1[10]="-";
                char title1[20]="";
                char delim1[] = " \:\t";
                while (str_ptr != NULL)
                {
                    str_ptr = strtok(NULL, delim1);
                    if (strcmp(str_ptr, "title") == 0)
                    {
                        strcpy(title1, strtok(NULL, delim1));
                        continue;
                    }
                    else if (strcmp(str_ptr, "destination") == 0)
                    {
                        strcpy(next_state1, strtok(NULL, delim1));
                        continue;
                    }
                    else if (strcmp(str_ptr, "executionTime") == 0)
                    {
                        strcpy(executionTime1, strtok(NULL, delim1));
                        continue;
                    }
                    else if (strcmp(str_ptr, "timeValue") == 0)
                    {
                        str_ptr = strtok(NULL, delim1);
                        if (strcmp(str_ptr, "-") != 0)
                        {
                            int exeTime = atoi(str_ptr);
                            printf("%ld %s %s %s %s %s\n",start_time, executionTime, present_state, message, next_state, variables);
                            FILE *ep;
                            ep = fopen(eventLogs, "a+");
                            fprintf(ep, "%ld %s %s %s %s %s\n",start_time, executionTime, present_state, message, next_state, variables);
                            fclose(ep);
                            sleep(exeTime);
                            start_time = time(NULL);
                            strcpy(present_state, next_state);
                            strcpy(executionTime, executionTime1);
                            strcpy(message, title1);
                            strcpy(next_state, next_state1);
                            fclose(fp);
                            snprintf(stateFile, sizeof(stateFile), "%s%s", stateSpaceDB, next_state);
                            fp = fopen(stateFile, "r");
                            while (fgets(str, MAXCHAR, fp) != NULL)
                            {
                                if ((strcmp(str_ptr, "variables") == 0))
                                {
                                        strcpy(variables, strtok(NULL, delim));
                                        int size = strlen(variables);
                                        variables[size - 3] = '\0';
                                        //free(size);
                                        break;
                                }
                            }
                            free(exeTime);
                        }
                        break;
                    }
                }
                free(next_state1);
                free(executionTime1);
                free(title1);
                free(delim1);
            }
            free(delim);
            free(str_ptr);
        }
        fclose(fp);
        printf("%ld %s %s %s %s %s\n",start_time, executionTime, present_state, message, next_state, variables);
        FILE *ep;
        ep = fopen(eventLogs, "a+");
        fprintf(ep, "%ld %s %s %s %s %s\n",start_time, executionTime, present_state, message, next_state, variables);
        fclose(ep);
        strcpy(present_state, next_state);
        strcpy(next_state, "");
        free(message);
        free(str);
        free(executionTime);
        free(title);
        free(variables);
        free(next_state);
        free(timeValue);
    }
    free(cwd);
    free(stateSpaceDB);
    free(eventLogs);
    free(present_state);
    free(stateFile);
    //return 0;
}

void createDataBase()
{
    char cwd[MAXCHAR];
    getcwd(cwd, sizeof(cwd));
    char stateSpaceDirectory[MAXCHAR]="";
    strcpy(stateSpaceDirectory, cwd);
    strcat(stateSpaceDirectory, "\\stateSpace\\");
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
            if (i == 3)
            {
                strcpy(stateSpaceFile,stateSpaceDirectory);
                strcat(stateSpaceFile,dir->d_name);
            }

        }
        free(i);
        closedir(d);
    }

    FILE *fstate;
    FILE *fp;
    char str[MAXCHAR];
    fp = fopen(stateSpaceFile, "r");
    if (fp == NULL)
    {
        printf("Could not open file statespace file");
        //return 1;
    }
    int initStateIndex=1;
    while (fgets(str, MAXCHAR, fp) != NULL)
    {
        char delim[] = " \"\t<>";
        char *str_ptr = strtok(str, delim);
        if (strcmp(str_ptr, "state") == 0)
        {
                char state_id[10]="-";
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
                strcat(stateSpaceDB,"\\stateSpaceDB\\");
                strcat(stateSpaceDB, state_id);
                fstate = fopen(stateSpaceDB, "a+");
                fprintf(fstate, "variables\t");
                free(state_id);
                free(stateSpaceDB);
        }
       else if (strcmp(str_ptr, "/state") == 0)
       {
           fclose(fstate);

       }
       else if (strcmp(str_ptr, "variable") == 0)
       {
           char variable_name[50]="-";
           char value[10]="-";
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
            free(variable_name);
            free(value);
       }
       else if (strcmp(str_ptr, "transition") == 0)
       {
           char destination[10]="-";
           char executionTime[10]="-";
           char title[20]="";
           char timeValue[10]="-";
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
                    strcat(stateSpaceDB,"\\stateSpaceDB\\");
                    strcat(stateSpaceDB, state_id);
                    fstate = fopen(stateSpaceDB, "a+");
                    fprintf(fstate, "\ntransition\t");
                    free(state_id);
                    free(stateSpaceDB);
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
                        //char t1[10] = "";
                        strcpy(title, strtok(NULL, delim));
                        //printf("%s\n", title);
                        //strcpy(title, strcat(title, ">"));
                        //strcpy(title, strcat(title, t1));
                        //strcpy(title, strtok(NULL, delim));
                        //free(t1);
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
            free(destination);
            free(executionTime);
            free(title);
            free(timeValue);
       }
       free(delim);
    }
    free(cwd);
    free(stateSpaceDirectory);
    free(stateSpaceFile);
    free(str);
    fclose(fp);
}
