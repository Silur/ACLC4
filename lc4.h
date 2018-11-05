#ifndef ACLC4_LC_H
#define ACLC4_LC_H

#include <stdint.h>
#include <stddef.h>

#ifndef LC4_BLOCK_SIZE
#    define LC4_BLOCK_SIZE 1024
#endif

#ifndef LC4_NONCE_MAX_SIZE
#    define LC4_NONCE_MAX_SIZE 256
#endif

#ifndef LC4_HEADER_MAX_SIZE
#    define LC4_HEADER_MAX_SIZE 0
#endif

#ifndef LC4_SIGNATURE_MAX_SIZE
#    define LC4_SIGNATURE_MAX_SIZE 256
#endif

typedef struct LC4
{
    uint8_t key[256];
    size_t i;
    size_t j;
    uint8_t s[16][16];
} LC4;



typedef struct LC4_plaintext
{
    size_t nonce_size;
    uint8_t nonce[LC4_NONCE_MAX_SIZE];
    size_t header_size;
    uint8_t header[LC4_HEADER_MAX_SIZE];
    size_t size;
    uint8_t* plaintext;
    size_t signature_size;
    uint8_t signature[LC4_SIGNATURE_MAX_SIZE];
} LC4_plaintext;

// Initializes the LC4 plaintext with default values
static inline LC4_plaintext
LC4_init_plaintext()
{
    LC4_plaintext pt;
    pt.nonce_size = pt.header_size = pt.size = pt.signature_size = 0;
    pt.plaintext = NULL;
    return pt;
}



typedef struct LC4_ciphertext
{
    size_t nonce_size;
    uint8_t nonce[LC4_NONCE_MAX_SIZE];
    size_t size;
    uint8_t* ciphertext;
} LC4_ciphertext;

// Initializes the LC4 ciphertext with default values
static inline LC4_ciphertext
LC4_init_ciphertext()
{
    LC4_ciphertext ct;
    ct.nonce_size = ct.size = 0;
    ct.ciphertext = NULL;
    return ct;
}


/*
 * Initializes LC4 state and purges the key afterwards. Allocates memory for
 * the LC4 state on the heap if given a NULL pointer.
 *
 * Arguments:
 * LC4: NULL or pointer to pointer to a sizeof(LC4) allocated memory
 * key: the 256 byte key of this LC4 cipher instance
 *
 * Returns pointer to LC4 instance if success, NULL if malloc() fails.
 */
LC4* LC4_init(LC4 * *const, uint8_t key[256]);

/*
 * Performs LC4 encryption of the given plaintext. Allocates memory for the
 * ciphertext if given a NULL pointer.
 *
 * Arguments:
 * LC4: the initialized LC4 state
 * LC4_plaintext: the filled plaintext to be encrypted
 * LC4_ciphertext: the encrypted plaintext
 *
 * Returns 1 if success, 0 if malloc() fails.
 */
int LC4_encrypt(LC4 *const, const LC4_plaintext *const, LC4_ciphertext *const);

/*
 * Performs LC4 decryption of the given ciphertext. Allocates memory for the
 * plaintext if given a NULL pointer. The following fields of the plaintext
 * must be filled:
 *  - header, header_size: known to both sides
 *  - signature, signature_size: signature for authentication
 *
 * Arguments:
 * LC4: the initialized LC4 state
 * LC4_ciphertext: the ciphertext to be decrypted
 * LC4_plaintext: the decrypted ciphertext
 *
 * Returns 1 if success, 0 if malloc() or authentication fails
 */
int LC4_decrypt(LC4 *const, const LC4_ciphertext *const, LC4_plaintext *const);

#endif
