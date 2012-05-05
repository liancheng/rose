#include "opaque.h"
#include "scanner.h"

#include "rose/port.h"

struct RScanner {
    RLexer* lexer;
    RToken* lookahead_token;
};

static void r_scanner_free(void* scanner, void* client_data);

static RScanner* get_scanner(rsexp context);

RScanner* r_scanner_new()
{
    RScanner* scanner = GC_NEW(RScanner);

    scanner->lexer = malloc(sizeof(RLexer));
    QUEX_NAME(construct_memory)(scanner->lexer, NULL, 0, NULL, NULL, false);
    scanner->lookahead_token = NULL;

    GC_REGISTER_FINALIZER(scanner, r_scanner_free, NULL, NULL, NULL);

    return scanner;
}

static void r_scanner_free(void* obj, void* client_data)
{
    RScanner* scanner = obj;

    QUEX_NAME(destruct)(scanner->lexer);
    free(scanner->lexer);

    if (!scanner->lookahead_token)
        r_scanner_free_token(scanner->lookahead_token);

    free(scanner);
}

static void reload_lexer(rsexp input, RLexer* lex)
{
    QUEX_NAME(buffer_fill_region_prepare)(lex);

    char* begin = (char*)QUEX_NAME(buffer_fill_region_begin)(lex);
    int   size  = QUEX_NAME(buffer_fill_region_size)(lex);
    char* line  = r_pgets(begin, size, input);
    int   len   = line ? strlen(line) : 0;

    QUEX_NAME(buffer_fill_region_finish)(lex, len);
}

static RToken* read_token(rsexp input, RScanner* scanner)
{
    RToken* t = NULL;

    QUEX_NAME(receive)(scanner->lexer, &t);

    if (TKN_TERMINATION == t->_id) {
        reload_lexer(input, scanner->lexer);
        QUEX_NAME(receive)(scanner->lexer, &t);
    }

    return r_scanner_copy_token(t);
}

static RScanner* get_scanner(rsexp context)
{
    return r_opaque_get(r_context_get(context, CTX_SCANNER));
}

void r_scanner_init(rsexp input, rsexp context)
{
    RScanner* scanner = get_scanner(context);
    reload_lexer(input, scanner->lexer);
}

RToken* r_scanner_copy_token(RToken* token)
{
    RToken* res = malloc(sizeof(RToken));
    QUEX_NAME_TOKEN(copy_construct)(res, token);
    return res;
}

// The caller is responsible for freeing the returned token.
RToken* r_scanner_next_token(rsexp input, rsexp context)
{
    RScanner* scanner = get_scanner(context);
    RToken* res = scanner->lookahead_token;

    if (res)
        scanner->lookahead_token = NULL;
    else
        res = read_token(input, scanner);

    return res;
}

// The caller must not free the returned token.
RToken* r_scanner_peek_token(rsexp input, rsexp context)
{
    RScanner* scanner = get_scanner(context);

    if (!scanner->lookahead_token)
        scanner->lookahead_token = read_token(input, scanner);

    return scanner->lookahead_token;
}

rtokenid r_scanner_peek_token_id(rsexp input, rsexp context)
{
    return r_scanner_peek_token(input, context)->_id;
}

void r_scanner_consume_token(rsexp input, rsexp context)
{
    r_scanner_free_token(r_scanner_next_token(input, context));
}

void r_scanner_free_token(RToken* token)
{
    QUEX_NAME_TOKEN(destruct)(token);
    free(token);
}
