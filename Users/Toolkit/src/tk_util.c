#ifdef WIN32
//#include "stdafx.h"
#endif
#include "..\..\crypto\inc\cr_apis.h"
#include "..\..\interfaces\inc\if_apis.h"
#include "..\..\toolkit\inc\tk_apis.h"
#include "..\..\gp\inc\gp_apis.h"
#include "..\..\defs.h"
#include "..\..\config.h"
#include <stdio.h>

typedef struct TKGsmUtfRecord {
	BYTE b;	 		//byte value
	WORD v;			//converted value
} TKGsmUtfRecord;
	
typedef struct TKUtfGsmRecord {
	BYTE b;			//byte value
	WORD v;			//converted value
} TKUtfGsmRecord;

CONST TKGsmUtfRecord g_tbGsmUtfTableExt[] = {
	//extended char table (0x1Bxx)	
	{0x0A, 0x0C},
	{0x14, 0x5E},
	{0x1B, 0x20},
	{0x28, 0x7B},
	{0x29, 0x7D},
	{0x2F, 0x5C},
	{0x3C, 0x5B},
	{0x3D, 0x7E},
	{0x3E, 0x5D},
	{0x40, 0x7C},
	{0x65, 0x20AC},	  		//euro sign
	{0x00, 0x00}
};

CONST TKGsmUtfRecord g_tbGsmUtfTable[] ={
	//GSM 7 bit default to UTF8 table 
 	{1,0xA3}, 
	{3,0xA5},
	{4,0xE8},
	{5,0xE9},
	{6,0xF9},
	{7,0xEC}, 
	{8,0xF2},
	{9,0xC7},
	{0x0B,0xD8},
	{0x0C,0xF8},
	{0x0E,0xC5}, 
	{0x0F,0xE5},
	{0x10,0x394},	
	{0x12,0x3A6},
	{0x13,0x393},
	{0x14,0x39B},
	{0x15,0x3A9}, 
	{0x16,0x3A0},
	{0x17,0x3A8},
	{0x18,0x3A3},  
	{0x19,0x398},
	{0x1A,0x39E},  
	{0x1C,0xC6},
	{0x1D,0xE6},
	{0x1E,0xDF},
	{0x1F,0xC9},  
	{0x24,0xA4},  
	{0x40,0xA1}, 
	{0x5B,0xC4},
	{0x5C,0xD6},
	{0x5D,0xD1},
	{0x5E,0xDC},
	{0x5F,0xA7}, 
	{0x60,0xBF}, 
	{0x7B,0xE4}, 
	{0x7C,0xF6},
	{0x7D,0xF1},
	{0x7E,0xFC},   
	{0x7F,0xE0},
	{0x00,0x00}
};

