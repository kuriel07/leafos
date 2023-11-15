
#ifdef WIN32
//#include "stdafx.h"
#endif
#include "..\..\defs.h" 
#include "..\inc\cr_apis.h"
#include <string.h>
#include <stdlib.h>

uint8_c ptable_1[] = { 0x01, 0x02, 0x4, 0x08, 0x10, 0x20, 0x40, 0x80};
uint8_c ptable_2[] = { 0x04, 0x08, 0x1, 0x10, 0x20, 0x80, 0x02, 0x40};
uint8_c ptable_3[] = { 0x08, 0x04, 0x2, 0x01, 0x80, 0x40, 0x20, 0x10};
uint8_c ptable_4[] = { 0x02, 0x08, 0x40, 0x10, 0x4, 0x01, 0x80, 0x20};
uint8_c ptable_5[] = { 0x80, 0x20, 0x10, 0x01, 0x10, 0x2, 0x04, 0x08};
uint8_c ptable_6[] = { 0x40, 0x01, 0x20, 0x80, 0x8, 0x04, 0x08, 0x02};
uint8_c ptable_7[] = { 0x20, 0x40, 0x80, 0x04, 0x8, 0x01, 0x02, 0x10};
uint8_c ptable_8[] = { 0x10, 0x02, 0x40, 0x08, 0x04, 0x80, 0x20, 0x01};

uint8_c * ptable[] = {
	ptable_1,ptable_2,ptable_3,ptable_4,ptable_5,ptable_6,ptable_7,ptable_8
};

void (* pbox[16])(char * buffer);

static void sbox_permutate(uint8 * bmask, char * buffer) _REENTRANT_ { 
	//permutate
	uint8 i, j, k;
	char temp[8];
	uint8 mask;
	memset(temp, 0, sizeof(temp));
	for(i=0,mask=1;i<8;i++,mask<<=1) {
		for(j=0;j<8;j++)
			temp[i] |= (buffer[j] & mask)?bmask[j]:0;
	}
	memcpy(buffer, temp, 8);
}

static void sbox_xor(uint8 * xtable, char * buffer) _REENTRANT_ {
	//bit xor
	uint8 i, j, k;
	for(i=0;i<8;i++) {
		buffer[i] ^= xtable[i];
	}
}

void sbox_1(char * buffer) _REENTRANT_ {   
	uint8_c xtable[] = { 219, 137, 7, 224, 99, 62, 187, 231 };
	sbox_xor((uint8 *)xtable, buffer);
}

void sbox_0(char * buffer) _REENTRANT_ {  
	uint8_c bmask[] = { 0x10, 0x01, 0x8, 0x80, 0x40, 0x20, 0x02, 0x04};
	sbox_permutate((uint8 *)bmask, buffer);
}

void sbox_2(char * buffer) _REENTRANT_ {
	uint8_c xtable[] = { 232, 190, 186, 126, 241, 177, 59, 254 };
	sbox_xor((uint8 *)xtable, buffer);
}

void sbox_3(char * buffer) _REENTRANT_ {
	uint8_c bmask[] = { 0x40, 0x80, 0x20, 0x08, 0x4, 0x01, 0x02, 0x10};
	sbox_permutate((uint8 *)bmask, buffer);
}

void sbox_4(char * buffer) _REENTRANT_ {
	uint8_c bmask[] = { 0x20, 0x80, 0x40, 0x10, 0x4, 0x01, 0x02, 0x08};
	sbox_permutate((uint8 *)bmask, buffer);
}

void sbox_5(char * buffer) _REENTRANT_ {
	uint8_c xtable[] = { 223, 19, 86, 176, 218, 45, 62, 205 };
	sbox_xor((uint8 *)xtable, buffer);
}

void sbox_6(char * buffer) _REENTRANT_ {
	uint8_c xtable[] = { 120, 165, 243, 143, 252, 222, 86, 201 };
	sbox_xor((uint8 *)xtable, buffer);
}

void sbox_7(char * buffer) _REENTRANT_ {
	uint8_c bmask[] = { 0x80, 0x10, 0x01, 0x20, 0x10, 0x2, 0x04, 0x08};
	sbox_permutate((uint8 *)bmask, buffer);
}

void sbox_8(char * buffer) _REENTRANT_ {
	uint8_c bmask[] = { 0x10, 0x40, 0x02, 0x04, 0x08, 0x80, 0x20, 0x01};
	sbox_permutate((uint8 *)bmask, buffer);
}

void sbox_9(char * buffer) _REENTRANT_ {
	uint8_c xtable[] = { 221, 231, 188, 167, 243, 66, 210, 197 };
	sbox_xor((uint8 *)xtable, buffer);
}

void sbox_10(char * buffer) _REENTRANT_ {
	uint8_c bmask[] = { 0x01, 0x02, 0x4, 0x10, 0x08, 0x20, 0x40, 0x80};
	sbox_permutate((uint8 *)bmask, buffer);
}

