#include <sys/types.h> 
#include <signal.h>
#include <sys/wait.h> 
#include "testdir.h"

testdirptr findNextTestdirByPid(testdirptr dirlist, const pid_t pid)
{
    assert(dirlist);
    if(dirlist == NULL) return NULL;        
    assert(pid != getpid());

    while(dirlist)
    {
        assert(dirlist->pid != getpid());
        if(dirlist->pid == pid) return dirlist;
        dirlist = dirlist->next;
    }

    return NULL;
}

waitTouchDirectoryReturn waitTouchDirectory(testdirptr dirlist)
{
    int status = 0;    
    errno = 0;
    pid_t child = waitpid(-1, &status, WNOHANG);

    ///no child are done yet
    if(child == 0) return WAITTOUCHDIR_AGAIN; 
    assert(!errno);
    if(child == -1) return WAITTOUCHDIR_ERROR;

    testdirptr dir = findNextTestdirByPid(dirlist, child);
    assert(dir);
    if(dir == NULL) return WAITTOUCHDIR_ERROR;

    if(WIFEXITED(status))
    {
        dir->exit = WEXITSTATUS(status);  
    }
    else ///didn't exit cleanly
    {
        dir->exit = TOUCHDIR_EXIT_SIG_KILL;
    }

    ///record endtime
    dir->stop = time(NULL);
    assert(dir->stop != -1); 

    return WAITTOUCHDIR_SUCCESS;
}

touchDirectoryListReturn waitTouchDirectoryMax(testdirptr const dirlist, const int max_forks, const time_t max_time ) 
{         
    assert(max_forks >= 0);

    while(1)
    {
        const time_t now = time(NULL);
        assert(now != -1);
        
        ///check for timeout
        if(now > max_time)
            return TOUCHDIRLIST_TIMEOUT;
       
        testdirptr dir = NULL; 
        int apro = 0; ///active process count 

        ///take a full count first
        for(dir = dirlist; dir; dir = dir->next)
            if(isTestDirProcessLive(dir))
                ++apro;

        ///escape if less than max forks are found
        if(apro <= max_forks)
            return TOUCHDIRLIST_SUCCESS;
        else
        {
            int wpro = 0; //wait for process count
            const int cpro = apro; //number of processes to wait for
            for(wpro = 0; wpro <= cpro; ++wpro)
            {
                waitTouchDirectoryReturn wret = waitTouchDirectory(dirlist);
                assert(wret != WAITTOUCHDIR_ERROR);
                if(wret == WAITTOUCHDIR_SUCCESS)
                {
                    --apro;

                    if(apro <= max_forks)
                        return TOUCHDIRLIST_SUCCESS;
                } 
                else if(wret == WAITTOUCHDIR_AGAIN)
                {
                    ///nothing yet                       
                    break;
                }
                else
                {
                    return TOUCHDIRLIST_ERROR;
                }
             } 
        }
        
        ///sleep for a little while
        ///this may be done better using a signal and yielding
        errno = 0;
        struct timespec rqtp={0};
        //rqtp.tv_sec = 1;
        rqtp.tv_nsec = 1000;
        int nret = nanosleep(&rqtp, NULL); 
        ///ignore sleep errors caused by interrupt
        if(nret == -1 && errno != EINTR) 
            return TOUCHDIRLIST_ERROR; 
    }

    assert(0);
    return TOUCHDIRLIST_ERROR;
 }

touchDirectoryListReturn touchDirectoryList(testdirptr const dirlist, const int max_forks, const time_t max_time )
{
    assert(max_forks > 0);
    testdirptr dir = NULL; 
    while((dir = findNextTestdirByPid(dirlist, 0)) != NULL)
    {
        {
            int fret = forktouchdirectory(dir); 
            assert(fret == EXIT_SUCCESS);
            if(fret != EXIT_SUCCESS) return TOUCHDIRLIST_ERROR;
        }
        
        const touchDirectoryListReturn wret = waitTouchDirectoryMax(dirlist, max_forks - 1, max_time);
        if(wret != TOUCHDIRLIST_SUCCESS) return wret;
    }

    ///make sure all process have been waited for
    const touchDirectoryListReturn wret = waitTouchDirectoryMax(dirlist, 0, max_time);
    if(wret != TOUCHDIRLIST_SUCCESS) return wret; 

    return TOUCHDIRLIST_SUCCESS;
}

int isTestDirProcessLive(testdirptr const dir)
{
    return dir->pid && dir->stop == 0 && dir->exit == TOUCHDIR_EXIT_NONE ? 1 : 0;
}

int termDirectoryList(testdirptr const dirlist)
{
    testdirptr dir = dirlist; 
    while(dir != NULL)
    {
        assert(dir);
        ///if there is a pid but no stop time
        ///then app has been killed or caught yet
        if(isTestDirProcessLive(dir))
        {
            assert(dir->pid != getpid());
            assert(dir->pid);

            errno = 0;
            ///child may be stuck in a driver locked state
            ///send the kill sig to make sure the app will die
            ///when ever the driver locked state ends
            const int kret = kill(dir->pid, SIGKILL);
            assert(kret == 0 || (kret == -1 && errno != ESRCH));

            ///program has exited already
            if(kret == -1 && errno != ESRCH)
                return EXIT_FAILURE;
        }

        dir = dir->next;
    }

    {
        errno = 0;
        struct timespec rqtp={0};
        rqtp.tv_nsec = 500;
        ///sleep for 500 mseconds
        ///just long enough for SIGKILL to do its magic
        int nret = nanosleep(&rqtp, NULL); 
        if(nret == -1 && errno != EINTR) return EXIT_FAILURE; 
    }

    return EXIT_SUCCESS;
}

