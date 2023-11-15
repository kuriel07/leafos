#include "..\..\defs.h"
#include "..\..\config.h"
#include "..\inc\MMApis.h"	
#include "..\inc\VMStackApis.h"	
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "..\toolkit\inc\tk_apis.h"

#define IS_REL_JUMP_ADDRESS		1 			//use relative jump on intermediate streamer

//#if TK_STACK_ENABLED									
#ifndef VM_GC_DEBUG
#define VM_GC_DEBUG				0
#endif

//static WORD _sp = VM_MAX_STACK_SIZE -1;		//stack pointer
//static WORD _bp = 0;		//base pointer
//static WORD _pc = 0;
//static BYTE _bp_start = 0;
//static void * _base_address = NULL;
//vm_object * _vm_stacks[VM_MAX_STACK_SIZE];

//vm_context _vm_file;
//BYTE _vm_state = VM_STATE_INIT;	
//sys_context g_sVaSysc; 
//BYTE g_vmExecContext = VEC_ORB_LOCAL;
//vm_object * g_pVaRetval;
//uint8 g_recursive_exec = FALSE;
//vm_object _vm_null = (vm_object){ .mgc_refcount = 0xe0, .len = 0, bytes={ 0 } };
//vm_object * _vm_null = "\xe0\x00\x00";

vm_instance * vm_new_instance(uchar * name) {
	vm_instance * instance = os_alloc(sizeof(vm_instance));
	if(instance != NULL) {
		memset(instance, 0, sizeof(vm_instance));
		instance->sp = VM_MAX_STACK_SIZE - 1;
		instance->exec_vector = VEC_ORB_LOCAL;
		instance->vm_state = VM_STATE_INIT;
		strncpy((char *)instance->name, (const char *)name, OS_TASK_MAX_NAME_SIZE);
		va_exec_init(instance);
	}
	return instance;
}

void vm_release_instance(VM_DEF_ARG) {
	if(VM_ARG != NULL) {
		va_exec_release(VM_ARG);
		os_free(VM_ARG);
	}
}

static void vm_exec_task() {
	VM_SET_ARG(os_get_context());
	while(VM_ARG != NULL) {
		VM_ARG = os_get_context();
		//start execute
		//do {
		vm_decode(VM_ARG);
		//} 
		//while(vm_is_running(VM_ARG));
		//check for returning state
		switch(vm_get_state()) {
			//suspend task and wait for event
			case VM_STATE_SUSPEND: os_suspend(); break;
			//release instance and terminate task
			case VM_STATE_EXCEPTION:
			case VM_STATE_ABORT:
				vm_close(VM_ARG);
				vm_release_instance(VM_ARG);
				os_get_active_task()->context = NULL;		//no instance
				goto vm_exit_task;
			//release instance but don't terminate task
			case VM_STATE_RUN:
				vm_set_state(VM_ARG, VM_STATE_INIT);
			case VM_STATE_INIT:
				vm_close(VM_ARG);
				vm_release_instance(VM_ARG);
				os_get_active_task()->context = NULL;		//no instance
				os_suspend();
				break;
		}
	}
	vm_exit_task:
	os_kill_active_task();
	while(1) ;
}

void vm_exec_instance(vm_instance * instance) {
	os_task * task = NULL;
	VM_SET_ARG(instance);
	task = os_find_task_by_name((const char *)instance->name);
	if(task != NULL) {
		if(task->context != NULL) {
			//vm_close(task->context);
			vm_release_instance(task->context);		//release previous instance
		}
		task->context = instance;				//set to new instance
		os_resume(task);
	} else {
		os_create_task(instance, vm_exec_task, (const char *)instance->name, 0xeff0, 2048 + VA_OBJECT_MAX_SIZE);
	}
}

BYTE vm_init(VM_DEF_ARG, vf_handle_p handle, WORD offset) _REENTRANT_ {		//original input file path as parameter
	vm_context vctx;
	vm_object * obj;
	if(vm_get_state() == VM_STATE_EXCEPTION) vm_set_state(VM_ARG, VM_STATE_INIT);		//from exception, set state to init
	if(vm_get_state() != VM_STATE_INIT) return -1;				//check VM_STATE if not ready return init failed
	va_exec_init(VM_ARG);			//initialize framework for execution
	//memset(_vm_stacks, 0, sizeof(_vm_stacks));
	vm_memcpy(&vctx, handle, sizeof(vf_handle));
	vctx.var_root = NULL;
	vctx.vars = vm_get_package().var_root;
	vctx.offset = 0xFFFF;
	vm_set_package(&vctx);
	//g_vmExecContext = VEC_ORB_LOCAL;				//execution context = local orb script (default)
	vm_set_pc(offset);
	vm_set_bp(0);
	//start garbage collect
	vm_garbage_collect(VM_ARG);
	vm_push_base(obj = vm_create_context(&vctx));
	vm_set_vars(&((vm_context *)obj->bytes)->var_root);
	return 0;
}

uint16 vm_fetch(VM_DEF_ARG, uint32 offset, uint8 * buffer, uint16 size) _REENTRANT_ {
	uint16 readed = 0;
	vf_handle_p handle = (vf_handle_p)VM_ARG;
	size = vf_read_handle(handle, handle->file_offset + offset, buffer, size);
	return size;
}

uint8 vm_is_running(VM_DEF_ARG) _REENTRANT_ {
	return (vm_get_state() != VM_STATE_INIT);
}

uint8 vm_is_aborted(VM_DEF_ARG) {
	return (vm_get_state() == VM_STATE_ABORT);
}

void vm_abort(VM_DEF_ARG) {
	if(VM_ARG == NULL) return;
	vm_get_state() = VM_STATE_ABORT;
}

void vm_close(VM_DEF_ARG) _REENTRANT_ {
	vm_object * obj;
	if(VM_ARG == NULL) return;
	vm_set_pc(0xFFFF);
	if(vm_get_state() == VM_STATE_INIT) return;
	//popping stack				   
	while(vm_get_sp() < VM_MAX_STACK_SIZE) {
	 	obj = vm_pop_stack();
		if(obj == ((vm_object *)VM_NULL_OBJECT)) continue;
		if((obj->mgc_refcount & VM_MAGIC) == VM_MAGIC) obj->mgc_refcount &= 0xF0;		//clear reference counter
	}  
	vm_set_sp(VM_MAX_STACK_SIZE -1);		//stack pointer
	//release user data
	while(vm_get_bp() != 0) {		//in-case exception make use of _bp_start
		obj = vm_pop_base(); 
		if(obj == ((vm_object *)VM_NULL_OBJECT)) continue;
		if((obj->mgc_refcount & VM_MAGIC) == VM_MAGIC) obj->mgc_refcount &= 0xF0;		//clear reference counter
	}
	//collect garbage  
	//vm_pop_stack_arc();
	vm_garbage_collect(VM_ARG);
	//release current file buffer if current bytecodes execution from sms-pp
#if FS_MAX_MUTEX != 0
	fsMutexUnlock(&_vm_file);
#endif
	vm_set_state(VM_ARG, VM_STATE_INIT);
	//va_exec_release(VM_ARG);			//release framework (end of execution)
}

