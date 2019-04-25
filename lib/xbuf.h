#ifndef GEN_XBUF

/* |size| muss immer als 2^n darstellbar sein. */

#define GEN_XBUF(name,size)						\
static volatile struct name ## _ ## BUF {				\
  uint8_t r,w, buf[ size ];						\
} name ## _ ## buf  ;							\
									\
void name ## _ ## init (void)						\
{									\
  name ## _ ## buf.r = 0;						\
  name ## _ ## buf.w = 0;						\
}									\
									\
void name ## _ ## put (char b)						\
{									\
									\
  register uint8_t a=name ## _ ## buf.w+1;				\
  a &= (size-1);							\
  if( a != name ## _ ## buf.r ) {					\
    name ## _ ## buf.buf[a] = b;					\
    name ## _ ## buf.w = a;						\
  }									\
}									\
                                                                        \
uint8_t name ## _ ##  get  (void)					\
{									\
  register uint8_t a;							\
  if( name ## _ ## buf.r != name ## _ ## buf.w ) {			\
    a = name ## _ ## buf.r+1;						\
    a  &= (size-1);                                                     \
    return name ## _ ## buf.buf[name ## _ ## buf.r=a];			\
  }									\
  return 0;								\
}									\
									\
char name ## _ ## is_empty (void)					\
{									\
  return name ## _ ## buf.r == name ## _ ## buf.w;			\
}									\
                                                                        \
int name ## _ ## queue_len (void)                                       \
{                                                                       \
  if( name ## _buf.w >= name ## _buf.r )                                \
  	return name ## _buf.w - name ## _buf.r;                         \
                                                                        \
  if(name ## _buf.r > name ## _buf.w)                                   \
        return name ## _buf.w + (size-1) -  name ## _buf.r + 1 ;        \
                                                                        \
          return name ## _buf.w - name ## _buf.r;                       \
}                                                                       \
									\
char name ## _ ## is_full (void)					\
{									\
  return ( ((name ## _ ## buf.w+1) & ( size -1)) == name ## _ ## buf.r ); \
}									\
									\
									\
char name ## _ ## getc (void)						\
{									\
  while( name ## _ ## is_empty () ) WDT_RESET();                        \
  return (char)name ## _ ## get();					\
}

#endif
