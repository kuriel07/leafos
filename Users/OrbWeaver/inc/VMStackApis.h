/*!\file 			vmstackapis.h
 * \brief     	Orb-Weaver Stack Machine APIs
 * \details   	Orb-Weaver Stack Machine APIs provide all necessary function for stack scripting engine service
 * \author    	AGP
 * \version   	1.2
 * \date      	Created by Agus Purwanto on 10/28/14.
 * \pre       	
 * \bug       	
 * \warning   	
 * \copyright 	OrbLeaf Technology
\verbatim
1.0
 * initial release stack virtual machine, automatic garbage collection,	ASN.1 package decoder
 * support send sms, display text, select item, get input, get key STK APIs
1.1
 * added F2O, convert offset to object for translating offset (added 2015.04.09)
 * set_timer APIs, cross interpreter callback, get_inkey (added 2015.04.06) 
 * display_text return boolean value
 * added out of bounds exception during string concatenation (added 2015.04.15)
 * bug fixed : set_value api, retval = param causing unitended object linkage without incrementing refcount (2015.18.04)
 * added vrListScripts, vrLoadScript, vrInstallScript (added 2015.05.xx)
1.2
 * implemented: cross package call as in original stack engine (now orb-weaver, added 2015.05.16)
 * changed: vrTriggerEvent use vrLoadScript instead, more readable and reduced codesize (2015.05.xx)
 * framework: added va_toolkit_create, va_toolkit_push, va_toolkit_dispatch for creating custom proactive command (2015.05.23)
 * deprecated: previous toolkit APIs marked as deprecated, except for set_timer and send_sms (2015.05.23)
 * added: va_array API for creating array value from decimal, similar with dec2bin (2015.05.24)
 * optimized: framework APIs from local retval to g_pVaRetval (global pointer) (2015.06.09)
 * removed: removed vrListScripts, use vrLoadScript with VM_SCRIPT_LIST mode instead (2015.06.17) 
 * added: execution context for orblet behavior either as local script or WIB plugin during terminal result (2015.07.02)
 * modified: to support AkashicFS bootstraping feature (2015.12.21)
1.3
 * added: support for extended API 0xFE for syscall (2016.04.18)
 * changed: EFVar (2000/2200/6F0D) to transparent type, use bootstrapper to access wrap variable access (2016.04.18)
 * added: JSON apis for Orb-Weaver, ported from visual studio (2016.05.01)
 * removed: stack framework display, input, bip, location, refresh, polling, browser, tone, sms, ussd, setup call (2016.05.02)
 * changed: crypto APIs from operation based to context based (2016.05.15)
 * added: base64 codec for converter APIs (2016.05.15)
 * implemented: OBJNEW for creating class instance, vm_load_method modified to support class instance (2016.05.15)
 * changed: OBJDUP no longer supported, added VX_UNRESOLVED_CLASS exception (2016.05.16)
 * fixed: consistency between arg_serialize/deserialize with arg_array/object/create on orb framework, untested (2016.05.27)
 * modified: EXTCALL supporting arguments counter, EXTCALL0 to EXTCALL15 (2016.06.01)
 * changed: fixed memory leakage when calling external methods (2016.06.02)
 * changed: fixed arg_at and arg_get on OrbWeaver framework (2016.06.02)
 * added: ISO8583 APIs, iso_create, iso_push, iso_get (2016.06.03)
 * added: va_random (2016.06.08)
1.4
 * added: integration with OWB object, for integration with latest compiler (VM_OBJ_MAGIC, 2016.06.29)
 * changed: OBJSUB no longer supported
 * added: substring api, va_substr (2016.06.29)
 * removed: va_concat for concatenating string (2016.016.29)
 * fixed: arg_array when passing string data, non OWB object automatically converted to OWB object (2016.06.29, untested)
 * fixed: vm_garbage_collect when collecting vm_magic and vm_obj_magic (2016.07.06)
 * fixed: OW suspend state check during vm_exec_local_script, to enable resist multiple terminal response (2016.08.02)
 * fixed: va_arg_at and va_arg_array support for ASN_TAG_OCTSTRING, include DES3 mode, va_crypto_create algo param include break (2017.03.31)
 * fixed: function parameter callback va_crypto_do alignment memory error when issuing LDRB using local container (pctx) (2017.03.31)
 * added: TCP/IP for network interface, va_net_open, va_net_send, va_net_recv, va_net_close (2017.06.13)
 * modified: com/RS232, net/TCP/IP operation write, read and close wrapped within fread, fwrite, fclose, reuseability (2017.06.13)
 * added: PICC APIs for vm_framework, va_picc_open, va_picc_read, va_picc_write, va_picc_close (2017.06.17)
 * fixed: OWB operation to use delimiters (VA_OBJECT_DELIMITER) from previous colon (2017.07.28)
 * changed: F2O to return vm_function (include argument count and handle) instead of plain integer offset (2018.01.10)
 * added: vm_exec_function for executing vm_function from syscall 31 (2018.01.10)
 * changed: va_net operation (open,write,read,close) to asynchronous, added SSL support (2018.02.18)
 * added : exception divide by zero (2018.04.17)
1.5
 * changed : OBJCONST now load data in extended LV mode supporting 65535 bytes of data (2018.05.01)
 * added : variable APIs and context APIs, changed magic number (2018.05.05)
 * modified : set_var and get_var API and params (backward compatibility), support for this() _syscall_0 context operation (2018.05.06)
 * modified : garbage collection also shift vm_context->vars variable for VM_CTX_MAGIC object (2018.05.06)
 * modified : vm_magic now 0x80 to support variety data, added VM_PCTX_MAGIC for pointer to context (2018.05.22)
1.6
* modified : added support for multithreaded VM through instantiation (2018.09.29)
* added : support for extension and logical operation (2019.01.31)
\endverbatim
 */
 					
