//  utils.c
//
//  Projeto 1 SOPE
//
//  Turma 1
//
//  ei11089 - Diogo Santos
//  ei11122 - Pedro Fernandes
//

#include "utilities.h"

char** createStrArray(unsigned int numberOfStrings, unsigned int sizeOfStrings) {
    
    char ** strArray = malloc ( numberOfStrings * sizeof ( char* ) );
    
    int i;
    
    for ( i = 0 ; i < numberOfStrings; ++i ) {
        strArray[i] = malloc ( sizeOfStrings * sizeof ( char ) );
    }
    
    return strArray;
    
}

void parsePath(char* path) {
    
    if(path == NULL)
        return;
    
    int i = 0;
    for(; i < strlen(path); i++)
        if(path[i] == '\n')
            path[i] = 0;
}

unsigned int convertToUnsignedInt(char c) {
    return c - '0';
}

int getNumOfLines ( const char * filePath )
{
    int file;
    file = open ( filePath, O_RDONLY );
    if ( file == -1 ) {
        perror ( filePath );
        exit ( 5 );
    }
    unsigned char ch[1];
    int number_of_lines = 1;
    
    while ( read ( file, ch, 1 ) > 0 ) {
        if ( ch[0] == '\n' ) {
            number_of_lines++;
        }
    }
    
    return number_of_lines;
}

int getNumOfDirectories ( DIR * folderDir )
{
    struct dirent *direntp;
    struct stat stat_buf;
    int n = 0;
    
    while ( ( direntp = readdir ( folderDir ) ) != NULL ) {
        if ( stat ( direntp->d_name, &stat_buf ) != 0 ) {
            printf ( "Error number %d: %s\n", errno, strerror ( errno ) );
            return -1;
        }
        if ( S_ISDIR ( stat_buf.st_mode )
            // and ignore the "." and ".."
            && strcmp ( direntp->d_name, "." )
            && strcmp ( direntp->d_name, ".." ) ) {
            n++;
        }
    }
    
    rewinddir ( folderDir );
    return n;
}


char* getLineAt ( unsigned int line, const char * filePath )
{
    
    if ( line <= 0 || line > getNumOfLines ( filePath ) ) {
        printf ( "Tried to access unexistent line at %s\n", filePath );
        exit ( -1 );
    }
    
    FILE * file;
    file = fopen ( filePath, "r" );
    
    if ( file == NULL ) {
        perror ( filePath );
        exit ( 5 );
    }
    
    char * str = malloc( MAX_LEN * sizeof(char) );
    
    int n = 0;
    while ( n != line ) {
        n++;
        fgets (str, MAX_LEN, file);
    }
    
    parsePath(str);
    return str;
}

char* getBackupInfo ( const char* restoreDate ) {
    char * backupInfoPath = malloc(MAX_LEN * sizeof(char) );
    sprintf ( backupInfoPath, "%s/%s", restoreDate, BACKUPINFO );
    return backupInfoPath;
}

char* getBackupFullPath (const char* path, const char* backupPath ) {
    char * backupFullPath = malloc(MAX_LEN * sizeof(char) );
    sprintf ( backupFullPath, "%s/%s", path, backupPath );
    return backupFullPath;
}

char* getFileFullPath (const char* path, const char* fileName) {
    char * backupFullPath = malloc(MAX_LEN * sizeof(char) );
    sprintf ( backupFullPath, "%s/%s", path, fileName );
    return backupFullPath;
}

int copyFile ( const char* fromPath, const char* toPath ) {
    
    pid_t pid;
    pid = fork();
    
    if ( pid == 0 ) {
        execlp ( "cp", "cp", fromPath, toPath, NULL);
        printf ( "Copy failed! \n" );
        return ( 1 );
    }
    
    return(0);
    
}

char* extractBackupPathFromInfoLine(const char* bckpInfoLine) {
    
    if(strlen(bckpInfoLine) < DATE_LEN - 1) {
        return NULL;
    }
    
    char* backupPath = malloc( DATE_LEN * sizeof(char) );
    
    memcpy(backupPath, bckpInfoLine, DATE_LEN-1);
    backupPath[DATE_LEN - 1] = '\0';
    
    return backupPath;
}

char* extractFileNameFromInfoLine(const char* bckpInfoLine) {
    
    if(strlen(bckpInfoLine) <= DATE_LEN - 1) {
        fprintf(stderr, "Invalid filename\n");
        return NULL;
    }
    
    unsigned long fileNameSize = strlen(bckpInfoLine) - (DATE_LEN - 2); //-2 to account for /0
    
    char* fileName = malloc( fileNameSize * sizeof(char) );
    
    int i = DATE_LEN;
    int j = 0;
    
    for( ; j < fileNameSize && bckpInfoLine[i] != '\n'; i++, j++) {
        fileName[j] = bckpInfoLine[i];
    }
    fileName[DATE_LEN - 1] = '\0';
    
    return fileName;
}

char* getFileLineFromBckpInfo(const char * bckpInfoPath, const char * fileName) {
    
    int i = 1;
    for(; i <= getNumOfLines(bckpInfoPath); i++) {
        char* fileLine = getLineAt(i, bckpInfoPath);
        
        char* fileFromBckpInfo = extractFileNameFromInfoLine(fileLine);
        
        if(strcmp(fileName, fileFromBckpInfo) == 0)
            return fileLine;
        
    }
    
    return NULL;
    
}
