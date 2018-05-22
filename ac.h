#ifndef LIBACLC4_H
#define LIBACLC4_H

#ifndef LIBCLESS
#include <stdio.h>
#endif

struct ACLC4_symbol {
	unsigned char byte;
	unsigned long low;
	unsigned long high;
};


#ifndef LIBCLESS
struct ACLC4_symbol *ACLC4_symbols;
int ACLC4_import_model(FILE *f, struct ACLC4_symbol **s);
int ACLC4_output_byte(FILE *f, unsigned char byte);
#else
extern struct ACLC4_symbol *symbols;
extern int ACLC4_output_byte(unsigned char byte);
#endif
#endif // LIBACLC4_H