#ifndef _DEFS__H
#include "..\..\defs.h"
#endif
#ifndef _CONFIG__H
#include "..\..\config.h"
#endif	
#ifndef TK_APIS__H
#include "..\..\toolkit\inc\tk_apis.h"
#endif
#include "..\inc\va_trap_apis.h"
#ifndef _VM_STACK__H  

#define VM_MAJOR_VERSION		1
#define VM_MINOR_VERSION		6
#define VM_STACK_VERSION		((VM_MAJOR_VERSION << 4) | VM_MINOR_VERSION)

#ifndef VM_MAX_STACK_SIZE
#define VM_MAX_STACK_SIZE 		64
#endif

#define VM_MAX_VAR_NAME			16
#define VA_OBJECT_MAX_SIZE			0x1000

#define FID_STACK					0x2000
#define FID_STACK_LOCAL			0x2200
#define FID_STACK_CONFIG		0x4F0C
#define FID_STACK_DATA			0x4F0D
#define FID_STACK_EXCEPTION	0x4F0E
#define FID_STACK_APIC			0x4FAC		//configuration
#define FID_STACK_CERT			0x4FAD		//data
	
//use ASN.1 tag values, compatibility purpose
#define ASN_TAG_CONSTRUCTED		0x20

#define ASN_TAG_RSV				0	//reserved
#define ASN_TAG_BOOL			1	//BOOLEAN
#define ASN_TAG_INTEGER			2	//INTEGER
#define ASN_TAG_BSTRING			3	//BIT STRING
#define ASN_TAG_OCTSTRING		4	//OCTET STRING
#define ASN_TAG_NULL			5	//NULL
#define ASN_TAG_OBJID			6	//OBJECT IDENTIFIER
#define ASN_TAG_OBJDESC			7	//ObjectDescriptor
#define ASN_TAG_EXTERNAL		8	//INSTANCE OF, EXTERNAL
#define ASN_TAG_REAL			9	//REAL
#define ASN_TAG_ENUM			10	//ENUMERATED
#define ASN_TAG_EMBEDDED		11	//EMBEDDED PDV
#define ASN_TAG_UTF8			12	//UTF8String
#define ASN_TAG_ROID			13	//RELATIVE-OID
#define ASN_TAG_SEQ				16	//SEQUENCE, SEQUENCE OF
#define ASN_TAG_SET				17	//SET, SET OF
#define ASN_TAG_NUMSTRING		18	//NumericString
#define ASN_TAG_PRINTSTRING		19	//PrintableString
#define ASN_TAG_TELETEX			20	//TeletexString, T61String
#define ASN_TAG_VIDEOTEX		21	//VideotexString
#define ASN_TAG_IA5STRING		22	//IA5String
#define ASN_TAG_UTCTIME			23	//UTCTime
#define ASN_TAG_GENTIME			24	//GeneralizedTime
#define ASN_TAG_GRAPHSTRING		25	//GraphicString
#define ASN_TAG_ISO646			26	//VisibleString, ISO646String
#define ASN_TAG_GENSTRING		27	//GeneralString
#define ASN_TAG_UNISTRING		28	//UniversalString
#define ASN_TAG_CHSTRING		29	//CHARACTER STRING
#define ASN_TAG_BMPSTRING		30	//BMPString

