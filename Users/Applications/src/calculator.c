#include "..\..\crypto\inc\cr_apis.h"
#include "..\..\interfaces\inc\if_apis.h"
#include "..\..\toolkit\inc\tk_apis.h"
#include "..\..\gp\inc\gp_apis.h"
#include "..\..\defs.h"
#include "..\..\build.h"
#include "..\..\config.h"
#include "..\..\gui\inc\ui_resources.h"
#include "..\..\gui\inc\ui_keyboard.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "jansson.h"
#if SHARD_SSL_SUPPORT
#include "wolfssl\version.h"
#endif
#include "libpng\png.h"
#include "zlib\zlib.h"
#include "jconfig.h"
#include <math.h>

#define OPC_NONE			0x00
#define OPC_HEX			0x300
#define OPC_DEC			0x301
#define OPC_OCT			0x302
#define OPC_BIN			0x303

#define OPC_AND			0x304
#define OPC_OR			0x305
#define OPC_XOR			0x306
#define OPC_NOT			0x307

#define OPC_SHL			0x308
#define OPC_SHR			0x309

#define OPC_2ND			0x30A
#define OPC_ADD			0x310
#define OPC_SUB			0x311
#define OPC_MUL			0x312
#define OPC_DIV			0x313
#define OPC_PI			0x314
#define OPC_SQR			0x315
#define OPC_SQRT		0x316
#define OPC_POW			0x317
#define OPC_CBRT		0x318
#define OPC_SIN			0x319
#define OPC_ASIN			0x320
#define OPC_COS			0x321
#define OPC_ACOS			0x322
#define OPC_TAN			0x323
#define OPC_ATAN			0x324
#define OPC_LOG10			0x325
#define OPC_LOG2			0x326
#define OPC_LBR				0x327		//parentheses
#define OPC_RBR				0x328		//parentheses
#define OPC_ANS			0x330
#define OPC_PLUSMIN		0x331
#define OPC_DOT				0x332
#define OPC_CLR			0x333
#define OPC_POP				0x334
#define OPC_PUSH				0x335
#define OPC_ABS				0x336
#define OPC_LOAD			0x337


typedef struct calc_node calc_node;

struct calc_node {
	uint16 opcode;
	double operand;
	calc_node * next;
};

#define MAX_CALC_NODE_STACK		64
#define MAX_CALC_OPR_STACK		128

#define BASE_MODE_HEX		16
#define BASE_MODE_DEC		10
#define BASE_MODE_OCT		8
#define BASE_MODE_BIN		2

static uint16 g_base_mode = BASE_MODE_DEC;
static uint8 g_function_index = 0;				//2nd function

static uint8 g_node_index = 0;
static calc_node * g_node_stack[MAX_CALC_NODE_STACK];
static calc_node * g_current_node = NULL;
static calc_node * g_first_node = NULL;

static double g_oprstack[MAX_CALC_OPR_STACK];
static uint8 g_oprindex = 0;
static double g_accumulator = 0;

static calc_node * create_node(uint16 opcode, double operand, calc_node * next ) {
	calc_node * node = os_alloc(sizeof(calc_node));
	node->operand = operand;
	node->opcode=opcode;
	node->next=next;
	return node;
}

static calc_node * calc_push_stack(calc_node * node) {
	if(g_node_index == (MAX_CALC_NODE_STACK - 1)) return NULL;
		g_node_stack[g_node_index++] = node;
	return node;
}

static calc_node * calc_pop_stack() {
	if(g_node_index == 0) return NULL;
	return g_node_stack[--g_node_index];
}

static calc_node * calc_peek_stack() {
	if(g_node_index < 0) return NULL;
	return g_node_stack[g_node_index - 1];
}

static const uint8 g_arr_char_value[] = 
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,			//0
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,			//1
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,			//2
			0,1,2,3,4,5,6,7,8,9,0,0,0,0,0,0, 			//3
			0,10,11,12,13,14,15,0,0,0,0,0,0,0,0,0,			//4 
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,			//5
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,			//6
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,			//7
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,			//8
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,			//9
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,			//10
		};
		
uint8 calc_char2num(char ch) {
	return g_arr_char_value[ch];
}


double calc_get_value(ui_object * lcd) {
	double ret = 0;
	int i = 0;
	uint8 * content = ((ui_textbox *)lcd)->content;
	switch(g_base_mode) {
		case BASE_MODE_DEC:
			return (double)atoi((const char *)content);
		case BASE_MODE_HEX:
				for ( i=(strlen((const char *)content) - 1);i>=0;i--) { ret *= 16; ret += calc_char2num(content[i]); }
				break;
		case BASE_MODE_OCT:
				for ( i=(strlen((const char *)content) - 1);i>=0;i--) { ret *= 8; ret += calc_char2num(content[i]); }
				break;
		case BASE_MODE_BIN:
				for ( i=(strlen((const char *)content) - 1);i>=0;i--) { ret *= 2; ret += calc_char2num(content[i]); }
				break;
	}
	return ret;
}