vm_object * vm_pop_stack_arc(VM_DEF_ARG) _REENTRANT_ {
	vm_object * obj = VM_NULL_OBJECT;
	if((vm_get_sp() + 1) == VM_MAX_STACK_SIZE) {
		vm_invoke_exception(VM_ARG, VX_STACK_UNDERFLOW);
		return VM_NULL_OBJECT;
	}
	obj = vm_pop_stack();
	vm_set_stack((vm_object *)VM_NULL_OBJECT);
	if(obj != VM_NULL_OBJECT) {
		obj->mgc_refcount--;
	}
	return obj;
}

void vm_push_argument(VM_DEF_ARG, BYTE size, BYTE * buffer) _REENTRANT_ {
	vm_object * obj;
	obj = vm_create_object(size, buffer); 
	vm_push_stack(obj);
}

void vm_push_argument_object(VM_DEF_ARG, vm_object * obj) _REENTRANT_ {
	vm_push_stack(obj);
	obj->mgc_refcount++;
}

BYTE vm_pop_result(VM_DEF_ARG, BYTE max_len, BYTE * buffer) _REENTRANT_ {
   	vm_object * obj = vm_pop_stack();
	BYTE size;
	if(obj == VM_NULL_OBJECT) return 0;
	size = (obj->len < max_len)?obj->len:max_len;
	obj->mgc_refcount--;
	vm_memcpy(buffer, obj->bytes, size);
	return size;
}

vm_variable * vm_variable_new(vm_variable ** root, uint8 type, uint16 length, uint8 * bytes) {
	vm_variable * var;
	vm_variable * iterator = root[0];
	//if(root != _var_root) return NULL;		//only current context could create variable, private accessor
	var = (vm_variable *)os_alloc(sizeof(vm_variable) + length);
	if(var != NULL) {
		var->mgc = type;
		var->len = length;
		memcpy(var->bytes, bytes, length);
		var->next = NULL;
		if(root[0] == NULL) {
			root[0] = var;
		} else {
			while(iterator->next != NULL) {
				iterator = iterator->next;
			}
			iterator->next = var;
		}
	}
	return var;
}

void vm_variable_release(vm_variable ** root, vm_variable * var) {
	vm_variable * iterator = root[0];
	//if(root != _var_root) return;			//only current context could release variable, private accessor
	if(iterator == var) {
		root[0] = iterator->next;
	} else {
		while(iterator != NULL) {
			if(iterator->next == var) {
				iterator->next = var->next;
				break;
			}
			iterator = iterator->next;
		}
	}
	os_free(var);
}

void vm_variable_clear(vm_variable ** root) {
	vm_variable * iterator = root[0];
	vm_variable * var;
	while(iterator != NULL) {
		var = iterator;
		iterator = iterator->next;
		os_free(var);
	}
	root = NULL;
}

//added 2018.01.10
void vm_exec_function(VM_DEF_ARG, vm_function * func) {
	vm_context vctx;
	uint8 hbuf[4];
	uint8 tag, hlen;
	uint16 header_size = 0;
	uint16 codestart;
	vm_object * obj;
	//save current vm_context base stack, PC should already incremented
	vm_memcpy(&vctx, vm_get_package(), sizeof(vm_context));
	vm_push_base(obj = vm_create_context(&vctx));
	//set new vm_context based on func argument
	vm_set_package(func);
	//only for terminal (because each bytecodes file are stored with their respectual header codes)
#if 1			
	hlen = vf_pop_handle((vf_handle_p)vm_get_package(), 0, &tag, &header_size);
	codestart = header_size + hlen;										//total header length
#endif
	vm_set_pc(func->base.offset + codestart);
	vm_set_rexec(TRUE);
}