#define PK_TAG_CLASS			0xCA
#define PK_TAG_METHOD			0xE0
#define PK_TAG_PROPERTY			0xA0

typedef struct pk_object {
	uchar tag;
	void * codebase;
	void * next;
} pk_object; 

typedef struct pk_class {
	pk_object base;
	uchar name[256];
	void * properties;
	void * codebase;
} pk_class;

typedef struct pk_method {
	void * handle;	
	uint16 offset;				//offset relative to class module
	//pk_object base;
	//pk_class * parent;
	//uchar name[256];
	//uchar numargs; 
} pk_method;

typedef struct pk_property {
	pk_object base;
	uchar name[256];
	uint16 offset;				//global offset relative to class
} pk_property;

typedef struct vf_handle {
	BSAllocHandle base;
	uint16 size;
	uint16 next_offset;		//offset of current iterator
	uint16 file_offset;
	uint16 file_size;
} vf_handle;

typedef struct vf_handle * vf_handle_p;

//#define MAX_BUFFER_SIZE		65536
//.db	-> data byte
//.dw	-> data word

#define INS_NOP			0	//-> no operation 		(pseudo instruction)
#define INS_SCTX		8	//-> save context for variables (function call)
#define INS_RCTX		9	//-> restore context memory (function return)
#define INS_OBJCONST	16	//-> load constant to stack
#define INS_OBJNEW		17	//-> allocate new instance
#define INS_OBJDUP		18	//-> duplicate stack
#define INS_OBJDEL		19	//-> delete (relative to sp, remove current object from stack)
#define INS_OBJSZ		22	//-> size of object on stack
#define INS_OBJSUB		23	//-> explode object (relative to sp)

#define INS_OBJPUSH		25	//-> push variable to stack		
#define INS_OBJPOP		26	//-> pop variable from stack
#define INS_OBJSTORE		29	//-> store variable from stack

//stack operation
#define INS_ADD		32	//-> addition (relative to sp)
#define INS_SUB		33	//-> subtract (relative to sp)
#define INS_MUL		34	//-> multiplication
#define INS_DIV		35	//-> division
#define INS_MOD		36	//-> modulus
#define INS_AND		40	//-> and operation
#define INS_OR		41	//-> or operation
#define INS_XOR		42	//-> xor operation
#define INS_NOT		43	//-> not operation

#define INS_END		60	//-> end function			(pseudo instruction)
#define INS_FUNC	61	//-> create function		(pseudo instruction)
#define INS_LBL		62	//-> create label			(pseudo instruction)
#define INS_CREQ		64	//-> jump if equal (relative to pc)
#define INS_CRNE			65	//-> jump not equal (relative to pc)
#define INS_CRGT			66	//-> jump greater than (relative to pc)
#define INS_CRLT			67	//-> jump less than (relative to pc)
#define INS_CRGTEQ			68	//-> jump greater than (relative to pc)
#define INS_CRLTEQ			69	//-> jump less than (relative to pc)
#define INS_JMP				70	//-> jump unconditional (relative to pc)
#define INS_JFALSE			71	//-> jump if false
#define INS_JTRUE			72	//-> jump if true
#define INS_SWITCH			74	//-> switch

#define INS_F2O_0			96	//-> function to object
#define INS_F2O_1			97	//-> function to object
#define INS_F2O_2			98	//-> function to object
#define INS_F2O_3			99	//-> function to object
#define INS_F2O_4			100	//-> function to object
#define INS_F2O_5			101	//-> function to object
#define INS_F2O_6			102	//-> function to object
#define INS_F2O_7			103	//-> function to object
#define INS_F2O_8			104	//-> function to object
#define INS_F2O_9			105	//-> function to object
#define INS_F2O_10			106	//-> function to object
#define INS_F2O_11			107	//-> function to object
#define INS_F2O_12			108	//-> function to object
#define INS_F2O_13			109	//-> function to object
#define INS_F2O_14			110	//-> function to object
#define INS_F2O_15			111	//-> function to object