void calc_set_value(ui_object * lcd, double val) {
	uint32 i=0;
	uint16 index = 0;
	switch(g_base_mode) {
		case BASE_MODE_DEC:
				if(val != (long)val)
					snprintf((char *)((ui_textbox *)lcd)->content, UI_MAX_TEXT, "%f", val);
				else
					snprintf((char *)((ui_textbox *)lcd)->content, UI_MAX_TEXT, "%ld", (long)val);
				break;
		case BASE_MODE_HEX:
				snprintf((char *)((ui_textbox *)lcd)->content, UI_MAX_TEXT, "%X", (unsigned )val);
				break;
		case BASE_MODE_OCT:
				snprintf((char *)((ui_textbox *)lcd)->content, UI_MAX_TEXT, "%o", (unsigned )val);
				break;
		case BASE_MODE_BIN:
				index = 0;
				for (i=0x80000000;i!=0;i>>=1) {				//only 32 bit length
					((ui_textbox *)lcd)->content[index++] = ((unsigned)val & i)? '1': '0';
				}
				((ui_textbox *)lcd)->content[index++] = 0;
				//snprintf((char *)((ui_textbox *)lcd)->content, UI_MAX_TEXT, "%o", (unsigned )val);
				break;
	}
	ui_reinitialize_object(lcd);
}

void calc_decode(calc_node * node) {
		calc_node * next;
		if(node == NULL) return;
		switch(node->opcode) {
			case OPC_ADD:
				g_accumulator += node->operand;
					break;
			case OPC_SUB:
				g_accumulator -= node->operand;
					break;
			case OPC_MUL:
				g_accumulator *= node->operand;
					break;
			case OPC_DIV:
				g_accumulator /= node->operand;
					break;
			case OPC_AND:
				g_accumulator = (long)g_accumulator & (long)node->operand;
					break;
			case OPC_OR:
				g_accumulator = (long)g_accumulator | (long)node->operand;
					break;
			case OPC_XOR:
				g_accumulator = (long)g_accumulator ^ (long)node->operand;
					break;
			case OPC_NOT:
				g_accumulator = ~(long)g_accumulator;
					break;
			case OPC_POP:
				if(g_oprindex == 0) break;									//no pop
				if(node->next == NULL) break;
				next = node->next;
				next->operand = g_oprstack[--g_oprindex];
				break;
			case OPC_PUSH:
				if(g_oprindex == MAX_CALC_OPR_STACK) break;		//no push
				g_oprstack[g_oprindex++] = g_accumulator;
				break;
			case OPC_SIN:
				g_accumulator = sin(g_accumulator);
				break;
			case OPC_COS:
				g_accumulator = cos(g_accumulator);
				break;
			case OPC_TAN:
				g_accumulator = tan(g_accumulator);
				break;
			case OPC_ASIN:
				g_accumulator = asin(g_accumulator);
				break;
			case OPC_ACOS:
				g_accumulator = acos(g_accumulator);
				break;
			case OPC_ATAN:
				g_accumulator = atan(g_accumulator);
				break;
			case OPC_LOG10:
				g_accumulator = log10(g_accumulator);
				break;
			case OPC_LOG2:
				g_accumulator = log2(g_accumulator);
				break;
			case OPC_SQR:
				g_accumulator = g_accumulator * g_accumulator;
				break;
			case OPC_SQRT:
				g_accumulator = sqrt(g_accumulator );
				break;
			case OPC_POW:
				g_accumulator = pow(g_accumulator, node->operand );
				break;
			case OPC_CBRT:
				g_accumulator = cbrt(g_accumulator);
				break;
			case OPC_SHL:
				g_accumulator = ((long)g_accumulator) >> 1;
				break;
			case OPC_SHR:
				g_accumulator = ((long)g_accumulator) << 1;
				break;
			case OPC_ABS:
				g_accumulator = abs((int)g_accumulator);
				break;
			case OPC_LOAD:
				g_accumulator = node->operand;
				break;
				
		}
}

void calc_deinit() {
	calc_node * iterator = NULL;
	calc_node * to_delete = NULL;
	//clear accumulator
	g_accumulator = 0;
	g_oprindex = 0;
	g_node_index = 0;		//clear node stack
	//release memory
	iterator = g_first_node;
	while(iterator != NULL) {
		to_delete = iterator;
			iterator = iterator->next;
		if(to_delete != NULL) os_free(to_delete);		//release node completely
	}
	g_first_node = NULL;
	g_current_node = NULL;
}

