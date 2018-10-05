#include<sys/types.h>
#include<pwd.h>
#include "testdir.h"

/**
 * Generate random file name
 * @return ptr to string of file name (ownership transfer)
 */ 
static char* genRandomFileName()
{    
    static FILE* frandom = 0;
    if(frandom == 0)
    {
        errno = 0;
        frandom = fopen("/dev/urandom", "r");
        if(frandom == NULL || errno != 0)
        {
            fprintf(stderr, "Error: Unable to open /dev/urandom");
            exit(EXIT_FAILURE);
        }
    }

    uint rnd[3];
    if(fread(rnd, sizeof(uint), 3, frandom) != 3 || ferror(frandom))
    {
        fprintf(stderr, "Error: Unable to read /dev/urandom");
        exit(EXIT_FAILURE); 
    }

    char * file = (char *) malloc(MAX_TEST_FILE_LENGTH); assert(file);
    if(file == NULL) exit(EXIT_FAILURE);

    snprintf(file, MAX_TEST_FILE_LENGTH, "%s-%u-%u-%u", RND_FILE_PREFIX, rnd[0], rnd[1], rnd[2]);
    return file;
}    

testdirptr readDirList(FILE *source)
{
    //ptr to first entry in test directory list
    testdirptr testdirlist = NULL;
    testdirptr lasttestdir = NULL;

    uint dirReadCnt;
    for(dirReadCnt = 0; dirReadCnt < MAX_TEST_DIR_CNT; ++dirReadCnt)
    {  
        char *strdir = malloc(MAX_TEST_DIR_LENGTH); assert(strdir);
        if(strdir == NULL) return NULL;
        uint uid = 0; assert(sizeof(uid_t) == sizeof(uint));
         
        const int fret = fscanf(source, TEST_ENTRY_FORMAT, strdir, &uid);
        if(fret == EOF) break;
        if(ferror(source))
        {
            fprintf(stderr, "Error: Unable to read input.\n");
            return NULL;
        }

        ///Require input to be read correctly
        if(fret != 2)
        {
            fprintf(stderr, "Error: Incorrectly Formated input.\n");
            return NULL; 
        }
    
        ///sanity test directory
        if(strlen(strdir) < 3 || strdir[0] != '/')
        {
            fprintf(stderr, "Error: Test Directory must start with /.\n");
            return NULL;
        }

        if(uid == 0)
        {
            fprintf(stderr, "Error: Invalid user id %u given. User cannot be root.\n", uid);
            return NULL; 
        }
        //fprintf(stdout, "testing: %s\n", strdir);

        testdirptr testdir = (testdirptr) calloc(1, sizeof(struct testdir_struct)); assert(testdir);
        if(testdir == NULL) return NULL;

        testdir->dir = strdir;
        testdir->file = genRandomFileName();

        errno = 0;
        struct passwd *upasswd = getpwuid(uid);
        if(!upasswd || errno)
        {
            fprintf(stderr, "Error: Invalid user id %u given. Unable to lookup user.\n", uid);
            return NULL;  
        }
        assert(upasswd->pw_uid == uid);
        if(upasswd->pw_uid == 0)
        {
            fprintf(stderr, "Error: User default group is root.");
            return NULL;  
        }
 
        testdir->user_name = strdup(upasswd->pw_name);
        assert(testdir->user_name);
        if(testdir->user_name == NULL) return NULL;
        testdir->uid = uid;
        testdir->gid = upasswd->pw_gid;

        if(lasttestdir != NULL)
        {
            assert(!lasttestdir->next);
            lasttestdir->next = testdir;
        }

        //put first entry on list if required
        if(testdirlist == NULL)
            testdirlist = testdir;                    

        lasttestdir = testdir;                    
   }       

    return testdirlist;
}

int writeDirList(FILE *sink, testdirptr dirlist)
{
    assert(dirlist);
    while(dirlist)
    {
        time_t elapsed = 0; 
        if(dirlist->stop > dirlist->start)
            elapsed = dirlist->stop - dirlist->start;
        const char* etype = NULL;

        switch(dirlist->exit)
        {
            case TOUCHDIR_EXIT_NONE:
                if(isTestDirProcessLive(dirlist))
                    etype = "hung";
                else
                    etype = "skipped";
                break;
            case TOUCHDIR_EXIT_SIG_KILL:
                etype = "timeout";
                break;
            case TOUCHDIR_EXIT_SUCCESS:
                etype = "ok";
                break;
            case TOUCHDIR_EXIT_FAIL_FILENAME:
                etype = "invalid_filename";
                break;
            case TOUCHDIR_EXIT_FAIL_OPEN:
                etype = "open_fail";
                break;
            case TOUCHDIR_EXIT_FAIL_WRITE:
                etype = "write_fail";
                break;
            case TOUCHDIR_EXIT_FAIL_UNLINK:
                etype = "unlink_fail";
                break;
            case TOUCHDIR_EXIT_FAIL_CLOSE:
                etype = "close_fail";
                break;
            case TOUCHDIR_EXIT_FAIL_SETUID:
                etype = "setuid_fail";
                break;
            case TOUCHDIR_EXIT_FAIL_CHDIR:
                etype = "chdir_fail";
                break;
            default:
                etype = "unknown"; 
                assert(0);
        }

        assert(etype);
        assert(elapsed >= 0);
        fprintf(sink, "%s %s %i\n", dirlist->dir, etype, (int) elapsed);
        dirlist = dirlist->next;
    }
    return EXIT_SUCCESS;
}