CONST TKUtfGsmRecord g_tbUtfGsmTable[] = {
	//UCS2 GSM default (WIB Annex A.1)
	{ 0x40, 0},
	{ 0x24, 2},
	{ 0x5f, 0x11},
	{ 0xa3, 1},
	{ 0xa5, 3},
	{ 0xe8, 4},
	{ 0xe9, 5},
	{ 0xf9, 6},
	{ 0xec, 7},
	{ 0xf2, 8},
	{ 0xd8, 0x0b},
	{ 0xf8, 0x0c},
	{ 0xc5, 0x0e},
	{ 0xe5, 0x0f},
	{ 0xc6, 0x1c},
	{ 0xe6, 0x1d},
	{ 0xdf, 0x1e},
	{ 0xc9, 0x1f},
	{ 0xa1, 0x40},
	{ 0xc4, 0x5b},
	{ 0xd6, 0x5c},
	{ 0xd1, 0x5d},
	{ 0xdc, 0x5e},
	{ 0xa7, 0x5f},
	{ 0xbf, 0x60},
	{ 0xe4, 0x7b},
	{ 0xf6, 0x7c},
	{ 0xf1, 0x7d},
	{ 0xfc, 0x7e},
	{ 0xe0, 0x7f},
	//extended character set (WIB annex A.1) 
	{ 0x0c, 0x1b0A},
	{ 0x5e, 0x1b14},
	{ 0x7b, 0x1b28},
	{ 0x7d, 0x1b29},
	{ 0x5c, 0x1b2f},
	{ 0x5b, 0x1b3c},
	{ 0x7e, 0x1b3d},
	{ 0x5d, 0x1b3e},
	{ 0x7c, 0x1b40},
	//UCS2 to GSM default (WIB annex A.2)
	{ 0xc0, 0x41 },	
	{ 0xc1, 0x41 },
	{ 0xc2, 0x41 },
	{ 0xc3, 0x41 },
	{ 0xc8, 0x45 },
	{ 0xca, 0x45 },
	{ 0xcb, 0x45 },
	{ 0xcc, 0x49 },
	{ 0xcd, 0x49 },
	{ 0xce, 0x49 },
	{ 0xcf, 0x49 },
	{ 0xd2, 0x4F },
	{ 0xd3, 0x4F },
	{ 0xd4, 0x4F },
	{ 0xd5, 0x4F },
	{ 0xd9, 0x55 },	
	{ 0xda, 0x55 },
	{ 0xdb, 0x55 },
	{ 0xdd, 0x59 },
	{ 0xe1, 0x61 },
	{ 0xe2, 0x61 },
	{ 0xe3, 0x61 },
	{ 0xe7, 0x09 },
	{ 0xea, 0x65 },
	{ 0xeb, 0x65 },
	{ 0xed, 0x69 },
	{ 0xee, 0x69 },
	{ 0xef, 0x69 },
	{ 0xf3, 0x6f },
	{ 0xf4, 0x6f },
	{ 0xf5, 0x6f },
	{ 0xfa, 0x75 },
	{ 0xfb, 0x75 },
	{ 0xfd, 0x79 },
	{ 0xff, 0x79 },
	{ 0x00, 0x00}
};

static WORD tkGsm2UtfCharTable(BYTE b, TKGsmUtfRecord * table) _REENTRANT_ {
 	BYTE i = 0;
	while(table[i].b != 0) {
	 	if(table[i].b == b) return table[i].v;
		i++;
	}
	return 0;
}

static WORD tkUtf2GsmCharTable(BYTE b) _REENTRANT_ {
	BYTE i = 0;
   	while(g_tbUtfGsmTable[i].b != 0) {
	  	if(g_tbUtfGsmTable[i].b == b) return g_tbUtfGsmTable[i].v;
		i++;
	}
	return 0;
}

BYTE tkUtf8Check(BYTE * buffer, BYTE size) _REENTRANT_ {
	BYTE i;
	for(i=0;i<size;i++) {
		if(buffer[i] & 0x80) return 1;
	}
	return 0;
}

BYTE tkUtf82Ucs2(BYTE * buffer, BYTE size, BYTE max_len) _REENTRANT_ {
	BYTE len = 0;
	BYTE * pbuffer = buffer + max_len - size;
	BYTE adv = 0;
	tk_memcpy(pbuffer, buffer, size);
	while(adv < size) {
		if((pbuffer[adv] & 0x80) == 0) {
			buffer[len++] = 0;
			buffer[len++] = pbuffer[adv];
		} else {
			if((pbuffer[adv] & 0xE0) == 0xC0) {
				buffer[len++] = ((pbuffer[adv] >> 2) & 0x7);
				buffer[len] = (pbuffer[adv++] & 0x03) << 6;
				buffer[len++] |= pbuffer[adv] & 0x3F;
			}
			if((pbuffer[adv] & 0xF0) == 0xE0) {
				buffer[len] = pbuffer[adv++] << 4;
				buffer[len++] |= ((pbuffer[adv] >> 2) & 0x0F);
				buffer[len] = pbuffer[adv++] << 6;
				buffer[len++] |= pbuffer[adv] & 0x3F;
			}
		}
		adv++;
	}
	return len;
}

