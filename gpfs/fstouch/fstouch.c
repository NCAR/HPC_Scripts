/**
 * @brief Filesystem Touch
 * Given a list of which formated as such
 *      {directory name (absolute path)} {user id}\n
 *
 * The list will be read in. The program will then fork
 * out a new process per directory with a max number of 
 * forks at a time. Each new process will then attempt
 * to open a file with random filename to avoid any caching.
 * A small bit will be written to the file. The file will
 * then be closed and unlinked. If there are any failures
 * or all of these events should not complete, it will be
 * returned in the status field. The time it takes for 
 * the touch to complete will also be recorded.
 *
 * The program will enforce a hard limit (in seconds)
 * for the total time taken to touch all of the directories.
 * It is expected that occasionally, the child processes
 * will get hung by the kernel and will fail to return.
 * If they should be hung, they will be sent a kill signal
 * and then cleaned up if possible. No matter what happens
 * to the child process, the parent should survive and report
 * back before the timeout.
 * 
 * The output will be as follows per directory entry:
 *      {directory name} {status} {elapsed time}
 */
#include<stdlib.h>
#include<stdio.h>
#include<assert.h>
#include<errno.h>
#include "testdir.h"
#include <sys/resource.h>

/**
 * be unnice
 * enforce a high priority as this is a time critical test
 * @return EXIT_SUCESS or EXIT_FAILURE
 */
int unnice()
{
    const int which = PRIO_PROCESS;
    const int priority = -1;
    const id_t pid = getpid();

    errno = 0;
    const int ret = setpriority(which, pid, priority);
    if(ret || errno) return EXIT_FAILURE;
    return EXIT_SUCCESS;
}

int main(int argc, char *argv[ ])
{
    if(argc != 4)
    {
        fprintf(stderr, "%s {directory list file} {max time (seconds)} {max child process}\n", argv[0]);
        return EXIT_FAILURE;
    }

    ///user must be root, setuid wont work correctly otherwise
    if(getuid() != 0)
    {
       fprintf(stderr, "You must be root\n");
       return EXIT_FAILURE; 
    }

    testdirptr dirlist = NULL;
    int max_time = 0;
    int max_forks = 0;

    {
        errno = 0;
        int sret = sscanf(argv[2], "%i", &max_time);
        if(sret != 1 || errno || max_time <= 0)
        {
            fprintf(stderr, "Invalid Max Time Given. Must be greater than 0.\n");
            return EXIT_FAILURE;
        }
        const time_t now = time(NULL);
        assert(now != -1);
        max_time += (int) now;
        assert(max_time > 1);
    }

    {
        errno = 0;
        int sret = sscanf(argv[3], "%i", &max_forks);
        if(sret != 1 || errno || max_forks <= 0)
        {
            fprintf(stderr, "Invalid Max Child Processes Given. Must be greater than 0.\n");
            return EXIT_FAILURE;
        }
        assert(max_forks > 0);
    }

    {
        errno = 0;
        FILE* dirsource = stdin;
        
        ///aix doesnt have /dev/stdin, so just act like it does
        if(strcmp("/dev/stdin", argv[1]))
            dirsource = fopen(argv[1], "r");

        if(dirsource == NULL || errno != 0)
        {

            fprintf(stderr, "Error: Unable to open %s.\n", argv[1]);
            return EXIT_FAILURE;
        }
     
        dirlist = readDirList(dirsource);
        if(dirlist == NULL)
        {
            fprintf(stderr, "Error: No test directories found.\n");
            return EXIT_FAILURE; 
        }
        fclose(dirsource);
    }

    ///make sure time critical piece has priority
    unnice();
  
    {
        assert(dirlist);
        touchDirectoryListReturn tret = touchDirectoryList(dirlist, max_forks, max_time - 1);
        if(tret == TOUCHDIRLIST_TIMEOUT)
        {
            ///kill all the remaining processes
            int termret = termDirectoryList(dirlist);
            assert(termret == EXIT_SUCCESS);
            if(termret != EXIT_SUCCESS) return EXIT_FAILURE;

            ///cleanup the zombies if possible            
            int wret = waitTouchDirectoryMax(dirlist, 0, max_time); 
            if(wret == TOUCHDIRLIST_ERROR)
                return EXIT_FAILURE;
        }
        else if(tret == TOUCHDIRLIST_ERROR)
        {
            return EXIT_FAILURE;
        }
    }

    return writeDirList(stdout, dirlist);
}

