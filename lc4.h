#ifndef ACLC4_LC_H
#define ACLC4_LC_H
unsigned char LC4_process_byte(unsigned char b, unsigned int d); // d=1 for decryption
unsigned long LC4_encrypt_block(unsigned char *p, unsigned long plen, 
				unsigned char *key,
				unsigned char *nonce, unsigned long nlen,
				unsigned char *c);
unsigned long LC4_decrypt_block(unsigned char *c, unsigned long clen, 
				unsigned char *key,
				unsigned char *nonce, unsigned long nlen,
				unsigned char *p);
#endif