void calc_push_node(ui_object * lcd, uint16 opcode) {
	uint8 * content = ((ui_textbox *)lcd)->content;
	uint16 len;
	uint8 has_dot = FALSE;
	calc_node * iterator = NULL;
	ui_object * calc_keypad1, * calc_keypad2;
	uint16 prev_opcode = 0;
	double val = 0;
	uint16 i = 0;
	switch(opcode) {
		
		case OPC_NOT:
		case OPC_SIN:
		case OPC_ASIN:
		case OPC_COS:
		case OPC_ACOS:
		case OPC_TAN:
		case OPC_ATAN:
				//add load before operation
				if(g_current_node == NULL) {
					g_current_node = create_node(OPC_LOAD, calc_get_value(lcd), NULL );
					calc_push_stack(g_current_node);
					g_first_node = g_current_node;
				}
				g_current_node->operand = (double)atoi((const char *)content);
				g_current_node->next = create_node(opcode, 0, NULL );
				g_current_node = g_current_node->next;
			break;
		case OPC_OR:
		case OPC_AND:
		case OPC_XOR:
		case OPC_ADD:
		case OPC_SUB:
		case OPC_MUL:
		case OPC_DIV:
				if(g_current_node == NULL) {
					g_current_node = create_node(OPC_LOAD, calc_get_value(lcd), NULL );
					calc_push_stack(g_current_node);
					g_first_node = g_current_node;
				}
				g_current_node->operand = (double)atoi((const char *)content);
				g_current_node->next = create_node(opcode, 0, NULL );
				g_current_node = g_current_node->next;
				snprintf((char *)((ui_textbox *)lcd)->content, UI_MAX_TEXT, "0");			//clear lcd (this operation require at least 2 operand
				ui_reinitialize_object(lcd);			//invalidate lcd display
			break;
		case OPC_LBR:
			calc_push_stack(g_current_node);		//push previous node into stack for pop operation
			g_current_node = create_node(OPC_LOAD, 0, NULL );			//create new empty node (this node will become first node)
			calc_push_stack(g_current_node);			//push newly created node into stack for later assignment (set first node)
			snprintf((char *)((ui_textbox *)lcd)->content, UI_MAX_TEXT, "0");			//clear lcd
			ui_reinitialize_object(lcd);			//invalidate lcd display
			break;
		case OPC_RBR:
			//push result into stack
			g_current_node->operand = (double)atoi((const char *)content);
			g_current_node->next = create_node(OPC_PUSH, 0, g_first_node );
			//pop previous stack
			g_first_node = calc_pop_stack();		//set current stack to first node
			g_current_node = calc_pop_stack();
		
			//add pop instruction into current instruction
			prev_opcode = g_current_node->opcode;
			g_current_node->opcode = OPC_POP;		//store to next instruction
			g_current_node->operand = 0;
			g_current_node->next = create_node(prev_opcode, 0, NULL );
			g_current_node = g_current_node->next;
			g_current_node->next = create_node(OPC_NONE, 0, NULL );		//create empty node for next operation
				break;
		
		
				//change display behaviour
		case OPC_HEX:
			val = calc_get_value(lcd);
			g_base_mode = BASE_MODE_HEX; 
			calc_set_value(lcd, val);
			break;
		case OPC_DEC:
			val = calc_get_value(lcd);
			g_base_mode = BASE_MODE_DEC; 
			calc_set_value(lcd, val);break;
		case OPC_OCT:
			val = calc_get_value(lcd);
			g_base_mode = BASE_MODE_OCT; 
			calc_set_value(lcd, val);break;
		case OPC_BIN:
			val = calc_get_value(lcd);
			g_base_mode = BASE_MODE_BIN; 
			calc_set_value(lcd, val);break;
		case OPC_PI:
			
			snprintf((char *)((ui_textbox *)lcd)->content, UI_MAX_TEXT, "3.1415926535");			//use pi
					ui_reinitialize_object(lcd);			//invalidate lcd display
			break;
		
		case 'A':
		case 'B':
		case 'C':
		case 'D':
		case 'E':
		case 'F':
				if(g_base_mode < BASE_MODE_HEX) break;		//skip if hex not selected
		case '8':
		case '9':
				if(g_base_mode < BASE_MODE_DEC) break;		//skip if decimal or less not selected
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
				if(g_base_mode < BASE_MODE_OCT) break;			//skip if octal or less not selected
			
		case '0':
		case '1':
				len = strlen((const char *)content);
				if(len < UI_MAX_TEXT) {
					content[len] = opcode;
					content[len +1] = 0;					//end of string
					if(content[0] == '0') {
						snprintf((char *)((ui_textbox *)lcd)->content, UI_MAX_TEXT, "%s", (const char *)content+1);			//remove 'padding 0'
					} else {
						snprintf((char *)((ui_textbox *)lcd)->content, UI_MAX_TEXT, "%s", (const char *)content);
					}
					ui_reinitialize_object(lcd);			//invalidate lcd display
				}
			break;
		case OPC_DOT:
				len = strlen((const char *)content);
				for (i=0;i<len;i++) {
					if(content[i] == '.') has_dot = TRUE;			//check if '.' already exist
				}
				if(has_dot) break;
				if(len < UI_MAX_TEXT) {
					content[len] = opcode;
					content[len +1] = 0;					//end of string
					ui_reinitialize_object(lcd);			//invalidate lcd display
				}
				break;
				
		case OPC_PLUSMIN:									//negative/plus sign
				len = strlen((const char *)content);
				if(content[0] == '-') {
					strcpy((char *)((ui_textbox *)lcd)->content, (const char *)(content + 1));			//clear minus sign
				} else {
					snprintf((char *)((ui_textbox *)lcd)->content, UI_MAX_TEXT, "-%s", content);			//print minus sign
				}
				ui_reinitialize_object(lcd);			//invalidate lcd display
				break;
		case OPC_CLR:				//only clear lcd
			snprintf((char *)((ui_textbox *)lcd)->content, UI_MAX_TEXT, "0");			//clear lcd
			ui_reinitialize_object(lcd);			//invalidate lcd display
			break;
		case OPC_2ND:
			g_function_index = (g_function_index + 1) % 2;
			calc_keypad1 = ui_get_object_by_name(lcd->display, "calc_keypad1");
			if(calc_keypad1 != NULL) ui_reinitialize_object(calc_keypad1);
			calc_keypad2 = ui_get_object_by_name(lcd->display, "calc_keypad1");
			if(calc_keypad2 != NULL) ui_reinitialize_object(calc_keypad2);
			break;
				
		case OPC_ANS:			//start calculation, release memory at the end of operation
			if(g_current_node != NULL) g_current_node->operand = (double)atoi((const char *)content);		//set last operand
			iterator = g_first_node;
			g_accumulator = 0;
			g_oprindex = 0;
			while(iterator != NULL) {
					//start decoding
					calc_decode(iterator);
					iterator = iterator->next;
			}
			calc_set_value(lcd, g_accumulator);			//view result on lcd
			calc_deinit();
			break;
	}
}

