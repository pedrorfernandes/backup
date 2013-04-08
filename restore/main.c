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
#define DATE_LEN 19
#define MAX_LEN 1024
#define BACKUP_INFO_LEN 33
#define BACKUPINFO "__bckpinfo__"

char** createStrArray(unsigned int numberOfStrings, unsigned int sizeOfStrings) {
  
    char ** strArray = malloc ( numberOfStrings * sizeof ( char* ) );
    
    int i;
    
    for ( i = 0 ; i < numberOfStrings; ++i ) {
        strArray[i] = malloc ( sizeOfStrings * sizeof ( char ) );
    }
    
    return strArray;
    
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

    char * str = malloc(MAX_LEN * sizeof(char));

    int n = 0;
    while ( n != line ) {
        n++;
        fgets ( str, MAX_LEN, file );
    }

    return str;
}

char* getBackupInfo ( const char* restoreDate ) {
    char * backupInfoPath = malloc(MAX_LEN * sizeof(char));
    sprintf ( backupInfoPath, "%s/%s", restoreDate, BACKUPINFO );
    return backupInfoPath;
}

char* getBackupFolder (const char* backupPath ) {
  
    char * backupFullPath = malloc(MAX_LEN * sizeof(char));
    sprintf ( backupFullPath, "%s", backupPath );
    return backupFullPath;
}


char** getAndPrintFolders ( DIR * backupDir )
{
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
            printf ( "%d - %-25s\n", n+1, direntp->d_name );
            n++;
        }
    }

    rewinddir ( backupDir );
    return backups;
}

static void call_getcwd(){
    char * cwd;
    cwd = getcwd (0, 0);
    if (! cwd) {
        fprintf (stderr, "getcwd failed: %s\n", strerror (errno));
    } else {
        printf ("%s\n", cwd);
        free (cwd);
    }
}


int printFiles (DIR * backupDir) {
    //TODO this only prints the files in the backup folder, we need it to print the files on the __bckpinfo__ instead!!!
  
    struct dirent *direntp;
    struct stat stat_buf;

    int n = 1;
    
    while ( ( direntp = readdir ( backupDir ) ) != NULL ) {
        if ( stat ( direntp->d_name, &stat_buf ) != 0 ) {
            printf ( "%s: %s\n", direntp->d_name, strerror ( errno ) );
            return -1;
        }
        if ( S_ISREG ( stat_buf.st_mode )
                && strcmp ( direntp->d_name, BACKUPINFO ) ) {
            printf ( "%d- %-25s\n", n, direntp->d_name );
            n++;
        }
    }

    rewinddir ( backupDir );
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
            perror ( argv[1] );
            exit ( 3 );
        }
    }

    printf ( "The following restore points are available:\n" );
    printf ( "(year_month_day_hours_minutes_seconds)\n" );
    
    // this avoids any errors with stat()
    if ( chdir(argv[1]) != 0 ){
        perror(argv[1]);
        exit(7);
    }

    char** backups = getAndPrintFolders ( backupDir );

    printf ( "Which restore point?\n%s", PROMPT );
    
    char lineSelection = getchar();
    int lineNumber = lineSelection - '0';
    lineNumber--;
    
    char* selectedBckpFolder = getBackupFolder(backups[lineNumber]);
    
    //Super testing printf
    //printf("Selected backup path:\n\n%s\n", selectedBckpPath);
    
    DIR *selectedBackup;
    
    if ( ( selectedBackup = opendir ( selectedBckpFolder ) ) == NULL ) {
        perror ( selectedBckpFolder );
        exit ( 2 );
    }
    
    if ( chdir(selectedBckpFolder) != 0 ){
        perror(selectedBckpFolder);
        exit(6);
    }
    
    printf ( "This backup contains the following files:\n" );
    printFiles(selectedBackup);
    
    printf("\nSelect a file to restore (0 to restore all): \n");
    
    //It's reading an extra \n from somewhere, apparently. This temporarily solves it
    getchar();
    
    lineSelection = getchar();
    lineNumber = lineSelection - '0';
    
    //TODO copy that file
    
    printf("\n\n%s restored!", getLineAt(lineNumber, getBackupInfo(selectedBckpFolder)));
    

    return 0;
}