void vm_decode(VM_DEF_ARG) _REENTRANT_ {
	OS_DEBUG_ENTRY(vm_decode);
	BYTE ibuf[3];
	BYTE opcode;
	BYTE psw = 0;
	WORD index = 0;
	vm_object * obj;
	vm_object * op1, * op2;
	vm_context vctx;
	
	vm_memcpy(&vctx, vm_get_package(), sizeof(vm_context)); 	 
	if(vm_get_state() != VM_STATE_EXCEPTION) vm_set_state(VM_ARG, VM_STATE_RUN);
	while(vm_get_state() == VM_STATE_RUN) {  
		vm_fetch(VM_ARG, vm_get_pc(), ibuf, 3); 
		opcode = ibuf[0];		
		vm_set_rexec(FALSE);		//set recursive_exec flag to false
		switch(opcode) {
			case INS_OBJPUSH:
				index = ibuf[1];
				index += 1;
				obj = vm_get_base(index);
				if(obj != VM_NULL_OBJECT) {
					obj->mgc_refcount ++;
				}
				vm_push_stack(obj);
				vm_add_pc(2);
				break;
			case INS_OBJSTORE:
				index = ibuf[1];
				index += 1;
				obj = vm_stack(vm_get_sp() + 1);
				goto vmd_store_variable;
			case INS_OBJPOP:
				index = ibuf[1];
				index += 1;
				obj = vm_pop_stack_arc(VM_ARG);
				vmd_store_variable:
				if(vm_get_base(index) != obj) {
					op1 = vm_get_base(index);
					if(op1 != VM_NULL_OBJECT) {
						op1->mgc_refcount--;
					}
					vm_set_base(index, obj);
					if(obj != VM_NULL_OBJECT) {
						obj->mgc_refcount++;
					}
				}
				vm_add_pc(2);
				break;
			case INS_SWITCH:
				obj = vm_pop_stack_arc(VM_ARG);
				index = end_swap16(*((uint16 * )(ibuf + 1)));
				#if IS_REL_JUMP_ADDRESS
				op1 = vm_load_constant(VM_ARG, index + vm_get_pc());
				#else	
				op1 = vm_load_constant(VM_ARG, index);
				#endif
				//jumptable
				for(psw =2; psw < op1->len; psw += 4) {
					index = end_swap16(*((uint16 * )(op1->bytes + psw)));
					#if IS_REL_JUMP_ADDRESS
					op2 = vm_load_constant(VM_ARG, index + vm_get_pc());
					#else
					op2 = vm_load_constant(VM_ARG, index);
					#endif
					//printf("%s\n", op2->bytes);
					if(vm_cmp_object(obj, op2) == VM_CMP_EQ) {			//object match found
						index = end_swap16(*((uint16 * )(op1->bytes + psw + 2)));
						vm_release_object(op2);							//release constant object
						goto start_jump;
					}
					vm_release_object(op2);								//release constant object
				}	
				//default index
				index = end_swap16(*((uint16 * )(op1->bytes)));			//default jump offset
				start_jump:
				vm_release_object(op1);									//release jump table
				#if IS_REL_JUMP_ADDRESS
				vm_set_pc(index + vm_get_pc());
				#else
				vm_set_pc(index);
				#endif	
				//vm_garbage_collect();
				break;
			case INS_OBJDEL:
				obj = vm_pop_stack_arc(VM_ARG);
				vm_release_object(obj);
				vm_add_pc(1);
				//vm_garbage_collect();
				break;
			#if 0		//deprecated, switch to substring APIs
			case INS_OBJSUB:
				op2 = vm_pop_stack_arc(VM_ARG);
				op1 = vm_pop_stack_arc(VM_ARG);
				obj = vm_pop_stack_arc(VM_ARG);
				obj = vm_split_object(op2, op1, obj);
				vm_push_stack(obj);
				vm_add_pc(1);
				break;
			#endif
			case INS_OBJSZ:
				op1 = vm_pop_stack_arc(VM_ARG);
				obj = vm_size_object(op1);
				vm_push_stack(obj);
				vm_add_pc(1);
				break;
			case INS_NOT:
				op2 = vm_pop_stack_arc(VM_ARG);
				op1 = op2;
				obj = vm_operation_object(VM_ARG, opcode, op2, op1);
				vm_push_stack(obj);
				vm_add_pc(1);
				break;
			case INS_ADD:
			case INS_MUL:
			case INS_DIV:
			case INS_SUB:
			case INS_MOD:
			case INS_AND:
			case INS_OR:
			case INS_XOR:
				op2 = vm_pop_stack_arc(VM_ARG);
				op1 = vm_pop_stack_arc(VM_ARG);
				obj = vm_operation_object(VM_ARG, opcode, op2, op1);
				vm_push_stack(obj);
				vm_add_pc(1);
				break;
			case INS_SYSCALL0:	   		//syscall without argument
			case INS_SYSCALL1:			//syscall with 1 argument
			case INS_SYSCALL2:			//syscall with 2 arguments
			case INS_SYSCALL3:			//syscall with 3 arguments
			case INS_SYSCALL4:			//syscall with 4 arguments
			case INS_SYSCALL5:			//syscall with 5 arguments
			case INS_SYSCALL6:			//syscall with 6 arguments
			case INS_SYSCALL7:			//syscall with 7 arguments
			case INS_SYSCALL8:			//syscall with 8 arguments
			case INS_SYSCALL9:			//syscall with 9 arguments
			case INS_SYSCALL10:			//syscall with 10 arguments
			case INS_SYSCALL11:			//syscall with 11 arguments
			case INS_SYSCALL12:			//syscall with 12 arguments
			case INS_SYSCALL13:			//syscall with 13 arguments
			case INS_SYSCALL14:			//syscall with 14 arguments
			case INS_SYSCALL15:			//syscall with 15 arguments
				vm_get_sysc().num_of_params = (opcode - INS_SYSCALL0);		//initialize syscall context
				if(ibuf[1] < 254) {		  	//basic API
					vm_add_pc(2);	//increment PC before execution
					obj = vm_syscall(VM_ARG, ibuf[1]);
					//pop argument stack
					if(vm_get_rexec() == FALSE) {
						for(index = 0;index < vm_get_sysc().num_of_params;index ++) {
							vm_pop_stack_arc(VM_ARG);
						}
						if(vm_get_state() == VM_STATE_RUN) {
							vm_push_stack(obj);
						}
					}
				} else {					//extended API  
					vm_add_pc(3);
					vm_push_stack(VM_NULL_OBJECT);
				}
				break; 
			//F2O support argument count and vm_context (added 2018.01.10)
			case INS_F2O_0:			//96	//-> function to object
			case INS_F2O_1:			//97	//-> function to object
			case INS_F2O_2:			//98	//-> function to object
			case INS_F2O_3:			//99	//-> function to object
			case INS_F2O_4:			//100	//-> function to object
			case INS_F2O_5:			//101	//-> function to object
			case INS_F2O_6:			//102	//-> function to object
			case INS_F2O_7:			//103	//-> function to object
			case INS_F2O_8:			//104	//-> function to object
			case INS_F2O_9:			//105	//-> function to object
			case INS_F2O_10:			//106	//-> function to object
			case INS_F2O_11:			//107	//-> function to object
			case INS_F2O_12:			//108	//-> function to object
			case INS_F2O_13:			//109	//-> function to object
			case INS_F2O_14:			//110	//-> function to object
			case INS_F2O_15:			//111	//-> function to object
				index = end_swap16(*((uint16 * )(ibuf + 1)));
				obj = vm_create_context(&vctx);				//copy current execution handle (vm_context)
				((vm_function *)obj->bytes)->arg_count = (opcode - INS_F2O_0);		//set argument count
				vm_memcpy(((vm_function *)obj->bytes)->name, os_get_active_task()->name, OS_TASK_MAX_NAME_SIZE);	
				((vm_context *)obj->bytes)->var_root = NULL;
				((vm_context *)obj->bytes)->vars = &((vm_context *)obj->bytes)->var_root;
				((vm_context *)obj->bytes)->offset = index;									//set function offset
				vm_push_stack(obj);
				vm_add_pc(3);
				break;
			case INS_SCTX:
				//increment base pointer
				index = ibuf[1];
				for(psw = 0; psw < index; psw ++) {
					vm_push_base((vm_object *)VM_NULL_OBJECT);
				}
				vm_add_pc(2);
				break;
			case INS_RCTX:
				//_bp -= ibuf[1];			//increment base pointer
				index = ibuf[1];
				for(psw = 0; psw < index; psw ++) {
					obj = vm_pop_base();
					vm_stack(vm_get_bp()) = VM_NULL_OBJECT;
					if(obj != VM_NULL_OBJECT) obj->mgc_refcount --;
				}
				vm_add_pc(2);
				break;
			case INS_RET:
				obj = vm_pop_base();
				vm_memcpy(&vctx, obj->bytes, sizeof(vm_context));
				vm_set_pc(vctx.offset);
				vm_set_package(&vctx);
				vm_stack(vm_get_bp()) = (vm_object *)VM_NULL_OBJECT;
				if(obj != VM_NULL_OBJECT) obj->mgc_refcount --;
				break;
			//case INS_EXTCALL: 
			case INS_EXTCALL0:
			case INS_EXTCALL1:
			case INS_EXTCALL2:
			case INS_EXTCALL3:
			case INS_EXTCALL4:
			case INS_EXTCALL5:
			case INS_EXTCALL6:
			case INS_EXTCALL7:
			case INS_EXTCALL8:
			case INS_EXTCALL9:
			case INS_EXTCALL10:
			case INS_EXTCALL11:
			case INS_EXTCALL12:
			case INS_EXTCALL13:
			case INS_EXTCALL14:
			case INS_EXTCALL15:	
				//implemented 2015.05.16, fixed invokation mechanism (2016.06.02)
				index = end_swap16(*((uint16 * )(ibuf + 1)));
				op1 = vm_pop_stack_arc(VM_ARG);		//class instance
				//method constant
				#if IS_REL_JUMP_ADDRESS
				op2 = vm_load_constant(VM_ARG, index + vm_get_pc());
				#else
				op2 = vm_load_constant(VM_ARG, index);
				#endif
				obj = (vm_object *)vm_load_method(op1, op2);
				if(obj == NULL) { vm_invoke_exception(VM_ARG, VX_UNRESOLVED_METHOD); vm_set_pc(0xFFFF); break; }
				//save current context
				vm_memcpy(&vctx, vm_get_package(), sizeof(vm_context));
				vctx.offset = (vm_get_pc() + 3);
				vm_push_base(vm_create_context(&vctx));
				//set to new context
				vm_set_package((vm_context *)obj->bytes);
				index = ((vm_context *)(obj->bytes))->offset; 
				//set new program counter
				vm_set_pc(index);
				break;			 
			case INS_CALL:
				//save current handle to stack
				vm_memcpy(&vctx, vm_get_package(), sizeof(vm_context));
				vctx.offset = (vm_get_pc() + 3);	
				vm_push_base(obj = vm_create_context(&vctx));
			case INS_JMP:									//jump to specified label
				index = end_swap16(*((uint16 * )(ibuf + 1)));
				#if IS_REL_JUMP_ADDRESS
				vm_set_pc(index + vm_get_pc());
				#else
				vm_set_pc(index);
				#endif
				break;
			case INS_OBJNEW: 
				//implemented 2016.05.15 (almost exactly one year after vm_load_method, coincidence??)
				index = end_swap16(*((uint16 * )(ibuf + 1)));
				#if IS_REL_JUMP_ADDRESS
				op1 = vm_load_constant(VM_ARG, index + vm_get_pc());
				#else
				op1 = vm_load_constant(VM_ARG, index);
				#endif 	
				obj = vm_load_class(VM_ARG, op1); 
				if(obj == VM_NULL_OBJECT) { vm_invoke_exception(VM_ARG, VX_UNRESOLVED_CLASS); vm_set_pc(0xFFFF); break; }
				obj->mgc_refcount++;
				vm_push_stack(obj);		
				vm_add_pc(3);
				vm_release_object(op1);
				break;
			case INS_OBJCONST:
				index = end_swap16(*((uint16 * )(ibuf + 1)));
				#if IS_REL_JUMP_ADDRESS
				obj = vm_load_constant(VM_ARG, index + vm_get_pc());
				#else
				obj = vm_load_constant(VM_ARG, index);
				#endif 
				obj->mgc_refcount++;
				vm_push_stack(obj);	
				vm_add_pc(3);
				break;
			case INS_JTRUE:
			case INS_JFALSE:
				op1 = vm_pop_stack_arc(VM_ARG);
				#if IS_REL_JUMP_ADDRESS
				index = 3;
				#else
				index = (vm_get_pc() + 3);
				#endif
				if(opcode == INS_JFALSE) {
					if(vm_memcmp(op1->bytes, "false", op1->len) == 0) {
						index = end_swap16(*((uint16 * )(ibuf + 1)));
					}
				} else {
					if(vm_memcmp(op1->bytes, "true", op1->len) == 0) {
						index = end_swap16(*((uint16 * )(ibuf + 1)));
					}
				}
				#if IS_REL_JUMP_ADDRESS
				vm_set_pc(index + vm_get_pc());
				#else
				vm_set_pc(index);
				#endif
				break;
			case INS_CREQ:		//64	//-> jump if equal (relative to pc)
			case INS_CRNE:			//65	//-> jump not equal (relative to pc)
			case INS_CRGT:			//66	//-> jump greater than (relative to pc)
			case INS_CRLT:			//67	//-> jump less than (relative to pc)
			case INS_CRGTEQ:			//68	//-> jump greater than (relative to pc)
			case INS_CRLTEQ:			//69	//-> jump less than (relative to pc)
				op2 = vm_pop_stack_arc(VM_ARG);
				op1 = vm_pop_stack_arc(VM_ARG);
				psw = vm_cmp_object(op2, op1);
				switch(opcode) {
					case INS_CREQ: obj = vm_load_bool((psw & VM_CMP_EQ)); break;
					case INS_CRNE: obj = vm_load_bool((psw & VM_CMP_EQ) == 0); break;
					case INS_CRGT: obj = vm_load_bool((psw == VM_CMP_GT)); break;
					case INS_CRLT: obj = vm_load_bool((psw == VM_CMP_LT)); break;
					case INS_CRGTEQ: obj = vm_load_bool((psw != VM_CMP_LT)); break;
					case INS_CRLTEQ: obj = vm_load_bool((psw != VM_CMP_GT)); break;
					default: obj = VM_NULL_OBJECT; break;
				}
				vm_push_stack(obj);
				vm_add_pc(1);
				break;
			default:			//unknown instruction exception
				//_pc += 1;
				vm_invoke_exception(VM_ARG, VX_UNKNOWN_INSTRUCTION);
				break;

		} 
		if(vm_get_bp() >= vm_get_sp()) {		//stack overflow exception
		  	vm_invoke_exception(VM_ARG, VX_STACK_OVERFLOW);
			break;
		}
		//getch();
		vm_garbage_collect(VM_ARG);
		if(vm_get_pc() == 0xFFFF) {	  		//normally terminated
			//release stack	and user data, prevent memory leakage even in-case of exception
			terminate_execution:
			///vm_set_state(VM_ARG, VM_STATE_INIT);		//reset VM
			break;
		}
	}
	OS_DEBUG_EXIT();
	return;
}

