//  backup.c
//
//  Projeto 1 SOPE
//
//  T1G05 Turma 1 Grupo 5
//
//  ei11089 - Diogo Santos
//  ei11122 - Pedro Fernandes
//

#include "backup.h"

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
    DIR * monitoredDir;;
    
    if ((monitoredDir = opendir(monitoredPath)) == NULL) {
        perror(monitoredPath);
        exit(2);
    }
    
    struct dirent *direntp;
    struct stat stat_buf;
    
    if(chdir(backupPath) != 0) {
        perror("Problem changing into the backup directory");
        exit(8);
    }
    
    time(backupDate);
    
    char* restorePoint = timeStructToBackupDate(*backupDate);
    if(mkdir(restorePoint, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0) {
        perror("Problem creating the restore directory");
        exit(5);
    }
    if(chdir(restorePoint) != 0) {
        perror("Problem changing into the restore directory");
        exit(8);
    }
    int bckpinfo = open(BACKUPINFO, O_WRONLY | O_CREAT | O_APPEND, 0644);
    // return from the backup folder
    if(chdir(pwd) != 0) {
        perror("Problem changing back into the backup directory");
        exit(8);
    }
    
    char fromPath[MAX_LEN];
    char toPath[MAX_LEN];
    char fileInfo[MAX_LEN];
    if(chdir(monitoredPath) != 0) {
        perror("Problem changing back into the monitored path");
        exit(8);
    }
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
            if(chdir(pwd) != 0) {
                perror(pwd);
                exit(8);
            }
            sprintf(fromPath, "%s/%s", monitoredPath, direntp->d_name);
            sprintf(toPath, "%s/%s/%s", backupPath, restorePoint, direntp->d_name);
            if(copyFile(fromPath, toPath) != 0)
                printf("Problem copying %s\n", fromPath);
            sprintf(fileInfo, "%s/%s\n", restorePoint, direntp->d_name);
            if(write(bckpinfo, fileInfo, strlen(fileInfo)) == -1) {
                perror("Problem writing to bckpinfo");
                exit(9);
            }
            if (chdir(monitoredPath) != 0) {
                perror("Problem changing back into the monitored path");
                exit(8);
            }
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
    if (chdir(pwd) != 0) {
        perror(pwd);
        exit(8);
    }
    char * latestRestorePoint = malloc(MAX_LEN * sizeof (char));
    sprintf(latestRestorePoint, "%s/%s", backupPath, restorePoint);
    free(restorePoint);
    return latestRestorePoint;
}