struct calc_function {
	uint16 code;
	char text[12];
} calc_function;

struct calc_keyset {
	uint16 x;
	uint16 y;
	uint16 w;
	uint16 h;
	uint32 color;
	struct calc_function function[2];
} calc_keyset;

struct calc_keyset_config {
	uint16 cols;
	uint16 rows;
	const struct calc_keyset * keyset;
} calc_keyset_config;

extern uint8_c calculator_appicon_png_48[4188];		//from power_vending_resources.h

const struct calc_keyset calc_keyset_1[] = {
	{ 0,0,1,1, UI_COLOR_ORANGE, {{OPC_HEX, "HEX"}, {OPC_HEX, "HEX"}} },
	{ 1,0,1,1, UI_COLOR_GREY, {{OPC_AND, "AND"}, {OPC_AND, "AND"}} },
	{ 2,0,1,1, UI_COLOR_GREY, {{OPC_SHL, " >>"}, {OPC_SHL, " >>"}} },
	{ 3,0,1,1, UI_COLOR_GREY, {{OPC_SHR, " <<"}, {OPC_SHR, " <<"}} },
	{ 4,0,1,1, UI_COLOR_GREY, {{OPC_PI, "Pi"}, {OPC_PI, "Pi"}} },
	{ 5,0,1,1, UI_COLOR_GREY, {{OPC_2ND, "2nd"}, {OPC_2ND, "2nd"}} },
	
	{ 0,1,1,1, UI_COLOR_ORANGE, {{OPC_DEC, "DEC"}, {OPC_DEC, "DEC"}} },
	{ 1,1,1,1, UI_COLOR_GREY, {{OPC_OR, "OR"}, {OPC_OR, "OR"}} },
	{ 2,1,1,1, UI_COLOR_GREY, {{OPC_SQR, "sqr"}, {OPC_SQR, "sqr"}}  },
	{ 3,1,1,1, UI_COLOR_GREY, {{OPC_SQRT, "sqrt"}, {OPC_SQRT, "sqrt"}}  },
	{ 4,1,1,1, UI_COLOR_GREY, {{OPC_POW, "pow"}, {OPC_POW, "pow"}}  },
	{ 5,1,1,1, UI_COLOR_GREY, {{OPC_CBRT, "cbrt"}, {OPC_CBRT, "cbrt"}}  },		//cubic root 
	
	{ 0,2,1,1, UI_COLOR_ORANGE, {{OPC_OCT, "OCT"}, {OPC_OCT, "OCT"}} },
	{ 1,2,1,1, UI_COLOR_GREY, {{OPC_XOR, "XOR"}, {OPC_XOR, "XOR"}} },
	{ 2,2,1,1, UI_COLOR_GREY, {{OPC_SIN, "sin"}, {OPC_ASIN, "asin"}} },
	{ 3,2,1,1, UI_COLOR_GREY, {{OPC_COS, "cos"}, {OPC_ACOS, "acos"}} },
	{ 4,2,1,1, UI_COLOR_GREY, {{OPC_TAN, "tan"}, {OPC_ATAN, "atan"}} },
	{ 5,2,1,1, UI_COLOR_GREY, {{OPC_LOG10, "log"}, {OPC_LOG2, "ln"}} },
	
	{ 0,3,1,1, UI_COLOR_ORANGE, {{OPC_BIN, "BIN"}, {OPC_BIN, "BIN"}} },
	{ 1,3,1,1, UI_COLOR_GREY, {{OPC_NOT, "NOT"}, {OPC_NOT, "NOT"}}  },
	{ 2,3,1,1, UI_COLOR_GREY, {{OPC_ABS, "ABS"}, {OPC_ABS, "ABS"}}  },			//absolute
	//{ 3,3,1,1, 'r' },
	{ 4,3,1,1, UI_COLOR_GREY, {{OPC_LBR, "("}, {OPC_LBR, "("}}  },
	{ 5,3,1,1, UI_COLOR_GREY, {{OPC_RBR, ")"}, {OPC_RBR, ")"}} },
	
	{ 0,0,0,0, UI_COLOR_GREY, {{NULL, ""}, {NULL, ""}} },				//end of iteration
};