#define INS_RET		126	//-> finished operation
#define INS_CALL		127
#define INS_SYSCALL		128
#define INS_SYSCALL0	128
#define INS_SYSCALL1	129
#define INS_SYSCALL2	130
#define INS_SYSCALL3	131
#define INS_SYSCALL4	132
#define INS_SYSCALL5	133
#define INS_SYSCALL6	134
#define INS_SYSCALL7	135
#define INS_SYSCALL8	136
#define INS_SYSCALL9	137
#define INS_SYSCALL10	138
#define INS_SYSCALL11	139
#define INS_SYSCALL12	140
#define INS_SYSCALL13	141
#define INS_SYSCALL14	142
#define INS_SYSCALL15	143

#define INS_EXTCALL		160
#define INS_EXTCALL0	160
#define INS_EXTCALL1	161
#define INS_EXTCALL2	162
#define INS_EXTCALL3	163
#define INS_EXTCALL4	164
#define INS_EXTCALL5	165
#define INS_EXTCALL6	166
#define INS_EXTCALL7	167
#define INS_EXTCALL8	168
#define INS_EXTCALL9	169
#define INS_EXTCALL10	170
#define INS_EXTCALL11	171
#define INS_EXTCALL12	172
#define INS_EXTCALL13	173
#define INS_EXTCALL14	174
#define INS_EXTCALL15	175

//virtual machine exception						 
#define VX_SYSTEM_EXCEPTION				0 
#define VX_UNIMPLEMENTED_APIS			1
#define VX_UNKNOWN_INSTRUCTION			2
#define VX_STACK_OVERFLOW				3 
#define VX_INSUFFICIENT_HEAP	 		4
#define VX_STACK_UNDERFLOW				5
#define VX_OUT_OF_BOUNDS				6
#define VX_UNRESOLVED_CLASS				7
#define VX_UNRESOLVED_METHOD			8
#define VX_ARGUMENT_MISMATCH			9
#define VX_INVALID_CONTEXT				10
#define VX_DIV_BY_ZERO						11

//.db	-> data byte
//.dw	-> data word
#define VM_STATE_INIT		0x00
#define VM_STATE_RUN 		0x01
#define VM_STATE_SUSPEND	0x03
#define VM_STATE_EXCEPTION	0x04
#define VM_STATE_ABORT		0x07

#define VM_MAGIC					0x80
#define VM_OBJ_MAGIC				0xC0		//object magic number (owb)
#define VM_PCTX_MAGIC				0xE0		//vm_context * magic
#define VM_CTX_MAGIC				0xF0		//vm_context magic (push during function call, pop on ret)
#define VM_EXT_MAGIC				0xD0		//custom operand
#define VM_MAGIC_MASK_ZERO			(VM_MAGIC | 0x0F)
#define VM_MAGIC_MASK				0xF0

#define VM_HANDLE_SIZE		3	//((sizeof(FSHandle) / sizeof(vm_object *)) + 1)

#define VM_CMP_EQ			0x10
#define VM_CMP_GT			0x20
#define VM_CMP_LT			0x40

/* query type */
#define VM_SCRIPT_BY_METHOD			0x04
#define VM_SCRIPT_BY_CLASS			0x03
#define VM_SCRIPT_BY_EVENT			0x02
#define VM_SCRIPT_BY_ALIAS			0x01
#define VM_SCRIPT_BY_INDEX			0x10
#define VM_SCRIPT_ACTIVE_ONLY		0x40
#define VM_SCRIPT_LIST				0x80

#define VM_EVENT_STARTUP			0x80
#define VM_EVENT_PICC_IN			0x90
#define VM_EVENT_IO					0x98
#define VM_EVENT_UART				0x9C

/* stack execution context */
#define VEC_ORB_LOCAL				0x01
#define VEC_WIB_PLUGIN				0x02

#define VA_ORC_API_TAG				0x96			//api key and configuration
#define VA_ORC_CERT_TAG				0x9C			//certificate file DER
#define VA_ORC_KEYS_TAG				0x9B			//keys file DER
#define VA_ORC_ICO_TAG				0x99			//icon file can be PNG or OCI

typedef struct vm_object {
	uint8 mgc_refcount;		//4(high) magic, 4(low) refcounter
	uint16 len;
	uint8 bytes[1];
} vm_object;

typedef struct vm_variable {
	uint8 mgc;
	struct vm_variable * next;
	uint8 name[VM_MAX_VAR_NAME];
	uint16 len;
	uint8 bytes[1];
} vm_variable;

