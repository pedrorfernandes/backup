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
#include <sys/wait.h>

#include "../utilities/utilities.h"

char * path;

// this will wait for all child processes and shutdown
void sigusr1_handler(int signo) {
    int status;
    printf("Waiting for child processes to end\n");
    while (waitpid(-1, &status, 0))
        if (errno == ECHILD)
            break;
    printf("All child processes are terminated, exiting...\n");
    exit(0);
}

// this will remove any zombie processes
void sigchld_handler(int signo) {
    int status;
    pid_t pid;
    
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0);
}

// returns the created/latest backup folder path
// for example: "backupPath/2013_04_13_14_56_19"
char* fullBackup(const char* monitoredPath, const char* backupPath, time_t* backupDate) {
    DIR * monitoredDir = opendir(monitoredPath);
    
    struct dirent *direntp;
    struct stat stat_buf;
    
    chdir(backupPath);
    time(backupDate);
    char* restorePoint = timeStructToBackupDate(*backupDate);
    mkdir(restorePoint, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    chdir(restorePoint);
    int bckpinfo = open(BACKUPINFO, O_WRONLY | O_CREAT | O_APPEND, 0644);
    // return from the backup folder
    chdir(path);
    
    char fromPath[MAX_LEN];
    char toPath[MAX_LEN];
    char fileInfo[MAX_LEN];
    chdir(monitoredPath);
    while ((direntp = readdir(monitoredDir)) != NULL) {
        if (stat(direntp->d_name, &stat_buf) != 0) {
            printf("Error number %d, %s: %s\n", errno, direntp->d_name, strerror(errno));
            exit(1);
        }
        if (S_ISREG(stat_buf.st_mode)
            // ignore ".hidden" files
            && strncmp(direntp->d_name, ".", 1)
            && strcmp(direntp->d_name, BACKUPINFO)) {
            printf("Backed up a new file: %-25s\n", direntp->d_name);
            chdir(path);
            sprintf(fromPath, "%s/%s", monitoredPath, direntp->d_name);
            sprintf(toPath, "%s/%s/%s", backupPath, restorePoint, direntp->d_name);
            copyFile(fromPath, toPath);
            sprintf(fileInfo, "%s/%s\n", restorePoint, direntp->d_name);
            write(bckpinfo, fileInfo, strlen(fileInfo));
            chdir(monitoredPath);
        }
    }
    
    if(close(bckpinfo) != 0) {
        printf("Problem closing backup info. Error number %d: %s\n", errno, strerror(errno));
        exit(1);
    }
    
    if(closedir(monitoredDir) != 0) {
        printf("Problem closing monitored directory. Error number %d: %s\n", errno, strerror(errno));
        exit(1);
    }
        
    // return from monitoredPath
    chdir(path);
    char * latestRestorePoint = malloc(MAX_LEN * sizeof (char));
    sprintf(latestRestorePoint, "%s/%s", backupPath, restorePoint);
    free(restorePoint);
    return latestRestorePoint;
}

int createRestorePoint(const char* backupPath, time_t * updateTime, char ** restorePoint, char* latestRestorePoint){
    chdir(backupPath);
    time(updateTime);
    *restorePoint = timeStructToBackupDate(*updateTime);
    sprintf(latestRestorePoint, "%s/%s", backupPath, *restorePoint);
    mkdir(*restorePoint, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    chdir(*restorePoint);
    int bckpinfo = open(BACKUPINFO, O_WRONLY | O_CREAT | O_APPEND, 0644);
    // return from the backup folder
    chdir(path);
    return bckpinfo;
}

int backupModifiedFiles(const char* monitoredPath, const char* backupPath, char* latestRestorePoint, time_t* lastUpdateTime) {
    DIR * monitoredDir = opendir(monitoredPath);
    
    struct dirent *direntp;
    struct stat stat_buf;
    
    char fromPath[MAX_LEN];
    char toPath[MAX_LEN];
    char fileInfo[MAX_LEN];
    
    char* restorePoint = NULL;  //= malloc(DATE_LEN * sizeof (char));
    char* previousBckpInfo = getBackupInfo(latestRestorePoint);
    int bckpinfo;
    time_t thisUpdateTime;
    
    bool restorePointCreated = false;
    
    //char cwd[MAX_LEN];
    //getcwd(cwd, sizeof (cwd));
    //printf("%s\n", cwd);
    
    if (filesDeleted(monitoredPath, previousBckpInfo) == 0) {
        bckpinfo = createRestorePoint(backupPath, &thisUpdateTime, &restorePoint, latestRestorePoint);
        restorePointCreated = true;
        printf("Detected file deletion: new restore point created\n");
    }
    
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
            
            // the BSD kernel has a stat with different variable names
#ifdef linux
            time_t modTime = stat_buf.st_mtim.tv_sec;
            time_t chgTime = stat_buf.st_ctim.tv_sec;
#else
            time_t modTime = stat_buf.st_mtimespec.tv_sec;
            time_t chgTime = stat_buf.st_ctimespec.tv_sec;
#endif
            bool fileModified = ( (difftime(modTime, *lastUpdateTime) > 0)
                                 || (difftime(chgTime, *lastUpdateTime) > 0) );
            
            if ( fileModified && !restorePointCreated) {
                chdir(path);
                bckpinfo = createRestorePoint(backupPath, &thisUpdateTime, &restorePoint, latestRestorePoint);
                chdir(monitoredPath);
                restorePointCreated = true;
                // if the restore point was created, we must check all files again
                // to update de bckpinfo
                rewinddir(monitoredDir);
            } else if ( fileModified && restorePointCreated) {
                chdir(path);
                printf("Backed up modified file: %-25s\n", direntp->d_name);
                sprintf(fromPath, "%s/%s", monitoredPath, direntp->d_name);
                sprintf(toPath, "%s/%s/%s", backupPath, restorePoint, direntp->d_name);
                copyFile(fromPath, toPath);
                sprintf(fileInfo, "%s/%s\n", restorePoint, direntp->d_name);
                write(bckpinfo, fileInfo, strlen(fileInfo));
                chdir(monitoredPath);
                
            } else {
                if (restorePointCreated) {
                    chdir(path);
                    char* fileLine = getFileLineFromBckpInfo(previousBckpInfo, direntp->d_name);
                    if(fileLine != NULL){
                        sprintf(fileInfo, "%s\n", fileLine);
                        //printf("Writing to bckpinfo: %s\n", fileLine);
                        write(bckpinfo, fileInfo, strlen(fileInfo));
                    } else {
                        // a file that is not in the latest bckpinfo must be backed up again
                        printf("Backed up a new file: %-25s\n", direntp->d_name);
                        sprintf(fromPath, "%s/%s", monitoredPath, direntp->d_name);
                        sprintf(toPath, "%s/%s/%s", backupPath, restorePoint, direntp->d_name);
                        copyFile(fromPath, toPath);
                        sprintf(fileInfo, "%s/%s\n", restorePoint, direntp->d_name);
                        write(bckpinfo, fileInfo, strlen(fileInfo));
                    }
                    free(fileLine);
                    chdir(monitoredPath);
                } else {
                    chdir(path);
                    char* fileLine = getFileLineFromBckpInfo(previousBckpInfo, direntp->d_name);
                    if (fileLine == NULL) {
                        // some file that is not in the latest bckpinfo showed up!
                        bckpinfo = createRestorePoint(backupPath, &thisUpdateTime, &restorePoint, latestRestorePoint);
                        chdir(monitoredPath);
                        restorePointCreated = true;
                        rewinddir(monitoredDir);
                    }
                    free(fileLine);
                    chdir(monitoredPath);
                }
            }
        }
    }
    

    chdir(path);
    if (restorePointCreated){
        *lastUpdateTime = thisUpdateTime;
        close(bckpinfo);
    }
    
    if(closedir(monitoredDir) != 0) {
        printf("Problem closing monitored directory. Error number %d: %s\n", errno, strerror(errno));
        exit(1);
    }
    
    free(previousBckpInfo);
    free(restorePoint);
    
    if (restorePointCreated)
        return 1;
    else
        return 0;
}