vm_object * vm_load_class(VM_DEF_ARG, vm_object * clsobj) _REENTRANT_ {		
	vm_object * obj = VM_NULL_OBJECT;
	//implemented 2016.05.15
	vm_context vctx;
	vm_memcpy(&vctx, vm_get_package(), sizeof(vm_context));
	if(vr_load_script(VM_SCRIPT_BY_CLASS, &vctx.handle, clsobj->len, clsobj->bytes) == 0) {
		obj = (vm_object *)vm_create_context(&vctx);		//return new context
		((vm_context *)(obj->bytes))->var_root = NULL;
		((vm_context *)(obj->bytes))->vars = &((vm_context *)(obj->bytes))->var_root;
		if(obj != VM_NULL_OBJECT) obj->mgc_refcount--; 		//unref constant, will automatically incremented by objnew
	}
	return obj;							//invalid class
}

vm_object * vm_load_method(vm_object * clins, vm_object * mthobj) _REENTRANT_ { 
	//implemented 2015.05.16
	//vm_object * obj;
	vm_context * vctx;	 
	if(clins == VM_NULL_OBJECT) return (vm_object *)0;
	if(clins->len == 0) return (vm_object *)0;
	vctx = (vm_context *)clins->bytes;
	vctx->offset = vr_load_script(VM_SCRIPT_BY_METHOD, &vctx->handle, mthobj->len, mthobj->bytes);
	//fixed 2016.05.22
	if(vctx->offset == (uint16)-1) return (vm_object *)0;		//invalid method
	return clins;
}

