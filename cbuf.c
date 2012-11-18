#include <string.h>
#include <stdlib.h>
#include "cbuf.h"

CBuf *
cbuf_new (int alloc)
{
  CBuf *buf = (CBuf *) malloc (sizeof (CBuf));
  buf->alloc = alloc;
  buf->len = 0;
  buf->str = (char *) malloc (alloc);
  buf->str[0] = '\0';
  return buf;
}

void
cbuf_append (CBuf *buf, const char *str)
{
  int str_len = strlen (str);
  int old_len = buf->len;
  int new_len = buf->len += str_len;
  while (new_len + 1 > buf->alloc)
    buf->str = realloc (buf->str, buf->alloc <<= 1);
  memcpy (buf->str + old_len, str, str_len);
  buf->str[new_len] = '\0';
}

void
cbuf_free (CBuf *buf)
{
  free (buf->str);
  free (buf);
}

char *
cbuf_string (CBuf *buf)
{
  return buf->str;
}

static inline void
reserve (CBuf *buf, int n)
{
  if (buf->len + n + 1 > buf->alloc)
    buf->str = realloc (buf->str, buf->alloc <<= 1);
}

static inline void
append_c (CBuf *buf, char c)
{
  reserve (buf, 1);
  buf->str[buf->len++] = c;
}

/* ripped off from glib */
#define IEEE754_DOUBLE_BIAS 1023
#ifdef BYTE_ORDER
#if BYTE_ORDER == BIG_ENDIAN
union DoubleIEEE754
{
  double x;
  struct {
    unsigned sign : 1;
    unsigned biased_exponent : 11;
    unsigned mantissa_high : 20;
    unsigned mantissa_low : 32;
  } ieee;
};
#else
union DoubleIEEE754
{
  double x;
  struct {
    unsigned mantissa_low : 32;
    unsigned mantissa_high : 20;
    unsigned biased_exponent : 11;
    unsigned sign : 1;
  } ieee;
};
#endif
#else
#error BYTE_ORDER not defined
#endif

#include "cbuf-exponents.inc"

void
cbuf_append_double (CBuf *buf, double x)
{
  static const char hex_digits[16] =
    {'0', '1', '2', '3', '4', '5', '6', '7',
     '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

  union DoubleIEEE754 d;
  d.x = x;

#define APPEND_C(c) buf->str[buf->len++] = (c)
#define APPEND_HEX(d) APPEND_C (hex_digits[d])

  if (d.x == 0.0)
    {
      reserve (buf, 3);
      APPEND_C ('0');
      APPEND_C ('.');
      APPEND_C ('0');
    }
  else if (d.ieee.biased_exponent == 0x7ff)
    {
      /* inf or nan */
      reserve (buf, 4);
      if (d.ieee.mantissa_high == 0 && d.ieee.mantissa_low == 0)
	{
	  /* inf */
	  if (d.ieee.sign)
	    APPEND_C ('-');
	  APPEND_C ('i');
	  APPEND_C ('n');
	  APPEND_C ('f');
	}
      else
	{
	  APPEND_C ('n');
	  APPEND_C ('a');
	  APPEND_C ('n');
	}
    }
  else
    {
      /* for 53 bits of mantissa, need 14 hex digits, 24 chars:
	   -0xd.dddddddddddddp+eeee
	 for simplicity, the leading digit will always be 1 for normalized
	 numbers, or 0 for denormals; the remaining 52 bits fill exactly
	 13 hex digits after the decimal point */
      reserve (buf, 24);

      if (d.ieee.sign)
	APPEND_C ('-');
      APPEND_C ('0');
      APPEND_C ('x');
      if (d.ieee.biased_exponent == 0)
	{
	  /* denormal (no implicit leading 1) */
	  APPEND_C ('0');
	}
      else
	{
	  /* normalized */
	  APPEND_C ('1');
	}
      APPEND_C ('.');
      APPEND_HEX ((d.ieee.mantissa_high >> 16));
      APPEND_HEX ((d.ieee.mantissa_high >> 12) & 0xf);
      APPEND_HEX ((d.ieee.mantissa_high >>  8) & 0xf);
      APPEND_HEX ((d.ieee.mantissa_high >>  4) & 0xf);
      APPEND_HEX ((d.ieee.mantissa_high)       & 0xf);
      APPEND_HEX ((d.ieee.mantissa_low  >> 28));
      APPEND_HEX ((d.ieee.mantissa_low  >> 24) & 0xf);
      APPEND_HEX ((d.ieee.mantissa_low  >> 20) & 0xf);
      APPEND_HEX ((d.ieee.mantissa_low  >> 16) & 0xf);
      APPEND_HEX ((d.ieee.mantissa_low  >> 12) & 0xf);
      APPEND_HEX ((d.ieee.mantissa_low  >>  8) & 0xf);
      APPEND_HEX ((d.ieee.mantissa_low  >>  4) & 0xf);
      APPEND_HEX ((d.ieee.mantissa_low)        & 0xf);

      const char *exponent = exponents[d.ieee.biased_exponent];
      APPEND_C ('p');
      APPEND_C (exponent[0]);
      APPEND_C (exponent[1]);
      APPEND_C (exponent[2]);
      APPEND_C (exponent[3]);
      APPEND_C (exponent[4]);
    }
  buf->str[buf->len] = '\0';
}