int main(int argc, const char * argv[], char* envp[]) {
    // usage: bckp dir1 dir2 dt &
    if (argc != 4) {
        fprintf(stderr, "Usage: %s dir_to_monitor dir_backup time_interval \n", argv[0]);
        fprintf(stderr, "Usage: %s dir_to_monitor dir_backup time_interval \n", argv[0]);
        exit(1);
    }
    
    path = getenv("PWD");
    const char * monitoredPath = argv[1];
    const char * backupPath = argv[2];
    DIR *monitoredDir;
    DIR *backupDir;
    int updateInterval;
    
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
    char * latestRestorePoint;
    time_t latestRestoreDate;
    time(&latestRestoreDate);
    latestRestorePoint = fullBackup(monitoredPath, backupPath, &latestRestoreDate);
    //printf("Latest restore point: %s\n", latestRestorePoint);
    
    // print out the date of the backup
    char * date = malloc(MAX_LEN * sizeof(char));
    struct tm * dateinfo;
    dateinfo = localtime(&latestRestoreDate);
    strftime(date, MAX_LEN, "Latest backup: %c\n", dateinfo);
    puts(date);
    
    
    while (true) {
        
        //Wait for the updateInterval duration
        int counter = updateInterval;
        while ((counter = sleep(counter)) != 0);
        
        if (backupModifiedFiles(monitoredPath, backupPath, latestRestorePoint, &latestRestoreDate) ){
            //printf("Latest restore point: %s\n", latestRestorePoint);
            dateinfo = localtime(&latestRestoreDate);
            strftime(date, MAX_LEN, "Latest backup: %c\n", dateinfo);
            puts(date);
        }
    }
    
    free(date);
    free(dateinfo);
    
    return 0;
}

