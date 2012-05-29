#ifndef __ROSE_PARSER_H__
#define __ROSE_PARSER_H__

#include "rose/sexp.h"

typedef struct RReaderState RReaderState;

rsexp         r_reader_result    (RReaderState* parser);
RReaderState* r_read_from_file   (char*         filename,
                                  rsexp         context);
RReaderState* r_read_from_string (char*         string,
                                  rsexp         context);
RReaderState* r_read_from_port   (rsexp         port,
                                  rsexp         context);

rsexp r_reader_error      (RReaderState* reader);
rsexp r_reader_last_error (RReaderState* reader);
int   r_reader_line       (RReaderState* reader);
int   r_reader_column     (RReaderState* reader);

#endif  //  __ROSE_PARSER_H__