BYTE tkUtf82Gsm(BYTE * buffer, BYTE size, BYTE max_len) _REENTRANT_ {
	BYTE len = 0;
	BYTE * pbuffer = buffer + max_len - size;
	BYTE adv = 0;
	BYTE b,h;
	WORD v;
	tk_memcpy(pbuffer, buffer, size);
	while(adv < size) {
		if((pbuffer[adv] & 0x80) == 0) {
			b = pbuffer[adv];
			if(b >= 0x20 && b <= 0x23) v = b;
			else if(b >= 0x25 && b <= 0x3f) v = b;
			else if(b >= 0x41 && b <= 0x5A)  v = b;
			else if(b >= 0x61 && b <= 0x7A)  v = b;	
			else if(b == 0x0A) v = b;
			else if(b == 0x0D) v = b;
			#if 0
			else if(b == 0x40) v = 0;
			else if(b == 0x24) v = 2;
			else if(b == 0x5f) v = 0x11;
			else if(b == 0x0c) { buffer[len++] = 0x1b; v = 0x0A;}
			else if(b == 0x5e) { buffer[len++] = 0x1b; v = 0x14;}
			else if(b == 0x7b) { buffer[len++] = 0x1b; v = 0x28;}
			else if(b == 0x7d) { buffer[len++] = 0x1b; v = 0x29;}
			else if(b == 0x5c) { buffer[len++] = 0x1b; v = 0x2f;}
			else if(b == 0x5b) { buffer[len++] = 0x1b; v = 0x3c;}
			else if(b == 0x7e) { buffer[len++] = 0x1b; v = 0x3d;}
			else if(b == 0x5d) { buffer[len++] = 0x1b; v = 0x3e;}
			else if(b == 0x7c) { buffer[len++] = 0x1b; v = 0x40;} 
			#endif
			else {
			 	v = tkUtf2GsmCharTable(b);
				if(v == 0) goto skip_char;
				if(v > 0xFF) { buffer[len++] = (v >> 8); }
			}
			buffer[len++] = (BYTE)v;
		} else {
			if((pbuffer[adv] & 0xE0) == 0xC0) {
				h = ((pbuffer[adv] >> 2) & 0x7);
				b = (pbuffer[adv++] & 0x03) << 6;
				b |= pbuffer[adv] & 0x3F;
				if(h == 0) {
					#if 0
					if(b == 0xa3) v = 1;
					else if(b == 0xa5) v = 3;
					else if(b == 0xe8) v = 4;
					else if(b == 0xe9) v = 5;
					else if(b == 0xf9) v = 6;
					else if(b == 0xec) v = 7;
					else if(b == 0xf2) v = 8;
					else if(b == 0xd8) v = 0x0b;
					else if(b == 0xf8) v = 0x0c;
					else if(b == 0xc5) v = 0x0e;
					else if(b == 0xe5) v = 0x0f;
					else if(b == 0xc6) v = 0x1c;
					else if(b == 0xe6) v = 0x1d;
					else if(b == 0xdf) v = 0x1e;
					else if(b == 0xc9) v = 0x1f;
					else if(b == 0xa1) v = 0x40;
					else if(b == 0xc4) v = 0x5b;
					else if(b == 0xd6) v = 0x5c;
					else if(b == 0xd1) v = 0x5d;
					else if(b == 0xdc) v = 0x5e;
					else if(b == 0xa7) v = 0x5f;
					else if(b == 0xbf) v = 0x60;
					else if(b == 0xe4) v = 0x7b;
					else if(b == 0xf6) v = 0x7c;
					else if(b == 0xf1) v = 0x7d;
					else if(b == 0xfc) v = 0x7e;
					else if(b == 0xe0) v = 0x7f;
					else goto skip_char;
					#else
					v = tkUtf2GsmCharTable(b);
					if(v == 0) goto skip_char;
					#endif
				}
			}
			if((pbuffer[adv] & 0xF0) == 0xE0) {
				h = pbuffer[adv++] << 4;
				h |= ((pbuffer[adv] >> 2) & 0x0F);
				b = pbuffer[adv++] << 6;
				b |= pbuffer[adv] & 0x3F;
				if(h == 0x20 && b == 0xac) { buffer[len++] = 0x1b; v = 0x65;}
				else goto skip_char; 
			} 
			buffer[len++] = v;
		}
		skip_char:
		adv++;
	}
	return len;
}

static BYTE tkUtf8PutBuffer(BYTE * buffer, WORD val) {
   	if(val < 0x80) {
	 	buffer[0] = (BYTE)val;
		return 1;
	} else if(val < 0x800) {
		buffer[0] = ((val >> 6 ) | 0xc0);
		buffer[1] = ((val & 0x3F) | 0x80);
		return 2;
	} else {  
		buffer[0] = ((val >> 12 ) | 0xe0);
		buffer[1] = (((val >> 6) & 0x3F) | 0x80);
		buffer[2] = ((val & 0x3F) | 0x80);
		return 3;
	}
	return 3; 
}