typedef struct sys_context {
	uint8 num_of_params;
} sys_context;

typedef struct vm_context {
 	vf_handle handle;
	vm_variable ** vars;
	uint16 offset;
	vm_variable * var_root;
	void (* set)(void *);
	void (* get)(void *);
	void (* remove)(void *);
} vm_context;


typedef struct vm_instance {
	vm_context base;
	uint16 sp;		//stack pointer
	uint16 bp;		//base pointer
	uint16 pc;
	void * base_address;
	void * vm_stacks[VM_MAX_STACK_SIZE];
	uchar vm_state;
	uint8 exec_vector;			//execution context (unused)
	uchar recursive_exec;		//set recursive_exec flag to false
	uint16 current_api;
	sys_context sysc;
	vm_object * ret;
	uchar name[OS_TASK_MAX_NAME_SIZE];
	void * gui_ctx;
	void * picc_ctx;
} vm_instance;

typedef struct vm_api_entry {
	uint8 id;
 	void (* entry)(vm_instance *); 
 	vm_object * (* exit)(uint8, uint8 *);
} vm_api_entry;

typedef struct vm_function {
	vm_context base;
	uint8 arg_count;
	uchar name[OS_TASK_MAX_NAME_SIZE];
} vm_function;

typedef struct vm_execution_context {
 	uint8 mode;						//VEC_ORB_LOCAL, VEC_WIB_PLUGIN
	uint8 (* entry)(BYTE);
} vm_execution_context;

typedef struct vm_loader_context {
	void * base;
	BYTE hlen;
	BYTE hbuf[4];
	BYTE tag;
	WORD length;
	WORD offset;
} vm_loader_context;

typedef struct vm_custom_opcode {
	uint16 (* text)(vm_object *, uint8 * buffer);
	vm_object * (* add)(vm_object *, vm_object *);
	vm_object * (* mul)(vm_object *, vm_object *);
	vm_object * (* div)(vm_object *, vm_object *);
	vm_object * (* sub)(vm_object *, vm_object *);
	//logical operation
	vm_object * (* and)(vm_object *, vm_object *);
	vm_object * (* or)(vm_object *, vm_object *);
	vm_object * (* xor)(vm_object *, vm_object *);
	vm_object * (* not)(vm_object *);
} vm_custom_opcode;

typedef struct vm_extension {
	uint8 tag;
	vm_custom_opcode * apis;
	uint8 payload[0];
} vm_extension;

typedef sys_context * sys_handle;

#define VM_SET_ARG(x)		vm_instance * ctx = x
#define VM_DEF_ARG			vm_instance * ctx
#define VM_ARG				ctx
#define vm_push_stack(x)	ctx->vm_stacks[ctx->sp--] = x
#define vm_pop_stack()		ctx->vm_stacks[++ctx->sp]
#define vm_push_base(x) 	ctx->vm_stacks[ctx->bp++] = x
#define vm_set_stack(x)		ctx->vm_stacks[ctx->sp] = x
#define vm_get_stack(x)		ctx->vm_stacks[ctx->sp]
#define vm_push_base(x)		ctx->vm_stacks[ctx->bp++] = x
#define vm_pop_base()		ctx->vm_stacks[--ctx->bp]
#define vm_get_base(i)		ctx->vm_stacks[ctx->bp - i]
#define vm_set_base(i,x)	ctx->vm_stacks[ctx->bp - i] = x
#define vm_stack(x)			ctx->vm_stacks[x]
#define vm_get_sp()			ctx->sp
#define vm_set_sp(x)		ctx->sp = x
#define vm_get_pc()			ctx->pc
#define vm_set_pc(x)		ctx->pc = x
#define vm_add_pc(x)		ctx->pc += x
#define vm_get_bp()			ctx->bp
#define vm_set_bp(x)		ctx->bp = x
#define vm_get_package()	&ctx->base
#define vm_set_package(x)	memcpy(&ctx->base, x, sizeof(vm_context))
#define vm_get_state()		ctx->vm_state
#define vm_get_sysc()		ctx->sysc
#define vm_get_rexec()		ctx->recursive_exec
#define vm_set_rexec(x)		ctx->recursive_exec = x
#define vm_get_context()	ctx->base
#define vm_get_cur_api()	ctx->current_api
#define vm_set_cur_api(x)	ctx->current_api = x
#define vm_get_retval()		ctx->ret
#define vm_set_retval(x)	ctx->ret = x
#define vm_get_vars()		ctx->base.vars
#define vm_set_vars(x)		ctx->base.vars = x
#define vm_get_gui(__TYPE__) 	((__TYPE__)ctx->gui_ctx)
#define vm_get_picc(__TYPE__)	((__TYPE__)ctx->picc_ctx)

