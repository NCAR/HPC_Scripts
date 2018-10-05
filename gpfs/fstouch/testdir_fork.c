#include <sys/types.h> 
#include <signal.h>
#include <sys/wait.h> 
#include <grp.h>
#include "testdir.h"

int forktouchdirectory(const testdirptr testdir)
{
    errno = 0;
    pid_t pid = fork();
    if(pid == -1 || errno)
    {
        fprintf(stderr, "Error: Unable to fork()");
        return EXIT_FAILURE;
    }

    ///child
    if(pid == 0)
    {
        ///switch to / to avoid any data leakage if called from root only directory
        {
            errno = 0;
            const int cret = chdir("/");
            if(cret || errno) exit(TOUCHDIR_EXIT_FAIL_CHDIR);
        }

        ///change to gid
        {
            assert(testdir->gid); ///should never be root, which should have been caught previously
            errno = 0;
            const int sret = setgid(testdir->gid);
            if(sret <= -1 || errno ) exit(TOUCHDIR_EXIT_FAIL_SETUID);
        }

        ///set groups (set gid first to avoid standardized non-standard setting of egid in groups)
        { 
            assert(testdir->user_name);
            const int iret = initgroups(testdir->user_name, testdir->gid);
            ///linux will change errno even if there is no error
            if(iret <= -1) exit(TOUCHDIR_EXIT_FAIL_SETUID); 
        }

        ///change to uid
        {
            assert(testdir->uid); ///should never be root, which should have been caught previously
            errno = 0;
            const int sret = setuid(testdir->uid);
            if(sret <= -1 || errno ) exit(TOUCHDIR_EXIT_FAIL_SETUID);
        }

        assert(getgid() == testdir->gid);
        assert(getuid() == testdir->uid); 

        ///concat the full file name
        const uint filenameLength = strlen(testdir->dir) + strlen(testdir->file) + 3; //3=/ + / + \0
        assert(filenameLength > 5);
        char *filename = (char *) malloc(filenameLength ); assert(filename);
        if(filename == NULL) exit(TOUCHDIR_EXIT_FAIL_FILENAME);
        snprintf(filename, filenameLength , "/%s/%s", testdir->dir, testdir->file);

        ///safety check first
        assert(strlen(filename) > 5);
        if(strlen(filename) < 5) exit(TOUCHDIR_EXIT_FAIL_FILENAME);

        ///open file for root only and smash anything that already exists
        errno = 0;
        //const int file = open(filename, O_CREAT|O_NOCTTY|O_EXCL|O_WRONLY, S_IRUSR|S_IWUSR);
        //const int file = open(filename, O_WRONLY|O_CREAT|O_TRUNC|O_LARGEFILE, S_IRUSR|S_IWUSR);
        const int file = open(filename, O_WRONLY|O_CREAT|O_TRUNC);
        assert(file != 2);
        if(file <= -1 || errno) exit(TOUCHDIR_EXIT_FAIL_OPEN);

        ///write file name to make sure writes work
        {
            errno = 0;
            const char teststr[] = "This is a test file. You can safely delete this file.";
            const ssize_t wret = write(file, teststr, strlen(teststr));
            assert(wret);
            if(wret == -1 || errno) exit(TOUCHDIR_EXIT_FAIL_WRITE);
        }

        ///safely close it
        {
            const int cret = close(file);
            if(cret) exit(TOUCHDIR_EXIT_FAIL_CLOSE);
        }
 
        ///unlink file from fs
        const int uret = unlink(filename);
        assert(!uret);
        if(uret) exit(TOUCHDIR_EXIT_FAIL_UNLINK);

        //while(1) usleep(1000);
        // usleep(100000);

        exit(TOUCHDIR_EXIT_SUCCESS);
    }
    else //parent
    {
        ///save pid for later
        testdir->pid = pid;
        testdir->start = time(NULL);
        assert(testdir->start != -1);
    }

    return EXIT_SUCCESS;
}


