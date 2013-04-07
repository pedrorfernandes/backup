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
 This program is launched through the command rstr dir2 dir3 where
    dir2 is the directory that was used to backup the files;
    dir3 is the directory where the backed up files are going to be restored.
 */

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#define PROMPT "> "

int printContents(DIR * backupDir){
    struct dirent *direntp;
    struct stat stat_buf;

    while ( (direntp = readdir(backupDir)) != NULL)
    {
        if ( stat(direntp->d_name, &stat_buf) != 0 ){
            printf("Error number %d: %s\n", errno, strerror(errno));
            return -1;
        }
        if (S_ISDIR(stat_buf.st_mode)
            && strcmp(direntp->d_name, ".") && strcmp(direntp->d_name, "..") )
            printf("%-25s\n", direntp->d_name);
    }
    
    return 0;
}

int main(int argc, const char * argv[])
{

    // usage: rstr dir1 dir2
    if (argc != 3){
        fprintf( stderr, "Usage: %s dir_with_files dir_backup\n", argv[0]);
        exit(1);
    }
    
    DIR *filesDir;
    DIR *backupDir;

    if ( (filesDir = opendir( argv[1]) ) == NULL) {
        perror(argv[1]);
        exit(2);
    }
    
    if ( (backupDir = opendir( argv[2]) ) == NULL) {
        perror(argv[2]);
        exit(3);
    }
    
    printf("The following restore points are available:\n");
    printf("(year_month_day_hours_minutes_seconds)\n");
    chdir(argv[2]); // this avoids any errors with stat()
    printContents(backupDir);
    printf("Which restore point?\n%s", PROMPT);
    getchar();
    
    return 0;
}