#define VPP_TAG_CONFIG			0x96 
#define VPP_UID_LENGTH			0x08 
#define VPP_MAX_VEND_LENGTH		0x07
#define VPP_MAX_NAME_LENGTH		0x0A		//maximum name length for vm_api_config

#define VPP_SEC_SSL			0x40		//require terminal for SSL
#define VPP_SEC_S1			0x10		//use secure session
#define VPP_SEC_AUTH		0x01		//require terminal authentication (secure terminal)
#define VPP_CFG_UPDT		0x01		//enable auto-update
#define VPP_CFG_USR			0x02		//use user confirmation (auto-update)
#define VPP_CFG_CLS			0x80		//support contactless communication

typedef _ALIGN1_ struct vm_api_config {
	uint8 	owver; 								//minimum orbweaver version
	uint8 	ver;								//application version
 	uint8 	api_level;							//current api configuration
	uint8 	security;							//security requirement
	uint8 	config;								//card-terminal configuration
	uint8 	vlen;								//vendor length
	uint8 	vendor[VPP_MAX_VEND_LENGTH];		//vendor identifier		
	uint8 	uid[VPP_UID_LENGTH];				//unique application identifier
	uint8 	plen;								//current appname length
	uint8	pname[VPP_MAX_NAME_LENGTH];			//appname
} vm_api_config;

#ifndef VM_NULL_OBJECT
#define VM_NULL_OBJECT		(vm_object *)"\xe0\x00\x00"
#endif

#define VM_TRUE_OBJECT		"true"
#define VM_FALSE_OBJECT		"false"
#define vm_strlen(x)				strlen(x)

//internal basic APIs
uint8 vm_is_running(VM_DEF_ARG) _REENTRANT_;
vm_object * vm_pop_stack_arc(VM_DEF_ARG) _REENTRANT_ ;
vm_object * vm_load_bool(BYTE value) _REENTRANT_ ;
vm_object * vm_load_constant(VM_DEF_ARG, WORD offset) _REENTRANT_ ;
vm_object * vm_create_object(uint16 length, void * buffer) _REENTRANT_ ;
vm_object * vm_create_context(vm_context * vctx);
vm_object * vm_create_extension(uint8 tag, vm_custom_opcode * apis, uint16 length, uchar * bytes);
uint8 vm_object_get_type(vm_object * obj);
uint16 vm_object_get_text(vm_object * obj, uint8 * text);
uint16 vm_object_get_length(vm_object * obj);
uint8 vm_ext_get_tag(vm_object * obj);
void vm_release_object(vm_object * obj) _REENTRANT_ ;
vm_object * vm_size_object(vm_object * op1) _REENTRANT_ ;
vm_object * vm_split_object(vm_object * op2, vm_object * op1, vm_object * target) _REENTRANT_ ;
vm_object * vm_operation_object(VM_DEF_ARG, uchar opcode, vm_object * op2, vm_object * op1) _REENTRANT_ ;
uint8 vm_is_numeric(uchar * buffer, uchar len) _REENTRANT_ ;
uint8 vm_cmp_object(vm_object * op2, vm_object * op1) _REENTRANT_ ;
vm_object * vm_load_class(VM_DEF_ARG, vm_object * clsobj) _REENTRANT_ ;
vm_object * vm_load_method(vm_object * clsins, vm_object * mthobj) _REENTRANT_ ;
void vm_push_argument(VM_DEF_ARG, uint8 size, uint8 * buffer) _REENTRANT_ ;
void vm_push_argument_object(VM_DEF_ARG, vm_object * obj) _REENTRANT_;
BYTE vm_pop_result(VM_DEF_ARG, BYTE max_len, uint8 * buffer) _REENTRANT_ ;
void vm_set_state(VM_DEF_ARG, BYTE state) _REENTRANT_ ;

void vm_garbage_collect(VM_DEF_ARG) _REENTRANT_ ;												//perform garbage collection (ref counting) with copying collector
void vm_update_mutator(VM_DEF_ARG, void * old_addr, void * new_addr) _REENTRANT_ ;			//replace mutator pre-allocated address with new allocated address

