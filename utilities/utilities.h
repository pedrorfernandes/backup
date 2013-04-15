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
#include <time.h>

#define PROMPT "> "
#define DATE_LEN 20
#define MAX_LEN 1024
#define BACKUP_INFO_LEN 33
#define BACKUPINFO "__bckpinfo__"

// TODO documentation

char* timeStructToBackupDate(time_t time);

time_t backupDateToTimeStruct(const char * backupDate);

time_t* createBackupTimeArray(char** backupArray, time_t* backupTimeArray, unsigned int numberOfBackups);

char** createStrArray(unsigned int numberOfStrings, unsigned int sizeOfStrings);

void freeStrArray(char** strArray, unsigned int numberOfStrings);

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

int fileExists(const char * filePath);

int filesDeleted(const char* dirPath, const char* bckpInfoPath);

int cmpBackupDates(const void *date1, const void *date2);

int getChoice(const char *prompt, int maxChoice);

int backgroundProcess();

#endif