vm_object * vm_load_bool(uchar value) _REENTRANT_ {
	vm_object * obj = VM_NULL_OBJECT;
	if(value) {
		obj = vm_create_object(4, VM_TRUE_OBJECT);		//create new object (clone object)
	} else {
		obj = vm_create_object(5, VM_FALSE_OBJECT);		//create new object (clone object)
	}
	return obj;
}

vm_object * vm_load_constant(VM_DEF_ARG, uint16 offset) _REENTRANT_ {
	vm_object * obj = VM_NULL_OBJECT;
	uint8 llen;
	uint8 lbuf[4];
	uint16 len = 0;
	//uchar * buffer = NULL;
	vm_fetch(VM_ARG, offset, lbuf, 4);
	if(lbuf[0] < 128) { len = lbuf[0]; llen = 1; }
	else {
		switch(lbuf[0] & 0x0F) {
			case 0x01: len = lbuf[1]; llen = 2; break;
			case 0x02: len = ((uint16)lbuf[1] << 8) | lbuf[2]; llen = 3; break;
			default: break;
		}
	}
	//buffer = mmAllocMemP(len + 1);
	if(len > VA_OBJECT_MAX_SIZE) {
		vm_invoke_exception(VM_ARG, VX_OUT_OF_BOUNDS);
		return VM_NULL_OBJECT;
	}
	obj = vm_create_object(len, NULL);
	if(obj->len != 0) {	   
		obj->mgc_refcount--; 									//unref constant
		vm_fetch(VM_ARG, offset + llen, obj->bytes, len);		//obj = jumptable
	}
	return obj;
}

vm_object * vm_create_object(uint16 length, void * buffer) _REENTRANT_ {
	vm_object * newobj;// = (vm_object *)mmAllocMemP(2 + length);
	uint8 * bytes = (uint8 *)buffer;
	newobj = (vm_object *)mmAllocMemP(sizeof(vm_object) + length);
	if(newobj == NULL) {
		//vm_garbage_collect();		//try collecting garbage first
		//newobj = (vm_object *)mmAllocMemP(sizeof(vm_object) + length);
		//if(newobj == NULL) {
			//memory exception
		//	vm_invoke_exception(VX_INSUFFICIENT_HEAP);
			return VM_NULL_OBJECT;
		//}
	}
	newobj->mgc_refcount = (VM_MAGIC | 1);
	newobj->len = length;
	vm_memcpy(newobj->bytes, bytes, length);
	return newobj;
}

vm_object * vm_create_context(vm_context * vctx) _REENTRANT_ {
	vm_object * newobj;// = (vm_object *)mmAllocMemP(2 + length);
	newobj = (vm_object *)mmAllocMemP(sizeof(vm_object) + sizeof(vm_context));
	if(newobj == NULL) {
		//vm_garbage_collect();		//try collecting garbage first
		//newobj = (vm_object *)mmAllocMemP(sizeof(vm_object) + sizeof(vm_context));
		//if(newobj == NULL) {
			//memory exception
		//	vm_invoke_exception(VX_INSUFFICIENT_HEAP);
			return VM_NULL_OBJECT;
		//}
	}
	newobj->mgc_refcount = (VM_CTX_MAGIC | 1);
	newobj->len = sizeof(vm_context);
	vm_memcpy(newobj->bytes, vctx, sizeof(vm_context));
	((vm_context *)newobj->bytes)->vars = &((vm_context *)newobj->bytes)->var_root;
	return newobj;
}

uint8 vm_object_get_type(vm_object * obj) {
	return (obj->mgc_refcount & VM_MAGIC_MASK);
}

uint16 vm_object_get_text(vm_object * obj, uint8 * text) {
	uint16 len = 0;
	switch(vm_object_get_type(obj)) {
		case VM_MAGIC:
			len = obj->len;
			memcpy(text, obj->bytes, len);
			break;
		case VM_EXT_MAGIC:
			len = ((vm_extension *)obj->bytes)->apis->text(obj, text);
			break;
	}
	return len;
}

uint16 vm_object_get_length(vm_object * obj) {
	uint16 len = 0;
	uint8 buffer[64];
	switch(vm_object_get_type(obj)) {
		case VM_MAGIC:
			len = obj->len;
			break;
		case VM_EXT_MAGIC:
			len = ((vm_extension *)obj->bytes)->apis->text(obj, buffer);
			break;
	}
	return len;
}

uint8 vm_ext_get_tag(vm_object * obj) {
	if(vm_object_get_type(obj) != VM_EXT_MAGIC) return ASN_TAG_OCTSTRING;
	return ((vm_extension *)(obj->bytes))->tag;
}

vm_object * vm_create_extension(uint8 tag, vm_custom_opcode * apis, uint16 length, uchar * bytes) {
	vm_object * newobj = (vm_object *)mmAllocMemP(sizeof(vm_object) + sizeof(vm_extension) + length);
	newobj->mgc_refcount = (VM_EXT_MAGIC | 1);
	newobj->len = sizeof(vm_extension) + length;
	((vm_extension *)(newobj->bytes))->tag = tag;
	((vm_extension *)(newobj->bytes))->apis = apis;
	if(bytes != NULL) memcpy(((vm_extension *)(newobj->bytes))->payload, bytes, length);
	else memset(((vm_extension *)(newobj->bytes))->payload, 0, length);
	return newobj;
}

