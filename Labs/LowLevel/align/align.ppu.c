#include <stdlib.h>
#include <stdio.h>
#include <sys/dbg.h>
#include <cell/sysmodule.h>
#include <sys/ppu_thread.h>
#include <sys/timer.h>

void dbg_exception_handler(uint64_t type, sys_ppu_thread_t thread_id, uint64_t dar);
void exception_trigger_thread(void);

int exception_count = 0;

void dbg_exception_handler(uint64_t type, sys_ppu_thread_t thread_id, uint64_t dar)
{
	if (type == 0)
		++exception_count;

	++exception_count;

	printf("EXCEPTION!!!\n");
}

void exception_trigger_thread(void)
{
	sys_timer_sleep(5);

	char* data = (char*) malloc(1024);
	printf("CHECK: misaligned float access (should crash)->");
	float* f0 = ((float*) (data+1));
	*f0 = 1.0f;        // interrupt unaligned access for floats
	//*f0 += 2.0f;
	//float fx = *f0+4.0f;
	//fx += 1.0f;
	printf("OK\n");
}


int main(void) 
{
	printf("CHECK: malloc 16B (quadword) aligned ->");
	char* data = (char*) malloc(1024);
	if ((int)data & 0xF)    // yes 16 byte aligned
	{
		printf("FAILED\n");
	}
	else
	{
		printf("OK\n");
	}


	printf("CHECK: misaligned int access ->");
	int* i0 = ((int*) (data+1));
	*i0 = 1;        // n.p unaligned access for ints
	*i0 += 2;
	int ix = *i0+4;
	ix += 1;
	printf("OK\n");

	// This does not catch misaligned float crashes
	int ret = cellSysmoduleLoadModule(CELL_SYSMODULE_LV2DBG);
	if (ret != CELL_OK) {
		printf("cellSysmoduleLoadModule() error 0x%x !\n", ret);
		exit(ret);
	}
	if ((sys_dbg_initialize_ppu_exception_handler(0) != CELL_OK)
		)
		printf("sys_dbg_initialize_ppu_exception_handler FAILED\n");

	if ((sys_dbg_register_ppu_exception_handler(&dbg_exception_handler, 0/*SYS_DBG_PPU_THREAD_STOP*/) != CELL_OK))
		printf("sys_dbg_register_ppu_exception_handler FAILED\n");

	sys_timer_sleep(1);

	sys_ppu_thread_t	pu_thr;
	float fvalue = 9.000E+32;
	printf("Creating a PPU thread...\n");
	ret = sys_ppu_thread_create(&pu_thr, &exception_trigger_thread,
		(uint64_t)(uintptr_t)&fvalue, 100, 0x4000,
		SYS_PPU_THREAD_CREATE_JOINABLE, "PPU Thread");
	if (ret != CELL_OK) {
		fprintf(stderr, "sys_ppu_thread_create failed: %#.8x\n", ret);
		fprintf(stderr, "Exiting...\n");
		exit(1);
	} else {
		printf("PPU thread %llu is created.\n", pu_thr);
	}
	

	//printf("Wait for the PPU thread %llu exits.\n", pu_thr);
	int exit_code;
	ret = sys_ppu_thread_join(pu_thr, &exit_code);
	if (ret != CELL_OK) {
		//printf("sys_ppu_thread_join failed (%#x)\n", ret);
		return 1;
	}
	//printf("PPU thread %llu's exit_code = %#x\n", pu_thr, exit_code);

	/*
	exception_trigger_thread();
	*/

	sys_timer_sleep(1);

	return 0;
}
