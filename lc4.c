#include "lc4.h"

#include <stdlib.h>
#include <string.h>

static void
safe_free(void *v, size_t n)
{
    volatile unsigned char *p = ( volatile unsigned char * )v;
    while( n-- ) *p++ = 0;
}

static int
safecmp(const void *a, const void *b, const size_t size) 
{
    const unsigned char *_a = (const unsigned char *) a;
    const unsigned char *_b = (const unsigned char *) b;
    int result = 0;

    size_t i;
    for (i = 0; i < size; i++)
        result |= _a[i] ^ _b[i];
 
    return result == 0;
}



static void
LC4_reinit(LC4 *const lc4)
{
    size_t x, y;
    for(x=0; x<16; x++)
        for(y=0; y<16; y++)
            lc4->s[x][y] = lc4->key[x*16+y];

    lc4->i = 0;
    lc4->j = 0;
}

LC4*
LC4_init(LC4 * *const lc4ptr, uint8_t key[256])
{
    LC4* lc4 = NULL;

    if (lc4ptr!=NULL)
      {
        // Use pointer (and maybe memory) provided by the caller
        if (*lc4ptr==NULL)
            *lc4ptr = malloc(sizeof(LC4));
        if (*lc4ptr==NULL)
            return NULL;
        lc4 = *lc4ptr;
      }
    else
      {
        // Allocate memory for ourselves and return it to the caller
        lc4 = malloc(sizeof(LC4));
        if (lc4==NULL)
            return NULL;
      }

    memcpy(lc4->key, key, 256);
    safe_free(key, 256);

    LC4_reinit(lc4);

    return lc4;
}



static void
LC4_right_rotate(LC4 *const lc4, const size_t *const x, size_t *const y,
                 const size_t *const r, size_t *const c)
{
    uint8_t tmp[16];

    size_t i;
    for(i=0; i<16; i++)
        tmp[i] = lc4->s[*r][(size_t)(i+1)%16];
    for(i=0; i<16; i++)
        lc4->s[*r][i] = tmp[i];

    *c = (*c+1)%16;

    if(safecmp(x, r, sizeof(size_t)))
        *y = (size_t)(*y+1)%16;
    if(safecmp(&lc4->i, r, sizeof(size_t)))
        lc4->j = (size_t)(lc4->j+1)%16;
}

static void
LC4_down_rotate(LC4 *const lc4, size_t *const x, const size_t *const y,
                size_t *const r, const size_t *const c)
{
    uint8_t tmp[16];

    size_t i;
    for(i=0; i<16; i++)
        tmp[i] = lc4->s[(size_t)(i+1)%16][*y];
    for(i=0; i<16; i++)
        lc4->s[i][*y] = tmp[i];

    *x = (*x+1)%16;

    if(safecmp(c, y, sizeof(size_t)))
        *r = (size_t)((*r+1)%16);
    if(safecmp(&lc4->j, y, sizeof(size_t)))
        lc4->i = (size_t)(lc4->i+1)%16;
}

static uint8_t
LC4_process_byte(LC4 *const lc4, const uint8_t p, int8_t d)
{
    // d: 1 is encrypt, -1 is decrypt
    d = (d==0) ? 1 : -1;
    size_t r=0, c=0;
    size_t x=0, y=0;

    size_t i, j;
    for(i=0; i<16; i++)
      {
        for(j=0; j<16; j++)
          {
            if(lc4->s[i][j] == p)
              {
                r = i;
                c = j;
              }
          }
      }

    x = (r+d*(lc4->s[lc4->i][lc4->j]/16)) % 16;
    y = (c+d*(lc4->s[lc4->i][lc4->j]%16)) % 16;
    const uint8_t C = lc4->s[x][y];

    if(d == 1)
      {
        /*
         * Encrypt
         * Whitepaper vs implementation:
         * r: r, c: c, x: x, y: y
         */

        LC4_right_rotate(lc4, &x, &y, &r, &c);
        LC4_down_rotate(lc4, &x, &y, &r, &c);

        lc4->i = (lc4->i+(C/16))%16;
        lc4->j = (lc4->j+(C%16))%16;
      }
    else
      {
        /*
         * Decrypt
         * Whitepaper vs implementation:
         * r: x, c: y, x: r, y: c
         */

        LC4_right_rotate(lc4, &r, &c, &x, &y);
        LC4_down_rotate(lc4, &r, &c, &x, &y);

        lc4->i = (lc4->i+(p/16))%16;
        lc4->j = (lc4->j+(p%16))%16;
      }

    return C;
}

static uint8_t
LC4_decrypt_byte(LC4 *const lc4, const uint8_t byte)
{
    return LC4_process_byte(lc4, byte, 1);
}

static uint8_t
LC4_encrypt_byte(LC4 *const lc4, const uint8_t byte)
{
    return LC4_process_byte(lc4, byte, 0);
}

int
LC4_decrypt(LC4 *const lc4, const LC4_ciphertext *const ct, LC4_plaintext *const pt)
{
    LC4_reinit(lc4);

    if (pt->plaintext == NULL)
        pt->plaintext = malloc(ct->size - pt->signature_size);
    pt->size = ct->size - pt->signature_size;

    if (pt->plaintext == NULL)
        return 0;

    size_t c;

    for (c=0; c<ct->nonce_size; c++)
        LC4_encrypt_byte(lc4, ct->nonce[c]);

    for (c=0; c<pt->header_size; c++)
        LC4_encrypt_byte(lc4, pt->header[c]);

    for (c=0; c<pt->size; c++)
        pt->plaintext[c] = LC4_decrypt_byte(lc4, ct->ciphertext[c]);

    uint8_t error = 0;
    for (c=0; c<pt->signature_size; c++)
      {
        uint8_t decrypted_signature = LC4_decrypt_byte(lc4, ct->ciphertext[c+pt->size]);
        uint8_t signature = pt->signature[c];
        error |= decrypted_signature ^ signature;
      }

    pt->nonce_size = ct->nonce_size;
    memcpy(pt->nonce, ct->nonce, ct->nonce_size);

    return error == 0;
}

int
LC4_encrypt(LC4 *const lc4, const LC4_plaintext *const pt, LC4_ciphertext *const ct)
{
    LC4_reinit(lc4);

    if (ct->ciphertext == NULL)
        ct->ciphertext = malloc(pt->size + pt->signature_size);
    ct->size = pt->size + pt->signature_size;

    if (ct->ciphertext == NULL)
        return 0;

    size_t c;

    for (c=0; c<pt->nonce_size; c++)
        LC4_encrypt_byte(lc4, pt->nonce[c]);

    for (c=0; c<pt->header_size; c++)
        LC4_encrypt_byte(lc4, pt->header[c]);

    for (c=0; c<pt->size; c++)
        ct->ciphertext[c] = LC4_encrypt_byte(lc4, pt->plaintext[c]);

    for (c=0; c<pt->signature_size; c++)
        ct->ciphertext[c+pt->size] = LC4_encrypt_byte(lc4, pt->signature[c]);

    ct->nonce_size = pt->nonce_size;
    memcpy(ct->nonce, pt->nonce, pt->nonce_size);

    return 1;
}