const struct calc_keyset calc_keyset_2[] = {
	{ 0,0,1,1, UI_COLOR_GREY, {{OPC_DIV, "/"}, {OPC_DIV, "/"}} },
	{ 1,0,1,1, UI_COLOR_BLACK, {{0x37, "7"}, {0x37, "7"}} },
	{ 2,0,1,1, UI_COLOR_BLACK, {{0x38, "8"}, {0x38, "8"}} },
	{ 3,0,1,1, UI_COLOR_BLACK, {{0x39, "9"}, {0x39, "9"}} },
	{ 4,0,1,1, UI_COLOR_GREY, {{0x41, "A"}, {0x41, "A"}} },
	{ 5,0,1,1, UI_COLOR_GREY, {{0x45, "E"}, {0x45, "E"}} },
	
	{ 0,1,1,1, UI_COLOR_GREY, {{OPC_MUL, "x"}, {OPC_MUL, "x"}} },
	{ 1,1,1,1, UI_COLOR_BLACK, {{0x34, "4"}, {0x34, "4"}} },
	{ 2,1,1,1, UI_COLOR_BLACK, {{0x35, "5"}, {0x35, "5"}} },
	{ 3,1,1,1, UI_COLOR_BLACK, {{0x36, "6"}, {0x36, "6"}}  },
	{ 4,1,1,1, UI_COLOR_GREY, {{0x42, "B"}, {0x42, "B"}} },
	{ 5,1,1,1, UI_COLOR_GREY, {{0x46, "F"}, {0x46, "F"}}  },
	
	{ 0,2,1,1, UI_COLOR_GREY, {{OPC_SUB, "-"}, {OPC_SUB, "-"}} },
	{ 1,2,1,1, UI_COLOR_BLACK, {{0x31, "1"}, {0x31, "1"}} },
	{ 2,2,1,1, UI_COLOR_BLACK, {{0x32, "2"}, {0x32, "2"}} },
	{ 3,2,1,1, UI_COLOR_BLACK, {{0x33, "3"}, {0x33, "3"}} },
	{ 4,2,1,1, UI_COLOR_GREY, {{0x43, "C"}, {0x43, "C"}} },
	{ 5,2,1,1, UI_COLOR_ORANGE, {{OPC_CLR, "CLR"}, {OPC_CLR, "CLR"}} },
	
	{ 0,3,1,1, UI_COLOR_GREY, {{OPC_ADD, "+"}, {OPC_ADD, "+"}} },
	{ 1,3,1,1, UI_COLOR_BLACK, {{0x30, "0"}, {0x30, "0"}} },
	{ 2,3,1,1, UI_COLOR_BLACK, {{OPC_PLUSMIN, "+/-"}, {OPC_PLUSMIN, "+/-"}}  },
	{ 3,3,1,1, UI_COLOR_BLACK, {{OPC_DOT, "."}, {OPC_DOT, "."}}  },
	{ 4,3,1,1, UI_COLOR_GREY, {{0x44, "D"}, {0x44, "D"}} },
	{ 5,3,1,1, UI_COLOR_ORANGE, {{OPC_ANS, "ANS"}, {OPC_ANS, "ANS"}} },
	
	{ 0,0,0,0, UI_COLOR_GREY, {{NULL, ""}, {NULL, ""}} },				//end of iteration
};

const struct calc_keyset_config  g_calc_keysets[] = { 
		{ 6, 5, calc_keyset_1 }, 
		{ 6, 4, calc_keyset_2 }  
} ;

void calc_draw_button(gui_handle_p display, ui_rect * rect, uint8 mode, uint32 bgcolor) {
	uint16 x = ((ui_rect *)rect)->x;
	uint16 y = ((ui_rect *)rect)->y;
	uint16 w = ((ui_rect *)rect)->w;
	uint16 h = ((ui_rect *)rect)->h;
	ui_object temp1;//, temp2;
	memcpy(&temp1, rect, sizeof(ui_object));
	((ui_rect *)&temp1)->x += 2;
	((ui_rect *)&temp1)->y += 2;
	((ui_rect *)&temp1)->w -= 4;
	((ui_rect *)&temp1)->h -= 4;
	//memcpy(&temp2, &temp1, sizeof(ui_object));
	//((ui_rect *)&temp2)->x += ((ui_rect *)&temp2)->w - 13;
	//((ui_rect *)&temp2)->w = 13;
	//display->draw_rectangle(display, x + 2, y + 2, w - 4, h - 4, UI_COLOR_DARK_GREY, 2);
	//display->draw_rectangle(display, x + 3, y + 3, w - 6, h - 6, UI_COLOR_DARK_GREY, 0);
	if((mode & 0x0F) == UI_BUTTON_STATE_PRESSED) {
		//display->fill_area(display, UI_COLOR_RGB(5,56,127), display->set_area(display, x + 4, y + 3, w - 8, 1));
		display->fill_area(display, UI_COLOR_WHITE, display->set_area(display, x + 3, y + 4, w - 5, h - 6));
		display->fill_area(display, bgcolor, display->set_area(display, x + 4, (y + h) - 4, w - 8, 1));
		//ui_image_render(display, (ui_rect *)&temp1, (uint8 *)g_png_bt2, sizeof(g_png_bt2), UI_IMAGE_ALIGN_FILL);
		//ui_image_render(display, (ui_rect *)&temp2, (uint8 *)g_png_bt2, sizeof(g_png_bt2), UI_IMAGE_ALIGN_FILL);
	} else {
		//display->fill_area(display, UI_COLOR_WHITE, display->set_area(display, x + 4, y + 3, w - 8, 1));
		display->fill_area(display, bgcolor, display->set_area(display, x + 3, y + 4, w - 5, h - 6));
		//display->fill_area(display, bgcolor, display->set_area(display, x + 4, (y + h) - 4, w - 8, 1));
		//switch(mode & 0xF0) {
		//	case UI_BUTTON_STYLE_BLUE:
		//		ui_image_render(display, (ui_rect *)&temp1, (uint8 *)g_png_bt4, sizeof(g_png_bt4), UI_IMAGE_ALIGN_FILL);
		//		break;
		//	default: 
		//		ui_image_render(display, (ui_rect *)&temp1, (uint8 *)g_png_bt1, sizeof(g_png_bt1), UI_IMAGE_ALIGN_FILL);
		//		break;
		//}
	}
}


