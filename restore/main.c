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

struct BackupInfo {
    char date[DATE_LEN];
    char filename[MAX_LEN];
    char path[MAX_LEN];
};

int getNumOfLines(const char * filePath){
    int file;
    file = open(filePath, O_RDONLY);
    if (file == -1){
        perror(filePath);
        exit(5);
    }
    unsigned char ch[1];
    int number_of_lines = 1;

	while ( read(file, ch, 1) > 0){
        if( ch[0] == '\n') number_of_lines++;
    }
    
    return number_of_lines;
}

int* getBackupInfo(const char * restoreDate){
    DIR *backupFolder;
    if ( (backupFolder = opendir(restoreDate) ) == NULL) {
        perror(restoreDate);
        exit(4);
    }
    
    char backupInfoPath[BACKUP_INFO_LEN];
    sprintf(backupInfoPath, "%s/%s", restoreDate, BACKUPINFO);
    struct BackupInfo files [ getNumOfLines(backupInfoPath) ];
    
    
    
}

int printFolders(DIR * backupDir){
    struct dirent *direntp;
    struct stat stat_buf;

    while ( (direntp = readdir(backupDir)) != NULL)
    {
        if ( stat(direntp->d_name, &stat_buf) != 0 ){
            printf("Error number %d: %s\n", errno, strerror(errno));
            return -1;
        }
        if (S_ISDIR(stat_buf.st_mode)
            // and ignore the "." and ".."
            && strcmp(direntp->d_name, ".")
            && strcmp(direntp->d_name, "..") )
            printf("%-25s\n", direntp->d_name);
    }
    
    return 0;
}

int main(int argc, const char * argv[])
{

    // usage: rstr dir2 dir3
    if (argc != 3){
        fprintf( stderr, "Usage: %s dir_backup dir_restore\n", argv[0]);
        exit(1);
    }
    
    DIR *backupDir;
    DIR *restoreDir;

    if ( (backupDir = opendir( argv[1]) ) == NULL) {
        perror(argv[1]);
        exit(2);
    }
    
    if ( (restoreDir = opendir( argv[2]) ) == NULL) {
        // if restore dir doesnt exist, create it
        mkdir(argv[2], S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        if ( (restoreDir = opendir( argv[2]) ) == NULL) {
            perror(argv[1]);
            exit(3);
        }
    }
    
    printf("The following restore points are available:\n");
    printf("(year_month_day_hours_minutes_seconds)\n");
    chdir(argv[1]); // this avoids any errors with stat()
    printFolders(backupDir);

    printf("Which restore point?\n%s", PROMPT);
    char restoreDate[DATE_LEN];
    scanf("%s", restoreDate);
    
    getBackupInfo(restoreDate);
    
    
    return 0;
}

