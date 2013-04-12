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

    // install sigusr1 handler
    // do them backups loop here
    
    return 0;
}