static void ui_calcpad_button_render(gui_handle_p display, uint16 x, uint16 y, uint16 w, uint16 h, uint16 state, uint16 code, char * text, uint32 bgcolor) {
	OS_DEBUG_ENTRY(ui_calcpad_button_render);
	uint16 nc, wtxt, xx, yy;
	//uint8 cbuf[5];
	uint32 color = UI_COLOR_WHITE;
	ui_object rect;
	//cbuf[0] = c;
	//cbuf[1] = 0;
	((ui_rect *)&rect)->x = x;
	((ui_rect *)&rect)->y = y;
	((ui_rect *)&rect)->w = w;
	((ui_rect *)&rect)->h = h;
	((ui_object *)&rect)->backcolor = UI_COLOR_BLACK;
	if(state & UI_STATE_INIT) {
		//display->fill_area(display, UI_COLOR_RGB(58,60,66), display->set_area(display, x + 1, y + 1, w - 2, h - 2));
		calc_draw_button(display, &rect, UI_BUTTON_STATE_NONE, bgcolor);
		goto start_render;
	}
	if(state & UI_STATE_KEYDOWN) {
		//display->fill_area(display, UI_COLOR_QUEEN_BLUE, display->set_area(display, x + 1, y + 1, w - 2, h - 2));
		//display->set_area(display, x, y, w, h);
		calc_draw_button(display, &rect, UI_BUTTON_STATE_PRESSED, bgcolor);
		goto start_render;
	}
	if(state & UI_STATE_KEYUP) {
		//display->fill_area(display, UI_COLOR_RGB(58,60,66), display->set_area(display, x + 1, y + 1, w - 2, h - 2));
		calc_draw_button(display, &rect, UI_BUTTON_STATE_NONE, bgcolor);
		goto start_render;
	}
	start_render:
	if(code < 0x100) {
		display->print_string(display, UI_FONT_DEFAULT, x + 8, y + (h / 2) - 8, (uint8 *)text, color);
	} else {
		display->print_string(display, UI_FONT_SMALL, x + 8, y + (h / 2) - 4, (uint8 *)text, color);
	}
	OS_DEBUG_EXIT();
}

static void ui_calcpad_render(ui_object * obj, gui_handle_p display) {
	OS_DEBUG_ENTRY(ui_calcpad_render);
	uint8 i,j,c;
	uint16 cx,cy;
	uint16 x = ((ui_rect *)obj)->x;
	uint16 y = ((ui_rect *)obj)->y;
	uint16 w = ((ui_rect *)obj)->w;
	uint16 h = ((ui_rect *)obj)->h;
	ui_object * calc_lcd;
	const struct calc_keyset_config keyset_cfg = g_calc_keysets[((ui_keyboard *)obj)->idx];
	const struct calc_keyset * keyset = keyset_cfg.keyset;
	uint16 bw = w / keyset_cfg.cols;
	uint16 bh = h / keyset_cfg.rows;
	struct calc_keyset * iterator = (struct calc_keyset *)keyset_cfg.keyset;
	uint8 index = 0;
	if(obj->state & UI_STATE_INIT) {
		while(iterator->function[g_function_index].code != NULL) {
			
			ui_calcpad_button_render(display, x + (iterator->x * bw), y + (iterator->y * bh), iterator->w * bw, iterator->h * bh, obj->state, iterator->function[g_function_index].code, iterator->function[g_function_index].text, iterator->color);
			iterator++;
			index++;
		}
	}
	iterator = (struct calc_keyset *)keyset;
	if(obj->state & UI_STATE_KEYDOWN && ((ui_keyboard *)obj)->duration == 0) {
		//if(obj->state & UI_STATE_KEYUP) return;
		if_touch_get(display, &cx, &cy);
		cy = cy - y;
		cx = cx - x;
		index = 0;
		while(iterator->function[g_function_index].code != NULL) {
			if(cx > (iterator->x * bw) && cx <= ((iterator->x * bw) + (iterator->w * bw))) {
				if(cy > (iterator->y * bh) && cy <= ((iterator->y * bh) + (iterator->h * bh))) {
					((ui_keyboard *)obj)->duration = UI_TEXTBOX_PRESS_DURATION;
					((ui_keyboard *)obj)->last_key = index;
					ui_calcpad_button_render(display, x + (iterator->x * bw), y + (iterator->y * bh), iterator->w * bw, iterator->h * bh, obj->state, iterator->function[g_function_index].code, iterator->function[g_function_index].text, iterator->color);
				}
			}
			index++;
			iterator++;
		}	
	}
	//key up event (duration based)
	if(((ui_keyboard *)obj)->duration != 0) {
		((ui_keyboard *)obj)->duration--;
		if(((ui_keyboard *)obj)->duration == 0) {
			//iterator = ((ui_keyboard *)obj)->last_key;
			iterator = &keyset[((ui_keyboard *)obj)->last_key];
			if(iterator != NULL) {
				if(iterator->function[g_function_index].code != NULL) {
					ui_calcpad_button_render(display, x + (iterator->x * bw), y + (iterator->y * bh), iterator->w * bw, iterator->h * bh, UI_STATE_KEYUP, iterator->function[g_function_index].code, iterator->function[g_function_index].text, iterator->color);	
				}
			}
		}
	}
	OS_DEBUG_EXIT();
}