BYTE tkUcs2Utf8(BYTE * buffer, BYTE size, BYTE max_len) _REENTRANT_ {
	BYTE len = 0;
	BYTE * pbuffer = ((BYTE*)buffer + max_len - size);
	BYTE adv = 0;
	WORD val;
	tk_memcpy((BYTE *)pbuffer, buffer, size);
	while(adv < size) {
		val = end_swap16(*(WORD *)(pbuffer + adv));
		adv += 2;
		len += tkUtf8PutBuffer(buffer + len, val) ;	
	}
	return len;	
}


BYTE tkGsm2Utf8(BYTE * buffer, BYTE size, BYTE max_len) _REENTRANT_ {
	BYTE len = 0;
	BYTE * pbuffer = buffer + max_len - size;
	BYTE adv = 0;
	BYTE val;
	WORD b;
	tk_memcpy(pbuffer, buffer, size);
	while(adv < size) {
		val = pbuffer[adv++];
		switch(val) {
		 	case 0x1B:		//extended character set
				val = pbuffer[adv++];
				#if 0
				if(val == 0x0A) { b=0x0C; }
				if(val == 0x14) { b=0x5E; }
				if(val == 0x1B) { b=0x20; }
				if(val == 0x28) { b=0x7B; }
				if(val == 0x29) { b=0x7D; }
				if(val == 0x2F) { b=0x5C; }
				if(val == 0x3C) { b=0x5B; }
				if(val == 0x3D) { b=0x7E; }
				if(val == 0x3E) { b=0x5D; }
				if(val == 0x40) { b=0x7C; }	
				if(val == 0x65) { b=0x20AC; break; }
				#else
				b = tkGsm2UtfCharTable(val, (TKGsmUtfRecord *)g_tbGsmUtfTableExt);
				if(b != 0) break;
				#endif
				continue;
			#if 0
			case 1: b = 0xA3; goto utf82_convert; 
			case 3: b = 0xA5; goto utf82_convert;
			case 4: b = 0xE8; goto utf82_convert;
			case 5: b = 0xE9; goto utf82_convert;
			case 6: b = 0xF9; goto utf82_convert;
			case 7: b = 0xEC; goto utf82_convert; 
			case 8: b = 0xF2; goto utf82_convert;
			case 9: b = 0xC7; goto utf82_convert;
			case 0x0B: b = 0xD8; goto utf82_convert;
			case 0x0C: b = 0xF8; goto utf82_convert;
			case 0x0E: b = 0xC5; goto utf82_convert; 
			case 0x0F: b = 0xE5; goto utf82_convert;
			case 0x10: b = 0x394; goto utf82_convert;
			
			case 0x12: b = 0x3A6; goto utf82_convert;
			case 0x13: b = 0x393; goto utf82_convert;
			case 0x14: b = 0x39B; goto utf82_convert;
			case 0x15: b = 0x3A9; goto utf82_convert; 
			case 0x16: b = 0x3A0; goto utf82_convert;
			case 0x17: b = 0x3A8; goto utf82_convert;
			case 0x18: b = 0x3A3; goto utf82_convert;  
			case 0x19: b = 0x398; goto utf82_convert;
			case 0x1A: b = 0x39E; goto utf82_convert;  
			case 0x1C: b = 0xC6; goto utf82_convert;
			case 0x1D: b = 0xE6; goto utf82_convert;
			case 0x1E: b = 0xDF; goto utf82_convert;
			case 0x1F: b = 0xC9; goto utf82_convert;  
			case 0x24: b = 0xA4; goto utf82_convert;  
			case 0x40: b = 0xA1; goto utf82_convert; 
			case 0x5B: b = 0xC4; goto utf82_convert;
			case 0x5C: b = 0xD6; goto utf82_convert;
			case 0x5D: b = 0xD1; goto utf82_convert;
			case 0x5E: b = 0xDC; goto utf82_convert;
			case 0x5F: b = 0xA7; goto utf82_convert; 
			case 0x60: b = 0xBF; goto utf82_convert; 
			case 0x7B: b = 0xE4; goto utf82_convert; 
			case 0x7C: b = 0xF6; goto utf82_convert;
			case 0x7D: b = 0xF1; goto utf82_convert;
			case 0x7E: b = 0xFC; goto utf82_convert;   
			case 0x7F: b = 0xE0; goto utf82_convert;
			#endif


			case 0x11: b = 0x5F; break;
			case 0: b = 0x40; break;
			case 2: b = 0x24; break;
			default:
			#if 1
				b = tkGsm2UtfCharTable(val, (TKGsmUtfRecord *)g_tbGsmUtfTable);
				if(b != 0) break; 
			#endif
				b = val; 
				break;
		}
		
		len += tkUtf8PutBuffer(buffer + len, b) ;	
		#if 0
		buffer[len++] = (BYTE)b;
		continue;
		utf82_convert:
		buffer[len++] = ((b >> 6 ) | 0xc0);
		buffer[len++] = ((b & 0x3F) | 0x80);
		continue;
		utf83_convert: 
		buffer[len++] = ((b >> 12 ) | 0xe0);
		buffer[len++] = (((b >> 6) & 0x3F) | 0x80);
		buffer[len++] = ((b & 0x3F) | 0x80);
		#endif
	}
	return len;
}

