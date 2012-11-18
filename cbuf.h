#ifndef __C_BUFFER_H__
#define __C_BUFFER_H__

typedef struct _CBuf CBuf;
struct _CBuf
{
  int alloc, len;
  char *str;
};

CBuf * cbuf_new (int alloc);
void cbuf_append (CBuf *buf, const char *str);
void cbuf_append_double (CBuf *buf, double x);
void cbuf_free (CBuf *buf);
char * cbuf_string (CBuf *buf);

#endif
