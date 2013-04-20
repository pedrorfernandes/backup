//  restore.c
//
//  Projeto 1 SOPE
//
//  T1G05 Turma 1 Grupo 5
//
//  ei11089 - Diogo Santos
//  ei11122 - Pedro Fernandes
//

#include "restore.h"

void printAvailableBackups(char **backupsArray, size_t numberOfBackups)
{
    int i = 0;
    
    char* datestr;
    
    for(; i<numberOfBackups; i++){
        datestr = backupDateToReadableDate(backupsArray[i]);
        printf("%d - %s\n", i+1, datestr);
        free(datestr);
    }
}

char** getAndPrintFolders(DIR * backupDir) {
    
    struct dirent *direntp;
    struct stat stat_buf;
    
    int numberOfBackups = getNumOfBackups(backupDir);
    
    char** backups = createStrArray(numberOfBackups, DATE_LEN);
    
    int n = 0;
    
    while ( ( direntp = readdir(backupDir) ) != NULL ) {
        if ( stat(direntp->d_name, &stat_buf) != 0 ) {
            printf( "Error number %d: %s\n", errno, strerror(errno) );
            freeStrArray(backups, numberOfBackups);
            return NULL;
        }
        if ( S_ISDIR( stat_buf.st_mode )
            // and ignore the "." and ".."
            && strcmp ( direntp->d_name, "." )
            && strcmp ( direntp->d_name, ".." )
            && isBackupString(direntp->d_name) ) {
            sprintf (backups[n], "%s", direntp->d_name );
            n++;
        }
    }
    
    rewinddir(backupDir);
    
    qsort(backups, numberOfBackups, sizeof(char *), cmpBackupDates);
    printAvailableBackups(backups, numberOfBackups);
    return backups;
}

int printBackupInfo(const char* bckpInfoPath) {
    
    int numberOfFiles = getNumOfLines(bckpInfoPath);
    char* infoLine;
    
    int i;
    for(i = 1; i <= numberOfFiles; i++) {
        
        printf("%d - ", i);
        
        if( ( infoLine = getLineAt(i, bckpInfoPath) ) == NULL) {
            printf("Error reading line from %s\n", BACKUPINFO);
            free(infoLine);
            return -1;
        }
        
        char* fileName = getFileNameFromInfoLine(infoLine);
        char* backupDate = getBackupPathFromInfoLine(infoLine);
        char* datestr = backupDateToReadableDate(backupDate);
        
        printf("%s - backed up at %s\n", fileName, datestr);
        
        free(infoLine);
        free(fileName);
        free(backupDate);
        free(datestr);
    }
    
    printf("%d - Restore the whole folder\n", i);
    
    return 0;
}


int main (int argc, const char * argv[], char* envp[])
{
    
    // usage: rstr dir2 dir3
    if ( argc != 3 ) {
        fprintf ( stderr, "Usage: %s dir_backup dir_restore\n", argv[0] );
        exit(1);
    }
    
    char * pwd = getenv("PWD");
    DIR *backupDir;
    DIR *restoreDir;
    
    if ( ( backupDir = opendir(argv[1]) ) == NULL ) {
        free(backupDir);
        perror(argv[1]);
        exit(2);
    }
    
    if ( ( restoreDir = opendir(argv[2]) ) == NULL ) {
        // if restore dir doesnt exist, create it
        mkdir(argv[2], S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        if ( ( restoreDir = opendir(argv[2]) ) == NULL ) {
            perror(argv[2]);
            closedir(backupDir);
            free(backupDir);
            free(restoreDir);
            exit(3);
        }
    }
    
    printf("The following restore points are available:\n");
    // moving into backupDir will allow directory reading
    chdir(argv[1]);
    
    char** backups = getAndPrintFolders(backupDir);
    int numberOfBackups = getNumOfBackups(backupDir);
    
    char prompt[MAX_LEN];
    sprintf(prompt,"Which restore point? (0 to cancel)\n%s", PROMPT);
    int lineNumber = getChoice(prompt, numberOfBackups);
    //User sees numbers from 1 to numberOfBackups, but we want it from 0 to numberOfBackups-1 due to array access
    if(lineNumber == 0) {
        printf("Restore program ended.\n");
        freeStrArray(backups, numberOfBackups);
        return 0;
    }
    
    lineNumber = lineNumber-1;
    
    char* selectedBckpPath = getBackupFullPath(argv[1], backups[lineNumber]);
    char* fullBckpInfoPath = getBackupInfo(selectedBckpPath);
    
    // moving out of the backupDir
    chdir(pwd);
    
    DIR *selectedBackup;
    if ( ( selectedBackup = opendir(selectedBckpPath) ) == NULL ) {
        freeStrArray(backups, numberOfBackups);
        perror(selectedBckpPath);
        exit(2);
    }
    
    char * datestr = backupDateToReadableDate(backups[lineNumber]);
    printf("\n%s restore point chosen.\n", datestr);
    free(datestr);
    freeStrArray(backups, numberOfBackups);
    printf("This backup contains the following files:\n\n");
    printBackupInfo(fullBckpInfoPath);
    
    int numberOfFiles = getNumOfLines(fullBckpInfoPath);
    sprintf(prompt,"\nSelect a file to restore (0 to cancel):\n%s", PROMPT);
    lineNumber = getChoice(prompt, numberOfFiles  + 1);
    
    printf("\n");
    if ( lineNumber == numberOfFiles + 1 ) {
        
        printf("Doing full restore!\n");
        char* fileRestorePath;
        char* destFilePath;
        char* originFilePath;
        
        int i;
        for (i = 1 ; i <= getNumOfLines(fullBckpInfoPath); i++ ) {
            
            fileRestorePath = getLineAt(i, fullBckpInfoPath);
            char* fileName = getFileNameFromInfoLine(fileRestorePath);
            destFilePath = getFileFullPath(argv[2], fileName);
            originFilePath = getFileFullPath(argv[1], fileRestorePath);
            
            if(copyFile(originFilePath, destFilePath) == 0)
                printf("Restored %s successfully!\n", fileName);
            else
                printf("Error copying %s!\n", fileName);
            
            free(fileRestorePath);
            free(fileName);
            free(destFilePath);
            free(originFilePath);
        }
        
    } else if ( lineNumber > 0 && lineNumber <= numberOfFiles ) {
        char* fileRestorePath = getLineAt(lineNumber, fullBckpInfoPath);
        char* fileName = getFileNameFromInfoLine(fileRestorePath);
        char* destFilePath = getFileFullPath( argv[2], fileName );
        char* originFilePath = getFileFullPath( argv[1], fileRestorePath );
        if(copyFile(originFilePath, destFilePath) == 0)
            printf("Restored %s successfully!\n", fileName);
        else
            printf("Error copying %s!\n", fileName);

        free(fileRestorePath);
        free(fileName);
        free(destFilePath);
        free(originFilePath);
    }
        
    // clean up the strings
    free(selectedBckpPath);
    free(fullBckpInfoPath);    
    
    closedir(backupDir);
    closedir(restoreDir);
    closedir(selectedBackup);
    
    printf("\nRestore program ended.\n");
    return 0;
}

