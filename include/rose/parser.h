#ifndef __ROSE_PARSER_H__
#define __ROSE_PARSER_H__

#include "rose/sexp.h"

typedef struct RParserState RParserState;

rsexp         r_parser_result (RParserState* parser);
RParserState* r_parse_file    (char*         filename,
                               rsexp         context);
RParserState* r_parse_string  (char*         string,
                               rsexp         context);
RParserState* r_parse_port    (rsexp         port,
                               rsexp         context);

rsexp r_parser_error (RParserState* parser);
rsexp r_parser_last_error (RParserState* parser);

#endif  //  __ROSE_PARSER_H__
