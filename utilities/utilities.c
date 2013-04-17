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

void freeStrArray(char** strArray, unsigned int numberOfStrings) {
    
    int i;
    
    for ( i = 0 ; i < numberOfStrings; ++i ) {
        free(strArray[i]);
    }
    
    free(strArray);
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
    int number_of_lines = 0;
    
    while ( read ( file, ch, 1 ) > 0 ) {
        if ( ch[0] == '\n' ) {
            number_of_lines++;
        }
    }
    
    if(close(file) != 0) {
        printf("Problem closing file. Error number %d: %s\n", errno, strerror(errno));
        exit(1);
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

int getNumOfBackups(DIR * folderDir){
    struct dirent *direntp;
    struct stat stat_buf;
    int n = 0;
    char * folderName = malloc(MAX_LEN * sizeof(char));
    
    while ( ( direntp = readdir ( folderDir ) ) != NULL ) {
        if ( stat ( direntp->d_name, &stat_buf ) != 0 ) {
            printf ( "Error number %d: %s\n", errno, strerror ( errno ) );
            return -1;
        }
        if ( S_ISDIR ( stat_buf.st_mode )
            // and ignore the "." and ".."
            && strcmp ( direntp->d_name, "." )
            && strcmp ( direntp->d_name, ".." ) ) {
            
            if ( isBackupString(direntp->d_name) )
                n++;
        }
    }
    
    free(folderName);
    rewinddir ( folderDir );
    return n;
}

int isBackupString(char * string){
    if (strlen(string) != DATE_LEN-1)
        return 0;
    
    char * stringcopy = malloc(DATE_LEN * sizeof(char));
    strcpy(stringcopy, string);
    char * currentNumber;
    currentNumber = strtok(stringcopy, "_");

    // check if there is a year
    if ( ! (atoi(currentNumber) >= 1900 && atoi(currentNumber) <= 9999) ){
        free(stringcopy);
        free(currentNumber);
        return 0;
    }
    
    // now check the rest of the numbers
    currentNumber = strtok(NULL, "_");
    while (currentNumber != NULL) {
        if ( !(atoi(currentNumber) >= 0 && atoi(currentNumber) <= 99) ){
            free(stringcopy);
            free(currentNumber);
            return 0;
        }
        currentNumber = strtok(NULL, "_");
    }
    free(stringcopy);
    free(currentNumber);
    return 1;
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
    
    fclose(file);
    
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
        execlp ( "cp", "cp", "-f", fromPath, toPath, NULL);
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
    
    fileName[fileNameSize - 1] = '\0';
    
    return fileName;
}

char* getFileLineFromBckpInfo(const char * bckpInfoPath, const char * fileName) {
    
    int i = 1;
    for(; i <= getNumOfLines(bckpInfoPath); i++) {
        char* fileLine = getLineAt(i, bckpInfoPath);
        
        char* fileFromBckpInfo = extractFileNameFromInfoLine(fileLine);
        if(strcmp(fileName, fileFromBckpInfo) == 0){
            free(fileFromBckpInfo);
            return fileLine;
        }
        free(fileFromBckpInfo);
        free(fileLine);
    }
    
    return NULL;
    
}

int fileExists(const char * filePath) {

    int file;
    file = open(filePath, O_RDONLY);

    if (file == -1)
        return 1;
    else {
        close(file);
        return 0;
    }
    
}

int filesDeleted(const char* dirPath, const char* bckpInfoPath) {
    
    char filePath[MAX_LEN];
    
    int i = 1;
    for(; i <= getNumOfLines(bckpInfoPath); i++) {
        char* fileLine = getLineAt(i, bckpInfoPath);
        char* fileFromBckpInfo = extractFileNameFromInfoLine(fileLine);
        
        sprintf(filePath, "%s/%s", dirPath, fileFromBckpInfo);
        
        free(fileLine);
        free(fileFromBckpInfo);
        if(fileExists(filePath) == 1){
            return 0;
        }
    }
    
    return 1;
    
}

time_t backupDateToTimeStruct(const char * backupDate) {
    time_t now;
    time(&now);
    struct tm date;
    // we must do this to get the daylight savings time parameter
    // or else return values from the same date are inconsistent
    date = *localtime(&now);
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

int cmpBackupDates(const void *date1, const void *date2) {

    const char **date1string = (const char **)date1;
    const char **date2string = (const char **)date2;
    
    const time_t date1time = backupDateToTimeStruct(*date1string);
    const time_t date2time = backupDateToTimeStruct(*date2string);
    
    // date2 - date1
    double diff = difftime(date1time, date2time);
    
    if(diff < 0)
        // date2 < date1
        return 1;
    else if(diff > 0)
        // date2 > date1
        return -1;
    else
        return 0;
}

int getChoice(const char *prompt, int maxChoice) {
    int choice;
    char input[MAX_LEN];
    
    while (fputs(prompt, stdout) != EOF && fgets(input, sizeof(input), stdin) != 0)
    {
        if (sscanf(input, "%d", &choice) == 1){
            if(choice >= 0 && choice <= maxChoice){
                return choice;
            } else {
                printf("Please select an available option\n");
            }
        }
        printf("That's not a valid input!\n");
    }

    printf("EOF problem or error!\n");
    exit(1);
}

char* backupDateToReadableDate(const char* backupDate){
    char * datestr = malloc(MAX_LEN * sizeof(char));
    struct tm * dateinfo;
    
    time_t date = backupDateToTimeStruct(backupDate);
    dateinfo = localtime(&date);
    strftime(datestr, MAX_LEN, "%c", dateinfo);
    return datestr;
}

