#include <in/main.h>
#include <sys/stat.h>

FILE*  global_fh;

int 
main(int argc, char** argv) 
{        
    {
        if( argc != 2 ) { return -1; }

        global_fh = fopen(argv[1], "r");
        if( global_fh == NULL ) { 
            printf("File '%s' not found.\n", argv[1]);
            return -1;
        }

    }
    const size_t   FileSize = get_file_size(argv[1]);

    scan_init(FileSize);

#   if   defined(QUEX_QUICK_BENCHMARK_VERSION)
    const double   ExperimentTime = 1.0;   // [sec]
#   elif defined(QUEX_BENCHMARK_SERIOUS)
    const double   ExperimentTime = 10.0;  // [sec]
#   else
    const double   ExperimentTime = 0.0;   // [sec]
#   endif

    int     checksum        = 0xFFFF;
    int     token_n         = (size_t)-1;
    double  time_per_run_ms = -1.0;
    {
        get_statistics(&checksum, &token_n, &time_per_run_ms);
    }
    const size_t   RepetitionN = ExperimentTime / time_per_run_ms;

    /* Measure the analyzis time + some overhead ______________________________*/
    const double   StartTime = my_time();
    {
        run_multiple_analyzis(RepetitionN, token_n, /* PseudoF */false);
    }
    const double   EndTime       = my_time();
    const double   Time_ms       = EndTime - StartTime; 
    const double   TimePerRun_ms = Time_ms / (double)RepetitionN;

    /* Measure the overhead ___________________________________________________*/
    const double   RefStartTime = my_time();
    {
        run_multiple_analyzis(RepetitionN, token_n, /* PseudoF */true);
    }
    const double   RefEndTime       = my_time();
    const double   RefTime_ms       = RefEndTime - RefStartTime; 
    const double   RefTimePerRun_ms = RefTime_ms / (double)RepetitionN;

    /* Raw analyzis time = ... */
    /* const double   RawTime_ms = Time_ms - RefTime_ms; */

    /* Reporting _____________________________________________________________*/
    report("only overhead",       RefTime_ms, RepetitionN, FileSize, /* CharacterSize = 1 */ 1);
    report("analyzis + overhead", Time_ms, RepetitionN, FileSize, /* CharacterSize = 1 */ 1);
    
    final_report(TimePerRun_ms, RefTimePerRun_ms, 
                 argv[1], FileSize, 
                 token_n, RepetitionN, 
                 get_file_size(argv[0], true));

    {
#       ifndef _POSIX_
        printf("Warning: precise measurements can currently only be performed on POSIX systems.\n");
#       endif
    }

    return 0;
} 

size_t
get_file_size(const char* Filename, bool SilentF /*=false*/)
{
    using namespace std;
    struct stat s;
    stat(Filename, &s);
    if( ! SilentF ) {
        printf("// FileSize: %i [Byte] = %f [kB] = %f [MB].\n", 
               (int)s.st_size,
               (float)(double(s.st_size) / double(1024.0)),
               (float)(double(s.st_size) / double(1024.0*1024.0)));
    }
    return s.st_size;
}

QUEX_TYPE_TOKEN_ID 
pseudo_scan()
{
    return (QUEX_TYPE_TOKEN_ID)1;
}