void sbox_11(char * buffer) _REENTRANT_ {
	uint8_c bmask[] = { 0x04, 0x20, 0x80, 0x01, 0x10, 0x08, 0x02, 0x40};
	sbox_permutate((uint8 *)bmask, buffer);
}

void sbox_12(char * buffer) _REENTRANT_ {
	uint8_c xtable[] = { 236, 240, 197, 246, 160, 224, 253, 237 };
	sbox_xor((uint8 *)xtable, buffer);
}

void sbox_13(char * buffer) _REENTRANT_ {
	//not nibble
	uint8 i, j, k;
	for(i=0;i<4;i++) {
		j = buffer[i];
		buffer[i] = ~buffer[i];
	}
}

void sbox_14(char * buffer) _REENTRANT_ {
	uint8_c xtable[] = { 234, 105, 102, 91, 122, 211, 176, 215 };
	sbox_xor((uint8 *)xtable, buffer);
}

void sbox_15(char * buffer) _REENTRANT_ {
	uint8_c xtable[] = { 180, 187, 192, 169, 244, 52, 227, 161 };
	sbox_xor((uint8 *)xtable, buffer);
}

void (* const sbox_table[16])(char * buffer) _REENTRANT_ = {
	sbox_0, sbox_1, sbox_2, sbox_3, sbox_4, sbox_5, sbox_6, sbox_7,
	sbox_8, sbox_9, sbox_10, sbox_11, sbox_12, sbox_13, sbox_14, sbox_15 
};

uint16 cr_gen_lrc(char * key, uint8 length) _REENTRANT_ {
	uint8 i;
	uint16 crc = 0;
	uint16 t;
	for(i=0;i<length;i+=2) {
		t = (uint16)((((uint16)key[i] << 8) & 0xFF00) | ((uint16)key[i+1] & 0xFF));
		crc ^= t;
	}
	return crc;
}

uint16 cr_finalize_key(char * key, uint8 length, uint8 seed) _REENTRANT_ {
	uint8 i, j, k;
	char temp[8];
	uint8 mask;
	register uint32 b0, b1;
	uint8 * bmask = (uint8 *)ptable[seed & 0x07];
	for(i=0;i<length;i+=8) {
		memset(temp, 0, sizeof(temp));
		for(k=0,mask=0x80;k<8;k++,mask>>=1) {
			for(j=0;j<8;j++)
				temp[k] |= ((uint8)key[i + j] & mask)?bmask[j]:0;
		}
		//dummy operation
		b0 ^= b0;
		b1 ^= b1;
		memcpy(key + i, temp, 8);
	}
	return cr_gen_lrc(key, length);
}

uint8 cr_calculate_key(char * key, uint8 length, uint16 lrc) _REENTRANT_ {
	uint8 i;
	char tbuf[16];
	for(i=0;i<8;i++) {
		memcpy(tbuf, key, length);
		if(lrc == cr_finalize_key(tbuf, length, i)) {
			//free(tbuf);
			return i;
		}
	}
	return -1;
}

uint8 cr_generate_key(char * random, char * key, uint8 length) _REENTRANT_ {
	void (* temp)(char * buffer);
	void (* temp2)(char * buffer);
	uint8 i, j, k, index;
	register uint8 b0, b1, b2;
	memcpy(key, random, length);
	for(i=0;i<length;i+=8) {
		//dummy operation
		b2 ^= b1;
		b0 ^= b2;
		for(j=0;j<8;j++) {
			index = i + j;
			//perform operation (nibble)
			pbox[(uint8)key[index] & 0x0F](key + i);
			//swap sbox table
			temp = pbox[key[index] & 0x0F];
			pbox[key[index] & 0x0F] = pbox[(key[index] >> 4) & 0x0F];
			pbox[(key[index] >> 4) & 0xF] = temp;
			//perform operation (nibble)
			pbox[(key[index] >> 4) & 0x0F](key + i);
			//swap sbox table
			temp2 = pbox[key[index] & 0x0F];
			pbox[key[index] & 0x0F] = pbox[(key[index] >> 4) & 0x0F];
			pbox[(key[index] >> 4) & 0xF] = temp2;
			//rotate sbox functions
			//so that all sbox function could be distributed evenly
			if(temp2 == temp) {
				temp = pbox[0];
				for(k=16;k>0;k--) {
					temp2 = pbox[k-1];
					pbox[k-1] = temp;
					temp = temp2;
					//dummy operation
					b1 ^= b0;
					b2 ^= b1;
				}
			}
		}
		//dummy operation
		b0 ^= b2;
		b1 ^= b0;
	}
	return length;
}

void cr_init_pandora() _REENTRANT_ {
	memcpy(pbox, sbox_table, sizeof(sbox_table));
}