int createRestorePoint(const char* backupPath, time_t * updateTime, char ** restorePoint, char* latestRestorePoint){
    if(chdir(backupPath) != 0) {
        perror("Problem changing into the backup directory");
        exit(8);
    }
    time(updateTime);
    *restorePoint = timeStructToBackupDate(*updateTime);
    sprintf(latestRestorePoint, "%s/%s", backupPath, *restorePoint);
    if(mkdir(*restorePoint, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0) {
        perror("Problem creating restore point");
        exit(5);
    }
    if(chdir(*restorePoint) != 0) {
        perror("Problem changing into the restore directory");
        exit(8);
    }
    int bckpinfo = open(BACKUPINFO, O_WRONLY | O_CREAT | O_APPEND, 0644);
    // return from the backup folder
    if (chdir(pwd) != 0) {
        perror(pwd);
        exit(8);
    }
    return bckpinfo;
}

int backupModifiedFiles(const char* monitoredPath, const char* backupPath, char* latestRestorePoint, time_t* lastUpdateTime) {
    DIR * monitoredDir;
    
    if ((monitoredDir = opendir(monitoredPath)) == NULL) {
        perror(monitoredPath);
        exit(2);
    }
    
    struct dirent *direntp;
    struct stat stat_buf;
    
    char fromPath[MAX_LEN];
    char toPath[MAX_LEN];
    char fileInfo[MAX_LEN];
    
    char* restorePoint = NULL;
    char* previousBckpInfo = getBackupInfo(latestRestorePoint);
    int bckpinfo;
    time_t thisUpdateTime;
    
    bool restorePointCreated = false;
    
    if ( filesDeleted(monitoredPath, previousBckpInfo) ) {
        bckpinfo = createRestorePoint(backupPath, &thisUpdateTime, &restorePoint, latestRestorePoint);
        restorePointCreated = true;
        printf("Detected file deletion: new restore point created\n");
    }
    
    if(chdir(monitoredPath) != 0) {
        perror("Problem changing back into the monitored path");
        exit(8);
    }
    while ((direntp = readdir(monitoredDir)) != NULL) {
        if (stat(direntp->d_name, &stat_buf) != 0) {
            printf("Error number %d: %s\n", errno, strerror(errno));
            exit(1);
        }
        if (S_ISREG(stat_buf.st_mode)
            // ignore ".hidden" files
            && strncmp(direntp->d_name, ".", 1)
            && strcmp(direntp->d_name, BACKUPINFO)) {
            
            // the BSD kernel has a stat struct with different variable names
#ifdef linux    
            // for the linux kernel
            time_t modTime = stat_buf.st_mtim.tv_sec;
            time_t chgTime = stat_buf.st_ctim.tv_sec;
#else       // for the BSD kernel
            time_t modTime = stat_buf.st_mtimespec.tv_sec;
            time_t chgTime = stat_buf.st_ctimespec.tv_sec;
#endif
            bool fileModified = ( (difftime(modTime, *lastUpdateTime) > 0)
                                 || (difftime(chgTime, *lastUpdateTime) > 0) );

            if (fileModified && !restorePointCreated) {
                if (chdir(pwd) != 0) {
                    perror(pwd);
                    exit(8);
                }
                bckpinfo = createRestorePoint(backupPath, &thisUpdateTime, &restorePoint, latestRestorePoint);
                if (chdir(monitoredPath) != 0) {
                    perror("Problem changing back into the monitored path");
                    exit(8);
                }
                restorePointCreated = true;
                // if the restore point was created, we must check all files again
                // to update the newly created bckpinfo
                rewinddir(monitoredDir);
            } else if ( fileModified && restorePointCreated) {
                if (chdir(pwd) != 0) {
                    perror(pwd);
                    exit(8);
                }
                
                printf("Backed up modified file: %-25s\n", direntp->d_name);
                sprintf(fromPath, "%s/%s", monitoredPath, direntp->d_name);
                sprintf(toPath, "%s/%s/%s", backupPath, restorePoint, direntp->d_name);
                copyFile(fromPath, toPath);
                sprintf(fileInfo, "%s/%s\n", restorePoint, direntp->d_name);
                
                if (write(bckpinfo, fileInfo, strlen(fileInfo)) == -1) {
                    perror("Problem writing to bckpinfo");
                    exit(9);
                }
                
                if (chdir(monitoredPath) != 0) {
                    perror("Problem changing back into the monitored path");
                    exit(8);
                }
                
            } else {
                if (restorePointCreated) {
                    if (chdir(pwd) != 0) {
                        perror(pwd);
                        exit(8);
                    }
                    char* fileLine = getFileLineFromBckpInfo(previousBckpInfo, direntp->d_name);
                    if(fileLine != NULL){
                        sprintf(fileInfo, "%s\n", fileLine);
                        // write to bckpinfo the information about this file
                        if (write(bckpinfo, fileInfo, strlen(fileInfo)) == -1) {
                            perror("Problem writing to bckpinfo");
                            exit(9);
                        }
                    } else {
                        // a file that is not in the latest bckpinfo must be backed up again
                        printf("Backed up a new file: %-25s\n", direntp->d_name);
                        sprintf(fromPath, "%s/%s", monitoredPath, direntp->d_name);
                        sprintf(toPath, "%s/%s/%s", backupPath, restorePoint, direntp->d_name);
                        if(copyFile(fromPath, toPath) != 0)
                            printf("Problem copying %s\n", fromPath);
                        sprintf(fileInfo, "%s/%s\n", restorePoint, direntp->d_name);
                        if (write(bckpinfo, fileInfo, strlen(fileInfo)) == -1) {
                            perror("Problem writing to bckpinfo");
                            exit(9);
                        }
                    }
                    free(fileLine);
                    if (chdir(monitoredPath) != 0) {
                        perror("Problem changing back into the monitored path");
                        exit(8);
                    }
                } else {
                    if (chdir(pwd) != 0) {
                        perror(pwd);
                        exit(8);
                    }
                    char* fileLine = getFileLineFromBckpInfo(previousBckpInfo, direntp->d_name);
                    if (fileLine == NULL) {
                        // some file that is not in the latest bckpinfo showed up! We must backup this.
                        bckpinfo = createRestorePoint(backupPath, &thisUpdateTime, &restorePoint, latestRestorePoint);
                        restorePointCreated = true;
                        rewinddir(monitoredDir);
                    }
                    free(fileLine);
                    if (chdir(monitoredPath) != 0) {
                        perror("Problem changing back into the monitored path");
                        exit(8);
                    }
                }
            }
        }
    }
    

    if (chdir(pwd) != 0) {
        perror(pwd);
        exit(8);
    }
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
    // usage: bckp dir1 dir2 dt
    if (argc != 4) {
        fprintf(stderr, "Usage: %s dir_to_monitor dir_backup time_interval \n", argv[0]);
        exit(1);
    }
    
    pwd = getenv("PWD");
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
        if(mkdir(argv[2], S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0) {
            perror("Problem creating the backup directory");
            exit(5);
        }
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
    
    // install sigint handler, is the same as sigusr1
    struct sigaction action3;
    action3.sa_handler = sigusr1_handler;
    sigemptyset(&action3.sa_mask);
    action3.sa_flags = 0;
    if (sigaction(SIGINT, &action3, NULL) < 0) {
        fprintf(stderr, "Unable to install SIGINT handler\n");
        exit(7);
    }
    
    // do a full backup of the monitored directory
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
        
        // check for modified files and backup them
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

