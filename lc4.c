#include "lc4.h"
static unsigned char state_matrix[16][16];
static void
safe_free(void *v, int n)
{
    volatile unsigned char *p = ( volatile unsigned char * )v;
    while( n-- ) *p++ = 0;
}

static int
safecmp(const void * a, const void *b, const unsigned long size) 
{
  const unsigned char *_a = (const unsigned char *) a;
  const unsigned char *_b = (const unsigned char *) b;
  unsigned char result = 0;
  unsigned long i;
 
  for (i = 0; i < size; i++) {
    result |= _a[i] ^ _b[i];
  }
 
  return result == 0;
}

unsigned char
LC4_process_byte(unsigned char p, unsigned int d)
{
    d = (d==0) ? 1 : -1;
    unsigned int j;
    unsigned int k;
    unsigned int r, c;
    unsigned int x, y;
    static unsigned int mx = 0;
    static unsigned int my = 0;
    for(j=0; j<16; j++)
        for(k=0; k<16; k++)
            if(state_matrix[j][k] == p)
            {
                r = j;
                c = k;
            }
    x = (r+d*(state_matrix[mx][my]/16)) & 15;
    y = (c+d*(state_matrix[mx][my]&15)) & 15;
    unsigned char C = state_matrix[x][y];
    
    if(d)
    {
        for(j=0; j<16; j++)
            state_matrix[r][j] = state_matrix[r][(unsigned int)(j+1)&15];
        c = (c+1)&255;
        if(safecmp(&x, &r, sizeof(unsigned int))) y = (unsigned int)(y+1)&255;
        if(safecmp(&mx, &r, sizeof(unsigned int))) my = (unsigned int)(my+1)&255;

        for(j=0; j<16; j++)
            state_matrix[j][y] = state_matrix[(unsigned int)(j+1)&15][y];
        // x = (x+1)&255;
        if(safecmp(&c, &y, sizeof(unsigned int))) r = (unsigned int)(r+1)&255;
        if(safecmp(&my, &y, sizeof(unsigned int))) mx = (unsigned int)(mx+1)&255;
    }
    else
    {
        for(j=0; j<16; j++)
            state_matrix[x][j] = state_matrix[x][(unsigned int)(j+1)&15];
        y = (y+1)&255;
        if(safecmp(&x, &r, sizeof(unsigned int))) y = (unsigned int)(y+1)&255;
        if(safecmp(&mx, &x, sizeof(unsigned int))) my = (unsigned int)(my+1)&255;

        for(j=0; j<16; j++)
            state_matrix[j][c] = state_matrix[(unsigned int)(j+1)&15][c];
        // r = (x+1)&255;
        if(safecmp(&c, &y, sizeof(unsigned int))) r = (unsigned int)(r+1)&255;
        if(safecmp(&my, &y, sizeof(unsigned int))) mx = (unsigned int)(mx+1)&255;

    }
}

unsigned long
LC4_encrypt_block(unsigned char *p, unsigned long plen, 
            unsigned char *key,
            unsigned char *nonce, unsigned long nlen,
            unsigned char *c)
{
	if(!c) return 0;
    unsigned int i;
    unsigned int j;
    for(i=0; i<16; i++)
        for(j=0; j<16; j++)
            state_matrix[i][j] = key[i*16+j];

    safe_free(key, 256);

	for(i=0; i<nlen; i++)
		LC4_process_byte(nonce[i], 0);

    for(i=0; i<plen; i++)
    {
        c[i] = LC4_process_byte(p[i], 0);
    }
	return i;
}
unsigned long LC4_decrypt_block(unsigned char *c, unsigned long clen, 
				unsigned char *key,
				unsigned char *nonce, unsigned long nlen,
				unsigned char *p)
{
	if(!p) return 0;
    unsigned int i;
    unsigned int j;
    for(i=0; i<16; i++)
        for(j=0; j<16; j++)
            state_matrix[i][j] = key[i*16+j];

    safe_free(key, 256);

	for(i=0; i<nlen; i++)
		LC4_process_byte(nonce[i], 1);

    for(i=0; i<clen; i++)
    {
		p[i] = LC4_process_byte(c[i], 1);
    }
	return i;
}