//vm ports apis
vm_instance * vm_new_instance(uchar * name);
void vm_release_instance(VM_DEF_ARG);
void vm_exec_instance(vm_instance * instance);
extern uint8 vm_init(VM_DEF_ARG, vf_handle_p handle, uint16 offset) _REENTRANT_ ;
extern uint16 vm_fetch(VM_DEF_ARG, uint32 offset, uint8 * buffer, uint16 size) _REENTRANT_ ;
extern void vm_close(VM_DEF_ARG) _REENTRANT_ ;
void vm_abort(VM_DEF_ARG);
uint8 vm_is_aborted(VM_DEF_ARG);

void vm_decode(VM_DEF_ARG) _REENTRANT_ ;

//syscall apis
void vm_invoke_exception(VM_DEF_ARG, BYTE excp) _REENTRANT_ ;
vm_object * vm_get_argument(VM_DEF_ARG, BYTE index) _REENTRANT_ ;
uchar vm_get_argument_count(VM_DEF_ARG) _REENTRANT_ ;
vm_object * vm_syscall(VM_DEF_ARG, uint16 api_id) _REENTRANT_ ;				//external hook function
vm_object * va_create_ext_float(uint16 length, uint8 * buffer);
void vm_syscall_ret(VM_DEF_ARG, BYTE size, BYTE * buffer) _REENTRANT_ ;
void va_init_context(tk_context_p ctx);		//called one time during context initialization
void va_exec_init(VM_DEF_ARG);			//called one time before any execution
void va_exec_release(VM_DEF_ARG);		//called one time after any execution
void vm_exec_function(VM_DEF_ARG, vm_function * func);
BYTE va_exec_context(VM_DEF_ARG, BYTE result) _REENTRANT_ ;				//hook to application context handler

WORD vrTriggerEvent(BYTE eventid, BYTE size, BYTE * buffer) _REENTRANT_ ;
BYTE vrExitResult(BYTE max_len, BYTE * buffer) _REENTRANT_ ;
BYTE vrSelectForUserEvent(BYTE * tar) _REENTRANT_ ;
BYTE vrSelectForExecution(BYTE * tar) _REENTRANT_ ;
BYTE vrConvertText(BYTE * text, BYTE length, BYTE * buffer, BYTE bufsize) _REENTRANT_ ; 
BYTE vrConvertTextDcs(BYTE * text, BYTE length, BYTE * buffer, BYTE bufsize) _REENTRANT_ ;
//BYTE vrInstallScript(vf_handle_p handle, WORD len, BYTE * buffer) _REENTRANT_;

//runtime APIs
void vr_init_global(tk_context_p ctx);
uint16 vr_load_script(uint8 mode, vf_handle_p handle, uint8 len, uint8 * buffer) _REENTRANT_ ;
uint8 vr_init_handle(vf_handle_p handle, uint8 api, uint32 address, uint16 size);
//runtime file APIs
void vf_first_handle(vf_handle_p handle);
uint8 vf_next_handle(vf_handle_p handle, uint8 * tag, uint16 * size);
uint16 vf_read_handle(vf_handle_p handle, uint16 offset, uint8 * buffer, uint16 size);
uint8 vf_pop_handle(vf_handle_p handle, uint16 offset, uint8 * tag, uint16 * size);

uint8 vm_exec_local_script(VM_DEF_ARG, uint8 result) _REENTRANT_ ;
#if STACK_WIB_APIS
uint8 vm_exec_wib_plugin(uint8 result) _REENTRANT_ ;
#endif

//util APIs
void vm_memcpy(void * dst, void * src, uint16 size) _REENTRANT_;
void vm_memset(void * buf, uint8 val, uint16 size) _REENTRANT_;
uint8 vm_memcmp(void * op1, void * op2, uint16 size) _REENTRANT_ ;
uint8 vm_imemcmp(void * op1, void * op2, uint16 size) _REENTRANT_;		//compare memory insensitive


//variable and context APIs
vm_variable * vm_variable_new(vm_variable ** root, uint8 type, uint16 length, uint8 * bytes);
void vm_variable_release(vm_variable ** root, vm_variable * var);
void vm_variable_clear(vm_variable ** root) ;

#define _VM_STACK__H
//#else
//#ifndef VM_STACK_VERSION
//#define VM_STACK_VERSION		0x00
//#endif
#endif
