#include "main.h"

#define CHECKSUM_INIT_VALUE               (0x77)
#define CHECKSUM_ACCUMULATE(CS, TokenID)  (((CS) + (TokenID)) % 0xFF)

int
run_multiple_analyzis(size_t RepetitionN, size_t TokenN, bool PseudoF)
    /* PseudoF == true: Execute only the 'overhead' not the real
     *                  analyzsis.                                */
{
    register int token_id  = TKN_TERMINATION;
    int          checksum  = 0;
    size_t       token_i   = (size_t)-1; 
    size_t       i         = 0;

    ANALYZER_RESET();
    /* The only reason of the checksum is to prevent that the return value
     * of the analyzis is not ommitted by the compiler, because it is not used.
     * Maybe, in this case, even the function call might be omitted.            */
    checksum = CHECKSUM_INIT_VALUE;

    for(i = 0; i < RepetitionN; ++i) {

        /* Assume that the time spent in ANALYZER_ANALYZE/ANALYZER_PSEUDO_ANALYZE
         * is much higher than the test for 'PseudoF' so it will to influence the
         * measurement.                                                           */
        if( PseudoF == false ) {

            for(token_i = 0; token_i < TokenN; ++token_i) {
                ANALYZER_ANALYZE(token_id);
                checksum = CHECKSUM_ACCUMULATE(checksum, token_id); 

#           if ! defined(QUEX_BENCHMARK_SERIOUS)
                printf("TokenID = %s\n", (const char*)(quex::Token::map_id_to_name(token_id)));
                printf("(%i) Checksum: %i\n", (int)token_i, (int)checksum);
#           endif
            } 
            /* Overhead-Intern: (addition, modulo division, assignment, 
             *                   increment by one, comparison) * token_n */
        } else {

            for(token_i = 0; token_i < TokenN; ++token_i) {
                token_id = pseudo_scan();
                checksum = CHECKSUM_ACCUMULATE(checksum, token_id); 
            } 

        }
        ANALYZER_RESET();
    }
    /* Need to return 'checksum' so that it is not omitted by the compiler
     * because it is not used.                                             */
    return checksum;
}

void      
get_statistics(int* checksum, int* token_n, double* time_per_run_ms)
    /* The time_per_run_ms is only an estimate, not necessarily 
     * a propper measurement.                                     */
{
    QUEX_TYPE_TOKEN_ID  token_id  = TKN_TERMINATION;
    const clock_t       StartTime = clock();
    clock_t             end_time  = (clock_t)-1;
    /* Run at least 200 ms */
    const clock_t       MinTime   = (clock_t)(StartTime + 0.2 * (double)CLOCKS_PER_SEC);
    double              repetition_n = 0;


    int n = 0;
    do {
        // (*) loop until the 'termination' token arrives
        *checksum = CHECKSUM_INIT_VALUE;
        for(n=0; ; ++n) {
            ANALYZER_ANALYZE(token_id);

            *checksum = CHECKSUM_ACCUMULATE(*checksum, token_id); 
#           if ! defined(QUEX_BENCHMARK_SERIOUS)
            // printf("TokenID = %s\n", (const char*)(quex::Token::map_id_to_name(token_id))); 
            printf("TokenID = %i\n", (int)token_id); 
            printf("(%i) Checksum: %i\n", (int)*token_n, (int)*checksum);
#           endif
#           ifdef ANALYZER_GENERATOR_RE2C
            /* WorkArround check for terminating zero by hand: 
             * How \000 => TKN_TERMINATION in re2c? */
            if( *global_re2c_buffer_iterator == 0 ) break;
#           else
            if( token_id == TKN_TERMINATION ) break;
#           endif
        } 
        ANALYZER_RESET();
        end_time = clock();
        repetition_n += 1.0;
    } while( end_time < MinTime );

    *token_n = n;
    printf("// TokenN: %i [1]\n", (int)n);
    *time_per_run_ms = (end_time - StartTime) / repetition_n / (double)CLOCKS_PER_SEC;
}

