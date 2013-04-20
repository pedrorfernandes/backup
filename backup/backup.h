//  backup.h
//
//  Projeto 1 SOPE
//
//  Turma 1
//
//  ei11089 - Diogo Santos
//  ei11122 - Pedro Fernandes
//

#ifndef _backup_h
#define _backup_h

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

/*
 This program is launched through the command bckp dir1 dir2 dt & where
 dir1 is the name of the directory to be monitored;
 dir2 is the directory where are saved the files in dir1 that have been
 modified/created or any other information necessary to restore a previous state
 of dir1;
 dt is the interval between two consecutive scannings of dir1.
 */

char * pwd; /**< The environment variable PWD, indicating the program working directory. */

/**
 * Handles any received USR1 signal by waiting for all child processes to end first, and only then the program will terminate.
 */
void sigusr1_handler(int signo);

/**
 * Will handle all received SIGCHLD signals, avoiding the creation of zombie processes.
 */
void sigchld_handler(int signo);

/**
 * Performs a full backup of the regular files contained inside a directory.
 * @param monitoredPath The path that contains the regular files.
 * @param backupPath The path that is used to create the backup files.
 * @param backupDate Time struct that will be updated with the date of the performed backup.
 * @return A string with the created backup folder path. For example: "backupPath/2013_04_13_14_56_19".
 */
char* fullBackup(const char* monitoredPath, const char* backupPath, time_t* backupDate);

/**
 * Creates a directory named according to the current time. For example: "backupPath/2013_04_13_14_56_19".
 * Also creates the bckpinfo file in the new folder.
 * @param backupPath The path that is used to create the backup files.
 * @param updateTime The time struct that will be updated with the current time.
 * @param restorePoint Pointer to the string that will contain the restore point name. Example: "2013_04_13_14_56_19".
 * @param latestRestorePoint A string that will be updated with the created backup folder path. For example: "backupPath/2013_04_13_14_56_19".
 * @return A file descriptor for the new bckpinfo.
 */
int createRestorePoint(const char* backupPath,
                       time_t * updateTime,
                       char ** restorePoint,
                       char* latestRestorePoint);
/**
 * Checks for any modified, deleted or added files and performs an incremental backup.
 * @param monitoredPath The path that contains the regular files.
 * @param backupPath The path that is used to create the backup files.
 * @param latestRestorePoint A string that contains the latest restore point path name. For example: "backupPath/2013_04_13_14_56_19". If a backup is performed, this string will be updated.
 * @param lastUpdateTime A time struct with the date of the latest restore point. This is used to check if any files were modified beyond this date.
 * @return 1 If a restore point was created, 0 if not.
 */
int backupModifiedFiles(const char* monitoredPath,
                        const char* backupPath,
                        char* latestRestorePoint,
                        time_t* lastUpdateTime);

/**
 * The program main function.
 * @param argc Argument count.
 * @param argv The program arguments.
 * @param envp The environmental parameters.
 * @return 0 if successful.
 */
int main(int argc, const char * argv[], char* envp[]);


#endif