static void ui_calcpad_button_click(ui_object * keyboard, void * params) {
	OS_DEBUG_ENTRY(ui_calcpad_button_click);
	uint8 i;
	ui_object * calc_lcd ;
	uint16 c;
	uint16 x = ((ui_rect *)keyboard)->x;
	uint16 y = ((ui_rect *)keyboard)->y;
	uint16 w = ((ui_rect *)keyboard)->w;
	uint16 h = ((ui_rect *)keyboard)->h;
	uint16 cx, cy;
	const struct calc_keyset_config keyset_cfg = g_calc_keysets[((ui_keyboard *)keyboard)->idx];
	const struct calc_keyset * keyset = keyset_cfg.keyset;
	struct calc_keyset * iterator = (struct calc_keyset *)keyset_cfg.keyset;
	uint16 bw = w / keyset_cfg.cols;
	uint16 bh = h / keyset_cfg.rows;
	ui_textbox * target = ((ui_textbox *)keyboard->target);
	uint8 charline = (((ui_rect *)target)->w / 8) - 1;			//number of character perline
	uint16 max_chars = ((((ui_rect *)target)->h / UI_FONT_DEFAULT) * charline) -1;
	if_touch_get(params, &cx, &cy);
	cy = cy - y;
	cx = cx - x;
	//c = keyset[((cy / bh) * 10) + (cx / bw)];
	while(iterator->function[g_function_index].code != NULL) {
		//ui_keyboard_button_render(display, x + (iterator->x * bw), y + (iterator->y * bh), iterator->w * bw, iterator->h * bh, obj->state, iterator->code);
		if(cx > (iterator->x * bw) && cx <= ((iterator->x * bw) + (iterator->w * bw))) {
			if(cy > (iterator->y * bh) && cy <= ((iterator->y * bh) + (iterator->h * bh))) {
				c = iterator->function[g_function_index].code;
				goto trigger_key_event;
			}
		}
		iterator++;
	}
	if(iterator->function[g_function_index].code != NULL) {
trigger_key_event:
		iterator = keyset;
		calc_lcd = ui_get_object_by_name(keyboard->display, "calc_lcd");
		calc_push_node(calc_lcd, c);
	}
	OS_DEBUG_EXIT();
}

ui_object * ui_calcpad_create(ui_object * target, uint16 x, uint16 y, uint16 w, uint16 h, uint8 keyset) {
	ui_keyboard * obj = (ui_keyboard *)ui_create_object(sizeof(ui_keyboard), UI_TYPE_LABEL | UI_ALIGN_FLOAT) ;
	((ui_object *)obj)->render = ui_calcpad_render;
	((ui_object *)obj)->rect.x = x;
	((ui_object *)obj)->rect.y = y;
	((ui_object *)obj)->rect.h = h;
	((ui_object *)obj)->rect.w = w;
	obj->idx = keyset;
	((ui_object *)obj)->target = target;
	((ui_object *)obj)->forecolor = UI_COLOR_WHITE;
	((ui_object *)obj)->backcolor = UI_COLOR_BLACK;
	((ui_object *)obj)->handler = ui_calcpad_button_click;
	return (ui_object *)obj;
}

static void calculator_exit_click(ui_object * obj, void * params) {
	OS_DEBUG_ENTRY(calculator_exit_click);
	tk_context_p ctx = (tk_context_p)obj->target;
	uint32 longtime = 0;
	ui_object * screen;
	ui_object * cfBtn;
	//tk_clear_subconfig_menu(ctx);
	calc_deinit();									//release memory
	ui_remove_screen(ctx->display, ui_get_screen_by_name(ctx->display, "calc"));
	//disable marker
	cfBtn = ui_get_object_by_name(ctx->display, "myCalc") ;
	if(cfBtn != NULL) {
		((ui_icon_tool *)cfBtn)->show_marker = FALSE;
	}
	//ui_clear_dispstack(ctx->display);
	//tk_clear_body(ctx);
	OS_DEBUG_EXIT();
}


