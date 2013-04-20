//  utils.h
//
//  Projeto 1 SOPE
//
//  T1G05 Turma 1 Grupo 5
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
#include <stdbool.h>

#define PROMPT "> "
#define DATE_LEN 20
#define MAX_LEN 1024
#define BACKUP_INFO_LEN 33
#define BACKUPINFO "__bckpinfo__"

/**
 * Creates a string array.
 * @param numberOfStrings The number of strings the array will contain.
 * @param sizeOfStrings The size of each string.
 * @return The string array.
 */
char** createStrArray(unsigned int numberOfStrings, unsigned int sizeOfStrings);

/**
 * Free the memory allocated to a string array.
 * @param strArray The string array.
 * @param numberOfStrings The number of strings in the array.
 */
void freeStrArray(char** strArray, unsigned int numberOfStrings);

/**
 * Parses a file/folder path to remove the '\n' at the end.
 * @param path The file/folder path.
 */
void parsePath(char* path);

/**
 * Converts a single character to an unsigned int.
 * @param c The character representing a number.
 * @return The unsigned int the number represents.
 */
unsigned int convertToUnsignedInt(char c);

/**
 * Reads the number of lines in a file.
 * @param filePath The path of the file.
 * @return The number of lines (minus 1).
 */
int getNumOfLines(const char * filePath);

/**
 * Counts the number of directories inside a folder.
 * @param folderDir The folder that will be searched.
 * @return Number of directoried counted.
 */
int getNumOfDirectories(DIR * folderDir);

/**
 * Counts the number of backup folder inside a directory.
 * Backup folder are of the following type: year_month_day_hours_minutes_seconds
 * @param folderDir The folder that will be searched.
 * @return Number of backup folders counted.
 */
int getNumOfBackups(DIR * folderDir);

/**
 * Checks if a given string belongs to a backup name.
 * A backup string is of the following type: year_month_day_hours_minutes_seconds
 * @param string The string that will be analyzed.
 * @return 1 if the string is of a backup name, 0 if not.
 */
int isBackupString(char * string);

/**
 * Returns a string which is the specified line of a file.
 * @param line The specified line
 * @param filePath The path of the file
 * @return String with the line contents.
 */
char* getLineAt(unsigned int line, const char * filePath);

/**
 * Concatenates a restore date with the bckinfo file name.
 * @param restoreDate The restore date.
 * @return The concatenated string.
 */
char* getBackupInfo(const char* restoreDate);

/**
 * Concatenates the backup folder path with a restore point folder.
 * @param path The backup folder.
 * @param backupPath The restore point folder.
 * @return The concatenated string.
 */
char* getBackupFullPath(const char* path, const char* backupPath);

/**
 * Concatenates a file path with it's name.
 * @param path The file path.
 * @param fileName The file name.
 * @return The concatenated string.
 */
char* getFileFullPath(const char* path, const char* fileName);

/**
 * Copies a file from a path to a new path.
 * @param fromPath The path of the file location.
 * @param toPath The path where the copy will be created.
 * @return 0 on success.
 */
int copyFile(const char* fromPath, const char* toPath);

/**
 * Retrieves the path from a line in bckpinfo. This path corresponds to the location where the file is backed up.
 * @param bckpInfoLine The line of the selected file.
 * @return The path to the backup location of the selected file.
 */
char* getBackupPathFromInfoLine(const char* bckpInfoLine);

/**
 * Retrieves a file name from a line in bckpinfo.
 * @param bckpInfoLine The line of the selected file.
 * @return The name of the selected file.
 */
char* getFileNameFromInfoLine(const char* bckpInfoLine);

/**
 * Retrieves a line from the bckpinfo file containing information for the specified file.
 * This line contains the path of the backup location of a file and the file name.
 * @param bckpInfoPath The bckpinfo path.
 * @param fileName The file to search.
 * @return The line with the information about the file.
 */
char* getFileLineFromBckpInfo(const char * bckpInfoPath, const char * fileName);

/**
 * Check if a file exists.
 * @param filePath The file path to check.
 * @return 1 if the file exists, 0 if not.
 */
int fileExists(const char * filePath);

/**
 * Checks if all files in the the specified bckpinfo exist in the given directory.
 * @param dirPath The directory to analyze.
 * @param bckpInfoPath The path to the bckpinfo.
 * @return 1 if a file was deleted, 0 if not.
 */
int filesDeleted(const char* dirPath, const char* bckpInfoPath);

/**
 * Converts a string representing a date to a time_t struct.
 * The string must be of type: year_month_day_hours_minutes_seconds
 * @param backupDate The date string.
 * @return The corresponding time_t struct.
 */
time_t backupDateToTimeStruct(const char * backupDate);

/**
 * Converts a time_t struct to a string representing the corresponding date.
 * The string will be written in this way: year_month_day_hours_minutes_seconds
 * @param time The time_t struct to convert.
 * @return The corresponding date in a string.
 */
char* timeStructToBackupDate(time_t time);

/**
 * Compares two date strings of type: year_month_day_hours_minutes_seconds
 * @param date1 The first date string.
 * @param date2 The second date string.
 * @return 0 if the dates are equal, 1 if date2 is later than date1, 2 if date2 is sooner than date1
 */
int cmpBackupDates(const void *date1, const void *date2);

/**
 * Gets a choice input from the user and returns it.
 * @param prompt The question made to the user.
 * @param maxChoice The maximum valid choice the user can make.
 * @return The user choice.
 */
int getChoice(const char *prompt, int maxChoice);

/**
 * Converts a backup date string to a more friendly readable date for the user to see.
 * @param backupDate The string containing the date.
 * @return A string with a fancier way to display the date.
 */
char* backupDateToReadableDate(const char* backupDate);

#endif
