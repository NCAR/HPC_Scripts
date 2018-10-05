#include<stdlib.h>
#include<stdio.h>
#include<assert.h>
#include<errno.h>
#include<unistd.h>
#include<string.h>
#include<time.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<sys/types.h>

#pragma once
#ifndef TESTDIR_H
#define TESTDIR_H

///Unsigned int alias
typedef unsigned int uint;
 
///pointer to a testdir structure
typedef struct testdir_struct* testdirptr;

/**
 * Test Directory Struct
 * Linked List 
 * 1 testdir_struct per test directory processed
 * all strings pointed to by members of this struct are owned by this struct
 */
struct testdir_struct
{
    /// directory name
    char *dir;
    /// test file name (randomly generated)
    char *file;

    /// pid of child process
    pid_t pid;
    /// user to assume before testing file
    uid_t uid; 
    /// group to assume before testing file
    gid_t gid;
    /// user name (required for initgroups())
    char* user_name;

    ///value of exit from process
    ///will be -1 if process didn't exit (aka death by signal)

    enum testdir_exit_type { 
        TOUCHDIR_EXIT_NONE = 0, ///exit not found or taken
        TOUCHDIR_EXIT_SIG_KILL, ///killed by signal
        TOUCHDIR_EXIT_SUCCESS, ///successfully exited
        TOUCHDIR_EXIT_FAIL_FILENAME, ///Invalid or impossible filename
        TOUCHDIR_EXIT_FAIL_OPEN, ///Failed to Open file
        TOUCHDIR_EXIT_FAIL_WRITE, ///Failed to write to file
        TOUCHDIR_EXIT_FAIL_UNLINK, ///Failed to unlink file
        TOUCHDIR_EXIT_FAIL_CLOSE, ///Failed to close file
        TOUCHDIR_EXIT_FAIL_SETUID, ///Failed to change to given user id
        TOUCHDIR_EXIT_FAIL_CHDIR ///Failed to change pwd to /    
    } exit;

    /// timestamp of when fork was called
    time_t start;
    /// timestamp of when child exited
    time_t stop;

    /// next testdir in list (may be null)
    testdirptr next;
};

/**
 * Read Directory List
 * @param source Souce file containing new line delimited directory names
 * @return NULL on error or ptr to first entry in directory list
 */
testdirptr readDirList(FILE *source);

/**
 * Write Directory List
 * @param sink Output file to write table 
 * @param dirlist linked-list of testdir
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
int writeDirList(FILE *sink, testdirptr dirlist);
 
/**
 * Fork and Touch Directory
 * @arg testdir struct with directory (will be modified)
 * @return EXIT_FAILURE or EXIT_SUCCESS
 */
int forktouchdirectory(const testdirptr testdir);

/**
 * Find next testdir by pid
 * @arg dirlist struct with (first) directory
 * @arg pid pid to search for (may be 0)
 * @return NULL if not found or ptr to testdir
 */
testdirptr findNextTestdirByPid(testdirptr dirlist, const pid_t pid);

/**
 * Is test Directory Process Live (aka not caught)
 * @arg dir testdirectory ptr to check
 * @return true if process is considered live and not been caught via wait
 */
int isTestDirProcessLive(testdirptr const dir);

/**
 * Enum of possible returns from waitTouchDirectory
 */
typedef enum { 
    WAITTOUCHDIR_SUCCESS,  ///process waited on successfully
    WAITTOUCHDIR_ERROR, ///error
    WAITTOUCHDIR_AGAIN ///no processes ready yet
} waitTouchDirectoryReturn;                      

/**
 * Wait for child to return and save exit
 * @arg dirlist struct with (first) directory
 * @return enum of states
 */
waitTouchDirectoryReturn waitTouchDirectory(testdirptr dirlist);

/**
 * Enum of possible returns from waitTouchDirectory
 */
typedef enum { 
    TOUCHDIRLIST_SUCCESS,  ///process waited on successfully
    TOUCHDIRLIST_ERROR,  ///error
    TOUCHDIRLIST_TIMEOUT,  ///timed out while waiting
} touchDirectoryListReturn;                      
 
/**
 * Touch Directory List
 * touch each directory upto max_forks at a time
 * will touch each directory and save exit
 * @arg dirlist struct with (first) directory
 * @arg max_forks max sub-processes at a time
 * @arg max_time max timestamp to wait
 * @return see touchDirectoryListReturn
 */
touchDirectoryListReturn touchDirectoryList(testdirptr const dirlist, const int max_forks, const time_t max_time);

/**
 * Wait until max forks have waited for (or timeout)
 * @arg dirlist struct with (first) directory
 * @arg max_forks max sub-processes at a time
 * @arg max_time max timestamp to wait
 * @return see touchDirectoryListReturn
 */
touchDirectoryListReturn waitTouchDirectoryMax(testdirptr const dirlist, const int max_forks, const time_t max_time );

/**
 * Terminate all remaining processes
 * @arg dirlist struct with (first) directory
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
int termDirectoryList(testdirptr const dirlist);

#define RND_FILE_PREFIX "TEST-FILE-DELME" ///prefix to random file name
#define MAX_TEST_FILE_LENGTH 128 ///max length of test file name (generated)
#define MAX_TEST_DIR_CNT 128 ///max number of lines to read containing test directories
#define MAX_TEST_DIR_LENGTH 2048 ///max length of test directory string
/** 
 * scanf format string for reading 1 line of input
 * format: file_name uid
 */
#define TEST_ENTRY_FORMAT "%2047s %u\n"


#endif // TESTDIR_H

