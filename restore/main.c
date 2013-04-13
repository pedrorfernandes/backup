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
#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define PROMPT "> "
#define DATE_LEN 20
#define MAX_LEN 1024
#define BACKUP_INFO_LEN 33
#define BACKUPINFO "__bckpinfo__"

#include "../utilities/utilities.h"

char** getAndPrintFolders ( DIR * backupDir )
{
    //TODO Order folders?
  
    struct dirent *direntp;
    struct stat stat_buf;

    int numberOfBackups = getNumOfDirectories ( backupDir );
    
    char** backups = createStrArray(numberOfBackups, DATE_LEN);

    int n = 0;

    while ( ( direntp = readdir ( backupDir ) ) != NULL ) {
        if ( stat ( direntp->d_name, &stat_buf ) != 0 ) {
            printf ( "Error number %d: %s\n", errno, strerror ( errno ) );
            return NULL;
        }
        if ( S_ISDIR ( stat_buf.st_mode )
                // and ignore the "." and ".."
                && strcmp ( direntp->d_name, "." )
                && strcmp ( direntp->d_name, ".." ) ) {
            sprintf (backups[n], "%s", direntp->d_name );
            printf ( "%d- %-25s\n", n, direntp->d_name );
            n++;
        }
    }

    rewinddir ( backupDir );
    return backups;
}

int printFiles (DIR * backupDir) {
    //This only prints the files in the backup folder, use showBackupInfo to print the bckpInfo file
  
    struct dirent *direntp;
    struct stat stat_buf;

    int n = 1;

    while ( ( direntp = readdir ( backupDir ) ) != NULL ) {
        if ( stat ( direntp->d_name, &stat_buf ) != 0 ) {
            printf ( "Error number %d: %s\n", errno, strerror ( errno ) );
            return -1;
        }
        if ( S_ISREG ( stat_buf.st_mode )
                // and ignore the "." and ".."
                && strcmp ( direntp->d_name, "." )
                && strcmp ( direntp->d_name, ".." )
                && strcmp ( direntp->d_name, BACKUPINFO ) ) {
            printf ( "%d- %-25s\n", n, direntp->d_name );
            n++;
        }
    }

    rewinddir ( backupDir );
    return 0;
}

int printBackupInfo(const char* bckpInfoPath) {
  
  int numberOfFiles = getNumOfLines(bckpInfoPath);
  char* infoLine;
  
  int i = 1;
  for( ; i < numberOfFiles+1; i++) {
    
    printf("%d- ", i);
    
    if( ( infoLine = getLineAt(i, bckpInfoPath) ) == NULL) {
      printf("Error reading line from __bckpinfo__\n");
      return -1;
    }
    
    
    printf("%s - backed up at %s\n", extractFileNameFromInfoLine(infoLine), extractBackupPathFromInfoLine(infoLine));
    
  }
  
  printf("%d- Restore the whole folder\n", i);
  
  return 0;
}    
  

int main ( int argc, const char * argv[] )
{

    // usage: rstr dir2 dir3
    if ( argc != 3 ) {
        fprintf ( stderr, "Usage: %s dir_backup dir_restore\n", argv[0] );
        exit ( 1 );
    }

    DIR *backupDir;
    DIR *restoreDir;

    if ( ( backupDir = opendir ( argv[1] ) ) == NULL ) {
        perror ( argv[1] );
        exit ( 2 );
    }

    if ( ( restoreDir = opendir ( argv[2] ) ) == NULL ) {
        // if restore dir doesnt exist, create it
        mkdir ( argv[2], S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH );
        if ( ( restoreDir = opendir ( argv[2] ) ) == NULL ) {
            perror ( argv[2] );
            exit ( 3 );
        }
    }

    printf ( "The following restore points are available:\n" );
    printf ( "(year_month_day_hours_minutes_seconds)\n" );
    // moving into backupDir will allow directory reading
    chdir ( argv[1] ); 

    char** backups = getAndPrintFolders ( backupDir );

    printf ( "Which restore point?\n%s", PROMPT );

    // TODO input verification
    char lineSelection = getchar();
    int lineNumber = convertToUnsignedInt(lineSelection);
    
    char* selectedBckpPath = getBackupFullPath(argv[1], backups[lineNumber]);
    char* fullBckpInfoPath = getBackupInfo(selectedBckpPath);
    
    // moving out of the backupDir
    chdir("..");
    
    DIR *selectedBackup;

    if ( ( selectedBackup = opendir ( selectedBckpPath ) ) == NULL ) {
        perror ( selectedBckpPath );
        exit ( 2 );
    }
    
    printf("\n%s chosen.\n", backups[lineNumber]);
    printf ( "This backup contains the following files:\n\n" );
    //printFiles(selectedBackup);
    printBackupInfo(fullBckpInfoPath);
    
    printf("\nSelect a file to restore (0 to cancel):\n%s", PROMPT);
    
    // TODO It's reading an extra \n from somewhere, apparently. This temporarily solves it
    getchar();
    
    lineSelection = getchar();
    lineNumber = convertToUnsignedInt(lineSelection);
    
    //TODO Cancel on 0 or invalid number
    if ( lineNumber == getNumOfLines ( fullBckpInfoPath ) +1 ) {

        printf ( "Doing full restore!\n" );
        char* fileRestorePath;
        char* destFilePath;
        char* originFilePath;
        
        int i = 1;
        for ( ; i <= getNumOfLines ( fullBckpInfoPath ); i++ ) {
            //TODO fix this
            fileRestorePath = getLineAt ( i, fullBckpInfoPath );
            destFilePath = getFileFullPath ( argv[2], extractFileNameFromInfoLine ( fileRestorePath ) );
            originFilePath = getFileFullPath ( argv[1], fileRestorePath );
            if(copyFile ( originFilePath, destFilePath ) == 0)
	      printf("\nRestored %s successfully!\n", extractFileNameFromInfoLine ( fileRestorePath ));
	    else
	      printf("\nError copying %s!\n", extractFileNameFromInfoLine ( fileRestorePath ));
        }
        
        free(fileRestorePath);
        free(destFilePath);
        free(originFilePath);

    } else if ( lineNumber != 0 ) {
        char* destFilePath = getFileFullPath ( argv[2], extractFileNameFromInfoLine ( getLineAt ( lineNumber, fullBckpInfoPath ) ) );
        char* originFilePath = getFileFullPath ( argv[1], getLineAt ( lineNumber, fullBckpInfoPath ) );
        copyFile ( originFilePath, destFilePath );

        printf ( "\n%s restored!\n", getLineAt ( lineNumber, fullBckpInfoPath ) );
        
        free(destFilePath);
        free(originFilePath);
    }
    
    // TODO the char** backups generating function must return a size so we can free the memory
    /*
    for ( i = 0 ; i < numberOfStrings; ++i ) {
        free(strArray[i]);
    }
     */
    
    // clean up the strings
    free(selectedBckpPath);
    free(fullBckpInfoPath);    
    
    printf("\nRestore finished!\n");
    return 0;
}