void vm_release_object(vm_object * obj) _REENTRANT_ {
	if(obj == NULL) return;
	if(obj == VM_NULL_OBJECT) return;
	if((obj->mgc_refcount & 0x0F) == 0) {
		mmFreeMem(obj);
	}
}

vm_object * vm_size_object(vm_object * op1) _REENTRANT_ {
	uint32 objlen = op1->len;
	BYTE buffer[16];	
	sprintf((char *)buffer, "%d", objlen);
	return vm_create_object(vm_strlen((const char *)buffer), buffer);
}

uchar vm_is_numeric(uchar * buffer, uchar len) _REENTRANT_ { 
	uint8 n;
	if(len == 0) return FALSE;
	while(len != 0 ) {
		n = buffer[--len];
		if((n == '-') && (len == 0)) break;
		if(n > 0x39 || n < 0x30) { return FALSE; }
	} 
	return TRUE;
}

#if 0			//deprecated, switched to substr API
vm_object * vm_split_object(vm_object * op2, vm_object * op1, vm_object * target) _REENTRANT_ {
	uint16 offset, len;
	vm_object * obj;
	uint8 * opd1, * opd2;
	opd1 = mmAllocMemP(op1->len + 1);
	opd2 = mmAllocMemP(op2->len + 1);
	vm_memcpy(opd1, op1->bytes, op1->len); opd1[op1->len] = 0;		//null terminated string
	vm_memcpy(opd2, op2->bytes, op2->len); opd2[op2->len] = 0;		//null terminated string
	if(vm_is_numeric(op1->bytes, op1->len) == FALSE) { offset = 0; } else { offset = atoi(opd1); }			//
	if(vm_is_numeric(op2->bytes, op2->len) == FALSE) { len = target->len; } else { len = atoi(opd2); }
	if(len > (target->len - offset)) len = (target->len - offset);
	mmFreeMem(opd1);
	mmFreeMem(opd2);
	obj = vm_create_object(len, target->bytes + offset);
	return obj;
}
#endif

int32 vm_atoi(const char * buf) _REENTRANT_ {
	int32 dwVal = 0;
	uint8 len;
	uint8 i;
	uint8 neg = 0;
	for(len = 0;buf[len]!=0;len++);
	for(i = 0;i!=len;i++) {
		if(buf[i] == '-') { neg = 1; continue; }
	 	dwVal *=	10;
		dwVal += (buf[i] & 0x0F);
	}
	if(neg) {
		dwVal = 0 - dwVal; 	
	}
	return dwVal;
}