BYTE tkUcs2Gsm(BYTE * buffer, BYTE size, BYTE max_len) _REENTRANT_ {
	BYTE len;
	len = tkUcs2Utf8(buffer, size, max_len);  	//UCS->UTF
	len = tkUtf82Gsm(buffer, len, max_len);		//UTF->GSM
	return len;	
}

BYTE tkGsm2Ucs(BYTE * buffer, BYTE size, BYTE max_len) _REENTRANT_ {
	BYTE len;
	if(size > (max_len >> 1)) return 0;			//unable to convert to UCS2 not enough buffer
	len = tkGsm2Utf8(buffer, size, max_len);	//GSM->UTF
	len = tkUtf82Ucs2(buffer, len, max_len);	//UTF->UCS
	return len;	
}

/* aaaack but it's fast and const should make it shared text page. */
static const unsigned char pr2six[256] =
{
    /* ASCII table */
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 62, 64, 64, 64, 63,
    52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 64, 64, 64, 64, 64, 64,
    64,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
    15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 64, 64, 64, 64, 64,
    64, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
    41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64
};

uint16 tk_base64_length(uint8 * bufcoded)
{
    int nbytesdecoded;
    register const unsigned char *bufin;
    register int nprbytes;

    bufin = (const unsigned char *) bufcoded;
    while (pr2six[*(bufin++)] <= 63);

    nprbytes = (bufin - (const unsigned char *) bufcoded) - 1;
    nbytesdecoded = ((nprbytes + 3) / 4) * 3;

    return nbytesdecoded + 1;
}

uint16 tk_base64_encode(uint8 * bytes_to_encode, uint16 in_len, uint8 * outbuf) {
  uint16 out_len = 0;
  uint16 i = 0;
  uint16 j = 0;
  uint8 char_array_3[3];
  uint8 char_array_4[4];
	const uint8 base64_chars[] = 
             "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
             "abcdefghijklmnopqrstuvwxyz"
             "0123456789+/";

  while (in_len--) {
    char_array_3[i++] = *(bytes_to_encode++);
    if (i == 3) {
      char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
      char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
      char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
      char_array_4[3] = char_array_3[2] & 0x3f;

      for(i = 0; (i <4) ; i++)
        outbuf[out_len++] = base64_chars[char_array_4[i]];
      i = 0;
    }
  }

  if (i)
  {
    for(j = i; j < 3; j++)
      char_array_3[j] = '\0';
	  
    char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
    char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
    char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
    char_array_4[3] = char_array_3[2] & 0x3f;

    for (j = 0; (j < i + 1); j++)
      outbuf[out_len++] = base64_chars[char_array_4[j]];

    while((i++ < 3))
      outbuf[out_len++] = '=';
  }
  outbuf[out_len] = 0;		//EOS
  return out_len;
}