static void calculator_show(tk_context_p ctx) {
	OS_DEBUG_ENTRY(calculator_show);
	ui_toggle * play;
	ui_datetime * dtime;
	datetime dval;
	ui_object * list;
	uint32 longtime;
	ui_object * obj;
	ui_object * calc_keypad1, * calc_keypad2;
	uint16 keypad_height = 0;
	uint16 y = 0;
	tk_config * conf = ctx->config;
	ui_object * lcd;
	//ui_clear_body(ctx->display, UI_COLOR_WHITE);
	//ui_clear_dispstack(ctx->display);
	//ui_clear_body(ctx->display, UI_COLOR_WHITE);
	obj = ui_get_object_by_name(ctx->display, "myCalc") ;
	if(obj != NULL) {
		((ui_icon_tool *)obj)->show_marker = TRUE;
	}
	obj = ui_get_object_by_name(ctx->display, "calc") ;
	if(obj != NULL) {
		ui_remove_screen(ctx->display, obj);
	}
	ui_object * screen = ui_push_screen(ctx->display, NULL);
	ui_set_object_name(screen, "calc");
	ui_set_text((ui_text *)screen, (uint8 *)"calc");
	ui_add_body(ctx->display, (lcd = ui_numbox_create((uint8 *)"", 40)));
	ui_set_object_name(lcd, "calc_lcd");
	ui_set_content(lcd, "0");
	y = screen->rect.y;
	y += lcd->rect.h + 8;
	keypad_height = screen->rect.h -  (lcd->rect.h + 8);
	
	if(((gui_handle_p)ctx->display)->orientation & 0x01) {
		ui_add_body(ctx->display, (calc_keypad1 = ui_calcpad_create(lcd, screen->rect.x, y, screen->rect.w, keypad_height / 2, 0)));
		ui_add_body(ctx->display, (calc_keypad2 = ui_calcpad_create(lcd, screen->rect.x, y + (keypad_height / 2), screen->rect.w, keypad_height / 2, 1)));
	} else {
		ui_add_body(ctx->display, (calc_keypad1 = ui_calcpad_create(lcd, screen->rect.x, y, screen->rect.w/2, keypad_height, 0)));
		ui_add_body(ctx->display, (calc_keypad2 = ui_calcpad_create(lcd, screen->rect.x + (screen->rect.w/2), y, screen->rect.w/2, keypad_height, 1)));
	}
	ui_set_object_name(calc_keypad1, "calc_keypad1");
	ui_set_object_name(calc_keypad2, "calc_keypad2");
	
	OS_DEBUG_EXIT();
}

static void calculator_click(ui_object * obj, void * params) {
	OS_DEBUG_ENTRY(calculator_click);
	tk_context_p ctx = (tk_context_p)obj->target;
	ui_object * instance;
	ui_textbox * textbox;
	uint16 ydiv = 0;
	uint8 tbuf[256];
	ui_object * screen;
	memset(tbuf, 0, sizeof(tbuf));
	if((screen = ui_get_screen_by_name(ctx->display, "calc")) != NULL) {
		if(((gui_handle_p)ctx->display)->body != screen) {
			calculator_exit_click(obj, params);
		} else {
			//ui_remove_screen_unsafe(ctx->display, screen);
			//screen = ui_push_screen(ctx->display, screen);
			//ui_set_object_name(screen, "calc");
			//ui_set_text((ui_text *)screen, (uint8 *)"calc");
		}
	} else {
			g_base_mode = BASE_MODE_DEC;
			g_function_index = 0;				//2nd function
			g_node_index = 0;
			g_current_node = NULL;
			g_first_node = NULL;
			g_oprindex = 0;
			g_accumulator = 0;
			calculator_show(ctx);
	}
	OS_DEBUG_EXIT();
	return;
}


void calculator_init(gui_handle_p handle, void * params) {
	OS_DEBUG_ENTRY(calculator_init);
	ui_object * calc = NULL;
	ui_object * header = handle->header;
	uint16 header_w;
	if(params != NULL && header != NULL) {
		header_w = header->rect.h / 4;
		ui_add_header(handle, (calc = ui_header_create((uint8 *)"myCalc", (uint8 *)"calc", header_w, header_w, sizeof(calculator_appicon_png_48), (uint8 *)calculator_appicon_png_48, calculator_click)));
		if(calc != NULL) ui_set_target(calc, params);
	}
	OS_DEBUG_EXIT();
}

void calculator_app_init(gui_handle_p handle, void * params, uint8 id) {
	OS_DEBUG_ENTRY(calculator_app_init);
	ui_object * calc = NULL;
	ui_object * body = handle->body;
	ui_object * obj;
	uint16 header_w = 64;
	if(params != NULL && body != NULL) {
		ui_add_body(handle, (calc = ui_launcher_create((uint8 *)"myCalc", (uint8 *)"calc", header_w, header_w, sizeof(calculator_appicon_png_48), (uint8 *)calculator_appicon_png_48, calculator_click)));
		if(calc != NULL) {
			obj = ui_get_object_by_name(handle, "calc") ;
			if(obj != NULL) {
				((ui_icon_tool *)calc)->show_marker = TRUE;
			}
			ui_set_target(calc, params);
		}
	}
	OS_DEBUG_EXIT();
}