CONST BYTE g_baByte2Char[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

uint8 vm_itoa(BYTE mode, BYTE * buffer, DWORD val) _REENTRANT_ {
	BYTE abuffer[16];
	BYTE alen = sizeof(abuffer);
	BYTE f = ' ';
	BYTE total_len = 0;
	BYTE radix = 10;
	BYTE i = 0;
	BYTE sindex;
	if(mode & 0x80) {
		if(mode & 0x40) f = '0';
	}
	memset(abuffer, f, sizeof(abuffer));
	if(mode & MM_ITOA_RADIX_16) radix = 16;
	if(mode & MM_ITOA_BYTE) val = (BYTE)val;
	if(val == 0) {
		buffer[i++] = 0x30;
	} else {
		while(val != 0) {
			abuffer[--alen] = g_baByte2Char[val % radix];
			val = val / radix;
		}
		if(mode & 0x80) alen = sizeof(abuffer) - (mode & 0x07);
		total_len = sizeof(abuffer) - alen;
		//sindex = total_len - alen;
		for(i=0;i<total_len;i++) {
			buffer[i] = abuffer[alen++];
		}
	}
	buffer[i] = 0;
	return i;
}

vm_object * vm_operation_object(VM_DEF_ARG, uchar opcode, vm_object * op2, vm_object * op1) _REENTRANT_ {
	#define VM_NAN_OBJECT		"NaN"
	int16 len;
	int32 value1, value2;
	uint8 buffer[16];
	vm_object * obj = VM_NULL_OBJECT;
	if(vm_object_get_type(op1) == VM_EXT_MAGIC && vm_object_get_type(op2) == VM_EXT_MAGIC) {
		switch(opcode) {
			case INS_ADD: obj = ((vm_extension *)op1->bytes)->apis->add(op1, op2); break;
			case INS_SUB: obj = ((vm_extension *)op1->bytes)->apis->sub(op1, op2); break;
			case INS_MUL: obj = ((vm_extension *)op1->bytes)->apis->mul(op1, op2); break;
			case INS_DIV: obj = ((vm_extension *)op1->bytes)->apis->div(op1, op2); break;
			case INS_AND: obj = ((vm_extension *)op1->bytes)->apis->and(op1, op2); break;
			case INS_OR: obj = ((vm_extension *)op1->bytes)->apis->or(op1, op2); break;
			case INS_XOR: obj = ((vm_extension *)op1->bytes)->apis->xor(op1, op2); break;
			case INS_NOT: obj = ((vm_extension *)op1->bytes)->apis->not(op1); break;
			default: break;
		}
	}
	else if(vm_is_numeric(op1->bytes, op1->len) == TRUE && vm_is_numeric(op2->bytes, op2->len) == TRUE) {		//number
		//buffer = mmAllocMemP(12);
		vm_memcpy(buffer, op1->bytes, op1->len);
		buffer[op1->len] = 0;
		value1 = vm_atoi((const char *)buffer);
		vm_memcpy(buffer, op2->bytes, op2->len);
		buffer[op2->len] = 0;
		value2 = vm_atoi((const char *)buffer);
		switch(opcode) {
			case INS_ADD: value1 = value1 + value2; break;
			case INS_SUB: value1 = value1 - value2; break;
			case INS_MUL: value1 = value1 * value2; break;
			case INS_DIV: 
				if(value2 == 0) {		//check for divide by zero
					vm_invoke_exception(VM_ARG, VX_DIV_BY_ZERO);
					break;
				}
				value1 = value1 / value2; 
				break;
			case INS_MOD: value1 = value1 % value2; break;
			//logical operation
			case INS_AND: value1 = value1 & value2; break;
			case INS_OR: value1 = value1 | value2; break;
			case INS_XOR: value1 = value1 ^ value2; break;
			case INS_NOT: value1 = !value1; break;
			default: break;
		}
		//mmItoa(MM_ITOA_WORD, buffer, (WORD)value1);
		//sprintf((char *)buffer, "%d", value1);
		//vm_itoa(MM_ITOA_WORD, buffer, value1);
		sprintf((char *)buffer, "%i", value1);
		obj = vm_create_object(vm_strlen((const char *)buffer), buffer);
		//mmFreeMem(buffer);
	} else {		//string
		switch(opcode) {
			case INS_ADD:					//string concatenation
				len = op1->len + op2->len;
				if(len > 0x1000) {			//4K bounds
					vm_invoke_exception(VM_ARG, VX_OUT_OF_BOUNDS);
					break;
				}
				obj = vm_create_object(len, buffer);
				if(obj != VM_NULL_OBJECT) {
					vm_memcpy(obj->bytes, op1->bytes, op1->len);
					vm_memcpy(obj->bytes + op1->len, op2->bytes, op2->len);
				}
				//mmFreeMem(buffer);
				break;
			default:
				obj = vm_create_object(3, VM_NAN_OBJECT);
				break;
		}
	}
	return obj;
}

BYTE vm_cmp_object(vm_object * op2, vm_object * op1) _REENTRANT_ {
	int32 value1, value2;
	uint8 opd1[16];
	uint8 opd2[16];
	if(vm_is_numeric(op1->bytes, op1->len) == TRUE && vm_is_numeric(op2->bytes, op2->len) == TRUE) {		//number
		vm_memcpy(opd1, op1->bytes, op1->len); opd1[op1->len] = 0;
		vm_memcpy(opd2, op2->bytes, op2->len); opd2[op2->len] = 0;
		value1 = atoi((const char *)opd1);
		value2 = atoi((const char *)opd2);
		value1 = value1 - value2;
	} else {		//string
		if(op1->len == op2->len) {
			value1 = vm_memcmp(op1->bytes, op2->bytes, op1->len);
		} else {
			value1 = op1->len - op2->len;
		}
	}
	if(value1 == 0) {
		return VM_CMP_EQ;
	} else if(value1 < 0) {
		return VM_CMP_LT;
	} else {
		return VM_CMP_GT;
	}
}

//#if MM_MALLOC_API == MM_API_CHAIN
void vm_garbage_collect(VM_DEF_ARG) _REENTRANT_ {									//reference counting garbage collector
	vm_object * candidate = NULL;
	vm_object * nextchunk = NULL;
	vm_object * shifted = NULL;
	vm_object * iterator = (vm_object *)mmFirstChunk();
	while(iterator != NULL) {
		if((iterator->mgc_refcount & VM_MAGIC_MASK_ZERO) == VM_MAGIC) {				//check vm_object reference counter == 0
			candidate = iterator;
		}
		nextchunk = (vm_object *)mmNextChunk(iterator);
		if(nextchunk == NULL) goto try_release;
		//shift vm_object
		if((nextchunk->mgc_refcount & VM_MAGIC_MASK_ZERO) == VM_MAGIC) { 		//check vm_object for shifting
			shifted = mmShiftNextChunk(iterator);
			if(shifted != nextchunk) {
				vm_update_mutator(VM_ARG, nextchunk, shifted);
				//shift pointer to var_root for context
				if((shifted->mgc_refcount & VM_CTX_MAGIC) == VM_CTX_MAGIC) {
					((vm_context *)shifted->bytes)->vars = &((vm_context *)shifted->bytes)->var_root;
				}
				nextchunk = shifted;
			}
		}
		try_release:
		iterator = nextchunk;
		if(candidate != NULL) {
			if((candidate->mgc_refcount & VM_CTX_MAGIC) == VM_CTX_MAGIC) {
				//context type, release context variable
				if(((vm_context *)candidate->bytes)->var_root != NULL) {
					vm_variable_clear(((vm_context *)candidate->bytes)->vars);
				}
			}
			//printf("[GarbageCollector] : clear an object %d bytes, %s\n", m_size_chunk(candidate), candidate->bytes);
			vm_memset(candidate, 0, mmSizeChunk(candidate));
			mmFreeMem(candidate);
			candidate = NULL;
		}
	}

}
//#endif

void vm_update_mutator(VM_DEF_ARG, void * old_addr, void * new_addr) _REENTRANT_ {									//realign all objects on heap
	BYTE i = 0;
	for(i =0 ;i<VM_MAX_STACK_SIZE; i++) {
		//check if object matched and not null object
		if(vm_stack(i) != VM_NULL_OBJECT && vm_stack(i) == old_addr) vm_stack(i) = new_addr;
	}
}

vm_object * vm_get_argument(VM_DEF_ARG, uchar index) _REENTRANT_ {
	vm_object * obj = VM_NULL_OBJECT;
	WORD base_arg = vm_get_sp() + vm_get_sysc().num_of_params;
	if(index >= vm_get_sysc().num_of_params) return VM_NULL_OBJECT;
	if(vm_get_sp() <= (base_arg - index)) {
		obj = vm_stack(base_arg - index);
	}
	return obj;
}

void va_sys_exec(VM_DEF_ARG) _REENTRANT_ {
	uint8 arglen, i;
	vm_function * func;
	uint16 base_arg = vm_get_sp() + vm_get_sysc().num_of_params;
	OS_DEBUG_ENTRY(va_sys_exec);
	if(vm_get_argument_count(VM_ARG) == 0) {
		vm_invoke_exception(VM_ARG, VX_ARGUMENT_MISMATCH);
		goto exit_sys_exec;
	}
	arglen = vm_get_argument_count(VM_ARG) - 1;
	((vm_object *)vm_get_argument(VM_ARG, 0))->mgc_refcount--;
	func = (vm_function *)(vm_get_argument(VM_ARG, 0)->bytes);
	//check function argument with syscall argument
	if(func->arg_count != arglen) {
		vm_invoke_exception(VM_ARG, VX_ARGUMENT_MISMATCH);
		goto exit_sys_exec;
	}
	//shift arguments by 1
	for(i=1;i<vm_get_argument_count(VM_ARG);i++) {
		vm_stack(base_arg) = vm_stack(base_arg - 1)  ;
		base_arg--;
	}
	vm_get_sp() += 1;
	//execute function (recursive execution from syscall)
	vm_exec_function(VM_ARG, func);
	exit_sys_exec:
	OS_DEBUG_EXIT();
}

uchar vm_get_argument_count(VM_DEF_ARG) _REENTRANT_ {	
	return vm_get_sysc().num_of_params;
}

#define VTXT_INSUFFICIENT_HEAP	 		"\x04Insufficient Heap"
#define VTXT_STACK_OVERFLOW				"\x04Stack Overflow"
#define VTXT_UNKNOWN_INSTRUCTION		"\x04Unknown Instruction"
#define VTXT_UNIMPLEMENTED_APIS			"\x04Unimplemented APIs"
#define VTXT_SYSTEM_EXCEPTION			"\x04System Exception" 
#define VTXT_STACK_UNDERFLOW			"\x04Stack Underflow"
#define VTXT_OUT_OF_BOUNDS				"\x04Out of Bounds"	 
#define VTXT_UNRESOLVED_CLASS			"\x04Unresolved Class"
#define VTXT_UNRESOLVED_METHOD			"\x04Unresolved Method"
#define VTXT_ARGUMENT_MISMATCH			"\x04Argument Mismatch"
#define VTXT_INVALID_CONTEXT			"\x04Invalid Context"
#define VTXT_DIVIDE_BY_ZERO				"\x04Divide by Zero"

static uint8_c * _vm_exception_text[] = {	
	(uint8_c *)VTXT_SYSTEM_EXCEPTION, 
	(uint8_c *)VTXT_UNIMPLEMENTED_APIS, 
	(uint8_c *)VTXT_UNKNOWN_INSTRUCTION, 
	(uint8_c *)VTXT_STACK_OVERFLOW,
  	(uint8_c *)VTXT_INSUFFICIENT_HEAP, 
	(uint8_c *)VTXT_STACK_UNDERFLOW,
	(uint8_c *)VTXT_OUT_OF_BOUNDS,
	(uint8_c *)VTXT_UNRESOLVED_CLASS,
	(uint8_c *)VTXT_UNRESOLVED_METHOD,
	(uint8_c *)VTXT_ARGUMENT_MISMATCH,
	(uint8_c *)VTXT_INVALID_CONTEXT,
	(uint8_c *)VTXT_DIVIDE_BY_ZERO,
};	

void vm_invoke_exception(VM_DEF_ARG, BYTE excp) _REENTRANT_ { 
	#define VX_MAX_EXCEPTION_LENGTH		28
	BYTE len;
	BYTE i = 0;//, j;
	//vm_object * obj;
	extern BYTE g_baTKBuffer[];
	BYTE buffer[VX_MAX_EXCEPTION_LENGTH + 1];
	//display exception text
	tk_console_print(NULL, (char *)_vm_exception_text[excp]);  
#if FS_MAX_MUTEX != 0
	fsMutexUnlock(&_vm_file);
#endif
	vm_set_state(VM_ARG, VM_STATE_EXCEPTION);
	vm_set_pc(0xFFFF);
} 

void vm_set_state(VM_DEF_ARG, BYTE state) _REENTRANT_ {
	vm_get_state() = state;
}

#if 1
BYTE _vm_current_api = 0;
extern CONST vm_api_entry g_vaRegisteredApis[];
#else
BYTE _vm_wait_tag = 0;
#endif

void vm_syscall_ret(VM_DEF_ARG, BYTE size, BYTE * buffer) _REENTRANT_ {
	vm_object * obj = VM_NULL_OBJECT;
#if 1
	vm_api_entry * iterator = (vm_api_entry *)&g_vaRegisteredApis;
	if(_vm_current_api != 0) {
		while(iterator->id != 0) {
		 	if(iterator->id == _vm_current_api) {
			 	obj = iterator->exit(size, buffer);
			} 
			iterator++;
		}
		_vm_current_api = 0; 
	}
#else
	BYTE i = 0, tag;
	WORD len = 0;
	while(i < size) {
		i += tkPop(buffer + i, &tag, &len, buffer); 
		if(tag == _vm_wait_tag) {
			if(tag == STK_TAG_ITEM_ID) {
				//mmItoa(MM_ITOA_WORD, buffer + 1, (WORD)buffer[0]);
				//itoa(buffer + 1, buffer[0]);
				sprintf((char *)buffer + 1, "%d", buffer[0]);
				len = vm_strlen(buffer + 1) + 1; 
			}
			obj = vm_create_object(len - 1, buffer + 1);
			break;
		}
	}
#endif
	vm_push_stack(obj);
	//_pc += 2;		//_pc should already incremented (2018.01.10)
}

vm_object * vm_syscall(VM_DEF_ARG, uint16 api_id) _REENTRANT_ {
	vm_api_entry * iterator = (vm_api_entry *)&g_vaRegisteredApis;
	_vm_current_api = api_id;
	while(iterator->entry != NULL) {
	 	if(iterator->id == api_id) {
			vm_set_retval((vm_object *)VM_NULL_OBJECT);
		 	iterator->entry(VM_ARG);
			return vm_get_retval();
		}
		iterator++;
	} 
	vm_invoke_exception(VM_ARG, VX_UNIMPLEMENTED_APIS);
}


CONST vm_execution_context g_vaExecutionContext[] = {
  	{ VEC_ORB_LOCAL, vm_exec_local_script },
#if STACK_WIB_APIS && TK_WIB_ENABLED
	{ VEC_WIB_PLUGIN, vm_exec_wib_plugin },
#endif
	{ 0x00, NULL},
};


BYTE vm_exec_local_script(VM_DEF_ARG, BYTE result) _REENTRANT_ { 
	BYTE retval = 0;	
	if(result == 0x10) goto exit_execution;	
	if(vm_get_state() != VM_STATE_SUSPEND) return -1;		//only for resume execution (from wait response state) //added 2016.08.02
	//if(g_baTKBuffer[0] < 0x20) return 0;
	switch(result & 0xF0) {		
			//wbExit(WIB_ERR_ABORT_RES); retval = -1; break; 
		case 0x20:
		case 0x30:
			//wbExit(WIB_ERR_CMD_UNSUPPORTED); retval = -1; break;
			vm_invoke_exception(VM_ARG, VX_SYSTEM_EXCEPTION);
			exit_execution:
			vm_close(VM_ARG);
			retval = -1;
			break;
		default: break;			
	}
	return retval;  
}  

BYTE va_exec_context(VM_DEF_ARG, BYTE result) _REENTRANT_ { 
	BYTE retval = -1;
	//BYTE i = 0;
	//g_bVrResult = result;
	//while(g_vaExecutionContext[i].mode != 0) {
	//	if(g_vaExecutionContext[i].mode == g_vmExecContext)
	//		if(g_vaExecutionContext[i].entry != NULL) retval = g_vaExecutionContext[i].entry(result);
	//	i++;	
	//}
	return retval;
}

//#endif

