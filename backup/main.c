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
#include <wait.h>

#include "../utilities/utilities.h"

// this will wait for all child processes and shutdown

void sigusr1_handler(int signo) {
    int status;
    printf("Waiting for child processes to end\n");
    while (waitpid(-1, &status, 0))
        if (errno == ECHILD)
            break;
    printf("All child processes terminated, exiting...\n");
    exit(0);
}

// this will remove any zombie processes

void sigchld_handler(int signo) {
    int status;
    pid_t pid;

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
        printf("PARENT: child with PID=%d terminated with exit code %d\n", pid, WEXITSTATUS(status));
}

char* timeStructToBackupDate(time_t time) {
    struct tm * date;
    date = localtime(&time);
    char* dateStr = malloc(DATE_LEN * sizeof (char));

    //year_month_day_hours_minutes_seconds
    sprintf(dateStr, "%04d_%02d_%02d_%02d_%02d_%02d",
            date->tm_year + 1900,
            date->tm_mon + 1,
            date->tm_mday,
            date->tm_hour,
            date->tm_min,
            date->tm_sec);

    return dateStr;
}

time_t backupDateToTimeStruct(char * backupDate) {
    struct tm date;
    int year, mon;
    sscanf(backupDate, "%04d_%02d_%02d_%02d_%02d_%02d",
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

// returns the created/latest backup folder path
// for example: "backupPath/2013_04_13_14_56_19"

char* fullBackup(const char* monitoredPath, const char* backupPath, time_t* backupDate) {
    DIR * monitoredDir = opendir(monitoredPath);

    struct dirent *direntp;
    struct stat stat_buf;

    chdir(backupPath);
    *backupDate = time(NULL);
    char* restorePoint = timeStructToBackupDate(*backupDate);
    mkdir(restorePoint, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    chdir(restorePoint);
    int bckpinfo = open(BACKUPINFO, O_WRONLY | O_CREAT | O_APPEND, 0644);
    // return from the backup folder
    chdir("..");
    chdir("..");

    char fromPath[MAX_LEN];
    char toPath[MAX_LEN];
    char fileInfo[MAX_LEN];
    chdir(monitoredPath);
    while ((direntp = readdir(monitoredDir)) != NULL) {
        if (stat(direntp->d_name, &stat_buf) != 0) {
            printf("Error number %d: %s\n", errno, strerror(errno));
            exit(1);
        }
        if (S_ISREG(stat_buf.st_mode)
                // ignore ".hidden" files
                && strncmp(direntp->d_name, ".", 1)
                && strcmp(direntp->d_name, BACKUPINFO)) {
            printf("Backed up %-25s\n", direntp->d_name);
            chdir("..");
            sprintf(fromPath, "%s/%s", monitoredPath, direntp->d_name);
            sprintf(toPath, "%s/%s/%s", backupPath, restorePoint, direntp->d_name);
            copyFile(fromPath, toPath);
            sprintf(fileInfo, "%s/%s\n", restorePoint, direntp->d_name);
            write(bckpinfo, fileInfo, strlen(fileInfo));
            chdir(monitoredPath);
        }
    }

    char * latestRestorePoint = malloc(MAX_LEN * sizeof (char));
    sprintf(latestRestorePoint, "%s/%s", backupPath, restorePoint);
    free(restorePoint);
    return latestRestorePoint;
}

// returns the created/latest backup folder path
// for example: "backupPath/2013_04_13_14_56_19"

void backupModifiedFiles(const char* monitoredPath, const char* backupPath, char* latestRestorePoint, time_t* lastUpdateTime) {
    DIR * monitoredDir = opendir(monitoredPath);

    struct dirent *direntp;
    struct stat stat_buf;

    char fromPath[MAX_LEN];
    char toPath[MAX_LEN];
    char fileInfo[MAX_LEN];
    chdir(monitoredPath);

    char* restorePoint = malloc(DATE_LEN * sizeof (char));
    char* previousBckpInfo = getBackupInfo(latestRestorePoint);
    int bckpinfo;
    time_t thisUpdateTime;

    bool restorePointCreated = false;

    while ((direntp = readdir(monitoredDir)) != NULL) {
        if (stat(direntp->d_name, &stat_buf) != 0) {
            printf("Error number %d: %s\n", errno, strerror(errno));
            exit(1);
        }
        if (S_ISREG(stat_buf.st_mode)
                // ignore ".hidden" files
                && strncmp(direntp->d_name, ".", 1)
                && strcmp(direntp->d_name, BACKUPINFO)) {

            time_t modTime = stat_buf.st_mtim.tv_sec;
            time_t chgTime = stat_buf.st_ctim.tv_sec;

            /* printf("The last update time: %f\n", (double) *lastUpdateTime);
             printf("The last modified time: %f\n", (double) modTime);
             printf("The dif time: %f\n", difftime(modTime, *lastUpdateTime));*/

            if ( (difftime(modTime, *lastUpdateTime) > 0) || (difftime(chgTime, *lastUpdateTime) > 0) ) {

                if (!restorePointCreated) {

                    chdir("..");
                    chdir(backupPath);
                    //*lastUpdateTime = time(NULL);
                    thisUpdateTime = time(NULL);
                   // restorePoint = timeStructToBackupDate(*lastUpdateTime);
                    restorePoint = timeStructToBackupDate(thisUpdateTime);
                    sprintf(latestRestorePoint, "%s/%s", backupPath, restorePoint);
                    mkdir(restorePoint, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
                    chdir(restorePoint);
                    bckpinfo = open(BACKUPINFO, O_WRONLY | O_CREAT | O_APPEND, 0644);
                    // return from the backup folder
                    chdir("..");
                    chdir("..");
                    chdir(monitoredPath);

                    restorePointCreated = true;

                }

                printf("Backed up %-25s\n", direntp->d_name);
                chdir("..");
                sprintf(fromPath, "%s/%s", monitoredPath, direntp->d_name);
                sprintf(toPath, "%s/%s/%s", backupPath, restorePoint, direntp->d_name);
                copyFile(fromPath, toPath);
                sprintf(fileInfo, "%s/%s\n", restorePoint, direntp->d_name);
                write(bckpinfo, fileInfo, strlen(fileInfo));
                chdir(monitoredPath);
            } else {
                if (restorePointCreated) {
                    char* fileLine = getFileLineFromBckpInfo(previousBckpInfo, direntp->d_name);
                    sprintf(fileInfo, "%s\n", fileLine);
                    printf("Writing to bckpinfo: %s\n", fileLine);
                    write(bckpinfo, fileInfo, strlen(fileInfo));
                }
            }
        }
    }

    //char * latestRestorePoint = malloc(MAX_LEN * sizeof (char));
    if(restorePointCreated)
        *lastUpdateTime = thisUpdateTime;
    
    free(restorePoint);
    //return latestRestorePoint;
}

int main(int argc, const char * argv[]) {
    // usage: bckp dir1 dir2 dt &
    if (argc != 4 && argc != 5) {
        fprintf(stderr, "Usage: %s dir_to_monitor dir_backup \n", argv[0]);
        fprintf(stderr, "Usage: %s dir_to_monitor dir_backup & \n", argv[0]);
        exit(1);
    }

    const char * monitoredPath = argv[1];
    const char * backupPath = argv[2];
    DIR *monitoredDir;
    DIR *backupDir;
    int updateInterval;
    bool backgrounded;

    if ((monitoredDir = opendir(monitoredPath)) == NULL) {
        perror(argv[1]);
        exit(2);
    }

    if ((backupDir = opendir(backupPath)) == NULL) {
        // if backup dir doesnt exist, create it
        mkdir(argv[2], S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        if ((backupDir = opendir(argv[2])) == NULL) {
            perror(argv[2]);
            exit(3);
        }
    }

    updateInterval = atoi(argv[3]);

    if (updateInterval <= 0) {
        fprintf(stderr, "Invalid update interval");
        exit(4);
    }

    if (argc == 5 && strcmp(argv[4], "&") == 0) {
        backgrounded = true;
    } else
        backgrounded = false;

    // install sigchild handler
    struct sigaction action;
    action.sa_handler = sigchld_handler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    if (sigaction(SIGCHLD, &action, NULL) < 0) {
        fprintf(stderr, "Unable to install SIGCHLD handler\n");
        exit(6);
    }

    // install sigusr1 handler
    struct sigaction action2;
    action2.sa_handler = sigusr1_handler;
    sigemptyset(&action2.sa_mask);
    action2.sa_flags = 0;
    if (sigaction(SIGUSR1, &action2, NULL) < 0) {
        fprintf(stderr, "Unable to install SIGUSR1 handler\n");
        exit(7);
    }

    // do them backups loop here
    //int counter =20; while( (counter = sleep(counter) )!=0 );
    char * latestRestorePoint;
    time_t* latestRestoreDate;
    latestRestorePoint = fullBackup(monitoredPath, backupPath, latestRestoreDate);
    printf("Latest restore point: %s\n", latestRestorePoint);


    while (true) {

        //Wait for the updateInterval duration
        int counter = updateInterval;
        while ((counter = sleep(counter)) != 0);

        backupModifiedFiles(monitoredPath, backupPath, latestRestorePoint, latestRestoreDate);
        printf("Latest restore point: %s\n", latestRestorePoint);
    }

    /*
    // time tests
    char date[DATE_LEN];
    time_t timer;
    time(&timer);
    sprintf(date, "%s", timeStructToBackupDate(timer) );
    printf("%ld - %s \n%ld \n", timer, date, backupDateToTimeStruct(date) );
     */

    return 0;
}

