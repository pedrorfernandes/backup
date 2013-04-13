//  main.c
//
//  Projeto 1 SOPE
//
//  Turma 1
//
//  ei11089 - Diogo Santos
//  ei11122 - Pedro Fernandes
//

/*
 This program is launched through the command bckp dir1 dir2 dt & where
    dir1 is the name of the directory to be monitored;
    dir2 is the directory where are saved the files in dir1 that have been
 modified/created or any other information necessary to restore a previous state
 of dir1;
    dt is the interval between two consecutive scannings of dir1.
 */

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <time.h>

#define DATE_LEN 20

// this will wait for all child processes and shutdown
void sigusr1_handler(int signo){
    int status;    
    printf("Waiting for child processes to end\n");
    while ( waitpid(-1, &status, 0) )
        if (errno == ECHILD)
            break;
    printf("All child processes terminated, exiting...\n");
    exit(0);
}

// this will remove any zombie processes
void sigchld_handler(int signo){
    int status;
    pid_t pid;
    
    while ( (pid=waitpid(-1, &status, WNOHANG)) > 0 )
        printf("PARENT: child with PID=%d terminated with exit code %d\n", pid, WEXITSTATUS(status));
}

char* timeStructToBackupDate(time_t time){
    struct tm * date;
    date = localtime(&time);
    char* dateStr = malloc(DATE_LEN * sizeof(char));
    
    //year_month_day_hours_minutes_seconds
    sprintf(dateStr, "%04d_%02d_%02d_%02d_%02d_%02d",
           date->tm_year+1900,
           date->tm_mon+1,
           date->tm_mday,
           date->tm_hour,
           date->tm_min,
           date->tm_sec);
    
    return dateStr;
}

time_t backupDateToTimeStruct(char * backupdate){
    struct tm date;
    int year, mon;
    sscanf(backupdate, "%04d_%02d_%02d_%02d_%02d_%02d",
           &year,
           &mon,
           &date.tm_mday,
           &date.tm_hour,
           &date.tm_min,
           &date.tm_sec);
    year -= 1900;
    mon -= 1;
    date.tm_year = year;
    date.tm_mon = mon;
    
    return mktime(&date);
}


void fullBackup(char* monitoredPath, char* backupPath){
    
}

int main(int argc, const char * argv[])
{
    // usage: bckp dir1 dir2 dt &
    if ( argc != 4 && argc != 5 ) {
        fprintf(stderr, "Usage: %s dir_to_monitor dir_backup \n", argv[0]);
        fprintf(stderr, "Usage: %s dir_to_monitor dir_backup & \n", argv[0]);
        exit(1);
    }

    DIR *monitoredDir;
    DIR *backupDir;
    int updateInterval;
    bool backgrounded;
    
    if ( (monitoredDir = opendir(argv[1])) == NULL ) {
        perror(argv[1]);
        exit(2);
    }
    
    if ( (backupDir = opendir(argv[2])) == NULL ) {
        // if backup dir doesnt exist, create it
        mkdir(argv[2], S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        if ( (backupDir = opendir(argv[2])) == NULL ) {
            perror(argv[2]);
            exit(3);
        }
    }
    
    updateInterval = atoi(argv[3]);
    
    if (updateInterval <= 0) {
        fprintf(stderr, "Invalid update interval");
        exit(4);
    }
    
    if ( argc == 5 && strcmp(argv[4], "&") == 0){
        backgrounded = true;
    } else
        backgrounded = false;
    
    // install sigchild handler
    struct sigaction action;
    action.sa_handler = sigchld_handler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    if (sigaction(SIGCHLD, &action, NULL) < 0)
    {
        fprintf(stderr,"Unable to install SIGCHLD handler\n");
        exit(6);
    }

    // install sigusr1 handler
    struct sigaction action2;
    action2.sa_handler = sigusr1_handler;
    sigemptyset(&action2.sa_mask);
    action2.sa_flags = 0;
    if (sigaction(SIGUSR1, &action2, NULL) < 0)
    {
        fprintf(stderr,"Unable to install SIGUSR1 handler\n");
        exit(7);
    }
    
    // do them backups loop here
    //int counter =20; while( (counter = sleep(counter) )!=0 );
    
    // time tests
    char date[DATE_LEN];
    time_t timer;
    time(&timer);
    sprintf(date, "%s", timeStructToBackupDate(timer) );
    printf("%ld - %s \n%ld \n", timer, date, backupDateToTimeStruct(date) );
    
    return 0;
}

