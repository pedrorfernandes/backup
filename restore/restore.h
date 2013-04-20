//  restore.h
//
//  Projeto 1 SOPE
//
//  T1G05 Turma 1 Grupo 5
//
//  ei11089 - Diogo Santos
//  ei11122 - Pedro Fernandes
//

#ifndef restore_h
#define restore_h

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>

#include "../utilities/utilities.h"

/*
 This program is launched through the command rstr dir2 dir3 where
 dir2 is the directory that was used to backup the files;
 dir3 is the directory where the backed up files are going to be restored.
 */

char * pwd; /**< The environment variable PWD, indicating the program working directory. */

/**
 * Prints all valid backups.
 * @param backupsArray The backup array (strings of the backup dates).
 * @param numberOfBackups The size of the backup array.
 */
void printAvailableBackups(char **backupsArray, size_t numberOfBackups);

/**
 * Scans the backup directory for valid backups, creates a string array to keep track of all the valid restore points detected and prints them.
 * @param backupDir The directory that contains backups.
 * @return A string array with the name of all restore points.
 */
char** getAndPrintFolders(DIR * backupDir);

/**
 * Prints all files that are registered in the bckpinfo file of the specified restore point.
 * @param bckpInfoPath The restore point path. For example: "backupPath/2013_04_13_14_56_19/__bckpinfo__".
 * @return 0 on success, different otherwise.
 */
int printBackupInfo(const char* bckpInfoPath);

/**
 * The program main function.
 * @param argc Argument count.
 * @param argv The program arguments.
 * @param envp The environmental parameters.
 * @return 0 if successful, different if an error occurs.
 */
int main(int argc, const char * argv[], char* envp[]);
    
#endif