uint16 tk_base64_decode(uint8 * buffer, uint16 size) {
	uint16 nbytesdecoded;
    register const unsigned char *bufin;
    register unsigned char *bufout;
    register int nprbytes;

    //bufin = (const unsigned char *) buffer;
    //while (pr2six[*(bufin++)] <= 63);
    //nprbytes = (bufin - (const unsigned char *) buffer) - 1;
    nprbytes = size;
	nbytesdecoded = ((nprbytes + 3) / 4) * 3;

    bufout = (unsigned char *) buffer;
    bufin = (const unsigned char *) buffer;

    while (nprbytes > 4) {
		*(bufout++) =
			(unsigned char) (pr2six[*bufin] << 2 | pr2six[bufin[1]] >> 4);
		*(bufout++) =
			(unsigned char) (pr2six[bufin[1]] << 4 | pr2six[bufin[2]] >> 2);
		*(bufout++) =
			(unsigned char) (pr2six[bufin[2]] << 6 | pr2six[bufin[3]]);
		bufin += 4;
		nprbytes -= 4;
    }

    /* Note: (nprbytes == 1) would be an error, so just ingore that case */
    if (nprbytes > 1) {
    *(bufout++) =
        (unsigned char) (pr2six[*bufin] << 2 | pr2six[bufin[1]] >> 4);
    }
    if (nprbytes > 2) {
    *(bufout++) =
        (unsigned char) (pr2six[bufin[1]] << 4 | pr2six[bufin[2]] >> 2);
    }
    if (nprbytes > 3) {
    *(bufout++) =
        (unsigned char) (pr2six[bufin[2]] << 6 | pr2six[bufin[3]]);
    }

    *(bufout++) = '\0';
    nbytesdecoded -= (4 - nprbytes) & 3;
    return nbytesdecoded;
}

static uint8 tk_hex2byte(uint8 hexchar) {
	if(hexchar >= 'a' && hexchar <= 'f') return (hexchar - 'a') + 10;
	if(hexchar >= 'A' && hexchar <= 'F') return (hexchar - 'A') + 10;
	if(hexchar >= '0' && hexchar <= '9') return hexchar - '0';
	return 0;
}

uint16 tk_hex2bin(uint8 * hexstring, uint8 * bytes) {
	uint16 i = 0;
	uint8 c;
	uint16 len=0;
	while(hexstring[i] != 0) {
		if(i & 0x01) {
			c <<= 4;
			c |= tk_hex2byte(hexstring[i]);
			bytes[len] = c;
			len++;
		} else {
			c = tk_hex2byte(hexstring[i]);
		}
		i++;
	}
	return len;
}

const char bin2hexchar[] = { '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F' };
uint16 tk_bin2hex(uint8 * bytes, uint16 len, uint8 * hexstring) {
	uint16 j;
	for(j=0;j<len;j++) {
		hexstring[j << 1] = bin2hexchar[bytes[j] >> 4];
		hexstring[(j << 1) + 1] = bin2hexchar[bytes[j] & 0x0F];
	}
	hexstring[len << 1] = 0;
	return len << 1;
}

uint16 tk_html_escape(uint8 * src, uint8 * dst) {
	uint16 i = 0, j = 0;
	uint8 c;
	while(src[i] != 0) {
		c = src[i];
		if(c >= 'a' && c <= 'z') dst[j++] = c;
		else if(c >= 'A' && c <= 'Z') dst[j++] = c;
		else if(c >= '0' && c <= '9') dst[j++] = c;
		else {
			dst[j++] = '%';
			dst[j++] = bin2hexchar[c >> 4];
			dst[j++] = bin2hexchar[c & 0x0F];
		}
		i++;
	}
	//null string
	dst[j++] = 0;
	return j;
}


