#include <sys/types.h>
#include <sys/vfs.h>
#include <stdio.h>

int __default_flush(vfs_file_t *f)
{
    struct _default_buffer *b = (struct _default_buffer *)f->vfsdata;
    int cnt = b->cnt;
    int r;

    if (cnt > 0) {
        r = (*f->write)(f, b->buf, cnt);
    } else {
        r = 0;
    }
    b->cnt = 0;
    b->ptr = 0;
    if (r < cnt) {
        return -1; // failed to flush
    }
    return 0;
}

int __default_filbuf(vfs_file_t *f)
{
    struct _default_buffer *b = (struct _default_buffer *)f->vfsdata;
    int r;

    r = (*f->read)(f, b->buf, _DEFAULT_BUFSIZ);
    if (r < 0) {
        return -1;
    }
    b->cnt = r;
    b->ptr = &b->buf[0];
    return r;
}

int __default_putc(int c,  vfs_file_t *f)
{
    struct _default_buffer *b = (struct _default_buffer *)f->vfsdata;
    int i = b->cnt;
    b->buf[i++] = c;
    c &= 0xff;
    b->cnt = i;
    if (c == '\n' || i == _DEFAULT_BUFSIZ) {
        if (__default_flush(f)) {
            c = -1;
        }
    }
    return c;
}

int __default_getc(vfs_file_t *f) {
    struct _default_buffer *b = (struct _default_buffer *)f->vfsdata;
    int i = b->cnt;
    unsigned char *ptr;
    
//    __builtin_printf("getc: %d\n", i);
    if (i == 0) {
        i = __default_filbuf(f);
//        __builtin_printf("filbuf: %d\n", i);
    }
    if (i <= 0) {
        return -1;
    }
    b->cnt = i-1;
    ptr = b->ptr;
    i = *ptr++;
    b->ptr = ptr;
    return i;
}