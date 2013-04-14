//  utils.h
//
//  Projeto 1 SOPE
//
//  Turma 1
//
//  ei11089 - Diogo Santos
//  ei11122 - Pedro Fernandes
//

#ifndef _utilities_h
#define _utilities_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>

#define PROMPT "> "
#define DATE_LEN 20
#define MAX_LEN 1024
#define BACKUP_INFO_LEN 33
#define BACKUPINFO "__bckpinfo__"

// TODO documentation

char** createStrArray(unsigned int numberOfStrings, unsigned int sizeOfStrings);

void parsePath(char* path);

unsigned int convertToUnsignedInt(char c);

int getNumOfLines(const char * filePath);

int getNumOfDirectories(DIR * folderDir);

char* getLineAt(unsigned int line, const char * filePath);

char* getBackupInfo(const char* restoreDate);

char* getBackupFullPath(const char* path, const char* backupPath);

char* getFileFullPath(const char* path, const char* fileName);

int copyFile(const char* fromPath, const char* toPath);

char* extractBackupPathFromInfoLine(const char* bckpInfoLine);

char* extractFileNameFromInfoLine(const char* bckpInfoLine);

char* getFileLineFromBckpInfo(const char * bckpInfoPath, const char * fileName);


#endif
