#ifdef __APPLE__

#include "detail/sstream.h"
#include "rose/types.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

typedef struct {
    size_t pos;
    size_t size;
    char* buf;
}
ISStream;

typedef struct {
    char** buf_ptr;
    size_t* size_ptr;
    size_t pos;
}
OSStream;

static int isstream_read (void* cookie, char* buf, int size)
{
    ISStream* stream = r_cast (ISStream*, cookie);
    size_t available = stream->size - stream->pos;

    if (size > available)
        size = available;

    memcpy (buf, stream->buf + stream->pos, sizeof (char) * size);
    stream->pos += size;

    return size;
}

static int isstream_write (void* cookie, char const* buf, int size)
{
    ISStream* stream = r_cast (ISStream*, cookie);
    size_t available = stream->size - stream->pos;

    if (size > available)
        size = available;

    memcpy (stream->buf + stream->pos, buf, sizeof (char) * size);
    stream->pos += size;

    return size;
}

static fpos_t isstream_seek (void* cookie, fpos_t offset, int whence)
{
    size_t pos;
    ISStream* stream = r_cast (ISStream*, cookie);

    switch (whence) {
        case SEEK_SET:
            pos = offset;
            break;

        case SEEK_CUR:
            pos = stream->pos + offset;
            break;

        case SEEK_END:
            pos = stream->size + offset;
            break;

        default:
            return -1;
    }

    if (pos > stream->size)
        return -1;

    stream->pos = pos;

    return r_cast (fpos_t, pos);
}

static int isstream_close (void* cookie)
{
    assert (cookie);
    free (cookie);
    return 0;
}

FILE* fmemopen (void* buf, size_t size, char const* mode)
{
    ISStream* stream;
    FILE* file;

    stream = malloc (sizeof (ISStream));

    if (!stream)
        return NULL;

    memset (stream, 0, sizeof (ISStream));

    stream->size = size;
    stream->buf = buf;

    file = funopen (stream, isstream_read, isstream_write,
                    isstream_seek, isstream_close);

    if (!file)
        free (stream);

    return file;
}

static void osstream_grow (OSStream* stream, size_t size)
{
    char* buf;

    if (size > *stream->size_ptr) {
        buf = realloc (*stream->buf_ptr, size + 1);

        if (buf) {
            memset (buf + *stream->size_ptr + 1, 0, size - *stream->size_ptr);
            *stream->buf_ptr = buf;
            *stream->size_ptr = size;
        }
    }
}

static int osstream_read (void* cookie, char* buf, int size)
{
    OSStream* stream;
    size_t available;

    stream = r_cast (OSStream*, cookie);
    osstream_grow (stream, stream->pos + size);
    available = *stream->size_ptr - stream->pos;

    if (size > available)
        size = available;

    memcpy (buf, *stream->buf_ptr + stream->pos, sizeof (char) * size);
    stream->pos += size;

    return size;
}

static int osstream_write (void* cookie, char const* buf, int size)
{
    OSStream* stream;
    size_t available;

    stream = r_cast (OSStream*, cookie);
    osstream_grow (stream, stream->pos + size);
    available = *stream->size_ptr - stream->pos;

    if (size > available)
        size = available;

    memcpy (*stream->buf_ptr + stream->pos, buf, sizeof (char) * size);
    stream->pos += size;

    return size;
}

static fpos_t osstream_seek (void* cookie, fpos_t offset, int whence)
{
    OSStream* stream = r_cast (OSStream*, cookie);

    switch (whence) {
        case SEEK_SET:
            stream->pos = offset;
            break;

        case SEEK_CUR:
            stream->pos += offset;
            break;

        case SEEK_END:
            stream->pos = *stream->size_ptr + offset;
            break;

        default:
            return -1;
    }

    return stream->pos;
}

static int osstream_close (void* cookie)
{
    assert (cookie);
    free (cookie);
    return 0;
}

FILE* open_memstream (char** buf_ptr, size_t* size_ptr)
{
    OSStream* stream;
    FILE* file;

    stream = malloc (sizeof (OSStream));

    if (!stream)
        return NULL;

    *buf_ptr = NULL;
    *size_ptr = 0;

    stream->buf_ptr = buf_ptr;
    stream->size_ptr = size_ptr;
    stream->pos = 0;

    file = funopen (stream, osstream_read, osstream_write,
                    osstream_seek, osstream_close);

    if (!file)
        free (stream);

    return file;
}

#endif /* __APPLE__ */