///////////////////////////////////////DATE TIME APIS//////////////////////////////////////
static uint8_c g_str_year_table[] = { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
static uint8_c g_str_leap_year_table[] = { 0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };


static uint8 iso8601_is_leap_year(uint16 year) {
	if((year % 400) == 0) return 1;
	if((year % 100) == 0) return 0;
	return ((year % 4) == 0);
}

uint32 tk_iso8601_to_filetime(struct datetime * dtime) {
	uint32 filetime = 0;
	uint8 * list_mm;
	uint16 year;
	uint8 i;
	uint16 days = 0;
	filetime = dtime->second + (dtime->minute * 60) + ((dtime->hour + dtime->tz) * 3600);
	if(iso8601_is_leap_year(dtime->year)) list_mm = (uint8 *)g_str_leap_year_table;
	else list_mm = (uint8 *)g_str_year_table;
	//convert to days
	for(i=0;i<dtime->month;i++) {
		days += list_mm[i];
	}
	days += (dtime->date - 1);
	dtime->days = days;
	filetime += (dtime->days * 86400);
	if(dtime->year >= 1970 && dtime->year <2049) {
		year = dtime->year - 1970;
		//if(dtime->days >= 365) year--;
		filetime += (year * 31557600);
	}
	return filetime;
};

uint8 tk_iso8601_from_filetime(struct datetime * dtime, uint32 filetime) {
	uint32 mres = 0;
	uint8 * list_mm;
	uint8 i;
	uint32 residue;
	uint16 days;
	uint16 year = filetime / 31557600;
	dtime->year = 1970 + year;
	if(iso8601_is_leap_year(dtime->year)) { list_mm = (uint8 *)g_str_leap_year_table; }
	else { list_mm = (uint8 *)g_str_year_table; }
restart_calc:
	//if(iso8601_is_leap_year(dtime->year)) { max_days = 366;}
	//else { max_days = 365; }
	residue = filetime % 31557600;
	dtime->days = (residue / 86400);
	days = dtime->days;
	//convert to month-date
	for(i=0;i<12;++i) {
		//mres = ;
		if(days <= (list_mm[i+1])) {
			break;
		} else 
			days -= list_mm[i+1];
		 
	}
	if(days == list_mm[i+1]) { days = 0; i += 1;}		//in-case days = max_days_in_month
	dtime->month = (i+1);
	dtime->date = (days + 1);
	//dtime->date = (residue / 86400);
	//dtime->date ++;
	residue = residue % 86400;
parse_time:
	dtime->hour = residue / 3600;
	residue = residue % 3600;
	dtime->minute = residue / 60;
	dtime->second = residue % 60;
	dtime->tz = 0;
	//if(iso8601_is_leap_year(dtime->year)) { list_mm = (uint8 *)g_str_leap_year_table; }
	//else { list_mm = (uint8 *)g_str_year_table; }
	return 0;
};

uint32 tk_iso8601_decode(char * str) {
	char c;
	uint8 state = 0;
	uint8 index = 0;
	uint16 temp;
	uint8 i;
	uint8 * list_mm;
	uint16 days = 0;
	uint8 negate_tz = 0;
	struct datetime dtime;
	uint16 field[7]={0,0,0,0,0,0,0};
	uint16 flen[] = {4,2,2,2,2,2,4};
	while((c = *str++) != 0) {
		switch(c) {
			case '-':
				if(state == 6) {
					negate_tz = 1;
				}
			case ':': break;

			case '+': break;
			case ' ': 
			case 'T': 
				state = 3;		//time
				break;
			case 'Z': break;
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				temp = field[state];
				temp *= 10;
				temp += (c & 0x0F);
				field[state] = temp;
				index ++;
				if(index == flen[state]) {
					state ++;
					index = 0;
				}
				break;
		}
	}
	if(negate_tz) {
		field[6] = 0 - field[6];
	}
	dtime.year = field[0];
	dtime.month = field[1];
	dtime.date = field[2];
	dtime.hour = field[3];
	dtime.minute = field[4];
	dtime.second = field[5];
	dtime.tz = field[6];
	if(iso8601_is_leap_year(dtime.year)) list_mm = (uint8 *)g_str_leap_year_table;
	else list_mm = (uint8 *)g_str_year_table;
	//convert from month-date
	for(i=0;i<dtime.month;i++) {
		days += list_mm[i];
	}
	days += (dtime.date -1);
	dtime.days = days;
	//state = 0;
	return tk_iso8601_to_filetime(&dtime);
}

void tk_iso8601_encode(struct datetime * dtime, uint8 * buffer) {
	uint8 c = '+';
	if(dtime->tz < 0) c = '-';
	sprintf((char *)buffer, "%04d-%02d-%02d %02d:%02d:%02d%c%02d", dtime->year, dtime->month, dtime->date, dtime->hour, dtime->minute, dtime->second, c, dtime->tz);
}
////////////////////////////////////END OF DATE TIME APIS//////////////////////////////////////
