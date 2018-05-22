#include "ac.h"
#ifndef LIBCLESS
#include <stdio.h>

/* import symbol table from file
binary format expects 65 byte symbol descriptors after each other:
[1 byte character -> 32/64bit prob low bound -> 32/64 high bound] */
int
ACLC4_import_model(FILE *f, struct ACLC4_symbol **s)
{
	fseek(f, 0, SEEK_END);
	unsigned long size = ftell(f)/((2*sizeof(unsigned long))+1);
	fseek(f, 0, SEEK_SET);
	unsigned long i;
	struct ACLC4_symbol symbols[size];
	int ulsize = sizeof(unsigned long);
	for(i=0; i<size; i++)
	{
		if(fread(&symbols[i].byte, 1, 1, f) != 1) goto read_err;
		if(fread(&symbols[i].low, ulsize, 1, f) != ulsize) goto read_err;
		if(fread(&symbols[i].high, ulsize, 1, f) != ulsize) goto read_err;
	}
	*s = symbols;
	return 1;
read_err:
	fprintf(stderr, "Error reading symbol file! maybe wrong format?\n");
	return 0;
}

int ACLC4_output_byte(FILE *f, unsigned char byte)
{
	return fwrite(&byte, 1, 1, f) == 1;
}

#endif
/* mask an output bit to a byte, then if the byte is full write it
   somewhere (defined in output_bit */
static int 
mask_output(unsigned char a)
{
	static unsigned char out_byte;
	static unsigned char out_mask=0x80;
	if(a) out_byte |= out_mask;
	out_mask >>= 1;

	if(!out_byte)
	{
#ifndef LIBCLESS
		ACLC4_output_byte(stdout, out_byte);
#else
		ACLC4_output_byte(out_byte);
#endif
		out_byte=0;
	}
}
int main()
{
	struct ACLC4_symbol symbols[4] = {{'A', 0, 10}, {'B', 10, 30}, {'C', 30, 60}, {'D', 60, 100}};
	unsigned int dc = 0;
	unsigned int range[2] = {0x00, 0xffffffff};//0x5f5e100};
	unsigned char *input = "CACBAD";
	unsigned long size = 6;
	int i;
	int j;
	for(i=0; i<size; i++)
	{
		for(j=0; j<4; j++)
		{
			if(input[i] == symbols[j].byte)
			{
				unsigned int interval = range[1] - range[0];
				range[0] = (interval*symbols[j].low)/100;
				range[1] = (interval*symbols[j].high)/100 - 1;
				for(;;)
				{
					if((range[0] & 0x80000000) == (range[1] & 0x80000000))
					{
						mask_output(range[0] & 0x80000000);
						while(dc>0)
						{
							mask_output(~range[0] & 0x80000000);
							dc--;
						}
					}
					else if( (range[0] & 0x40000000) && !(range[1] & 0x40000000))
					{
						dc++;
						range[0] &= 0x3fffffff;
						range[1] |= 0x40000000;
					}
					else break;

					range[0] <<= 1;
					range[1] <<= 1;
					range[1] |= 1;
				}
			}
		}
	}
	printf("\n[%x, %x)\n", range[0], range[1]); // debug
	return 0;
}
