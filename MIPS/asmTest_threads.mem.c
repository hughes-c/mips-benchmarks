//
//
// Spawns input + 1 threads
////////////////////////////////////////


#include <stdio.h>

#include "sescapi.h"

#define TOGGLE_ON(n) do {                      \
        __asm__ __volatile__ (".word 0x74000000+" #n);  \
} while (0)

#define BEGIN_TRANSACTION(n) do {                       \
        __asm__ __volatile__ (".word 0x70000000+" #n);  \
} while (0)

#define COMMIT_TRANSACTION(n) do {                      \
        __asm__ __volatile__ (".word 0x7C000000+" #n);  \
} while (0)

void boo();
void boo2();
void boo3();

	slock_t myLock;
	slock_t myLock2, myLock3;
	sbarrier_t myBarr;
	sbarrier_t myBarr2, myBarr3;

	int* gblobalMemInt;
	register int g_data_out_int asm("23");

	int numThreads;

int main(int argc, char *argv[])
{
	size_t j;
	register int r_zero asm("0");
	register int r_out_t0 asm("8");
	register int r_out_t1 asm("9");
	register int r_out_t2 asm("10");
	register int r_out_t3 asm("11");
	register int r_out_t4 asm("12");

	numThreads = atoi(argv[1]);

	int* memInt;
	register int data_out_int asm("22");
	memInt          = (int*)malloc(sizeof(int) * 2048);
	data_out_int    = (int)&(memInt[0]);

	gblobalMemInt     = (int*)malloc(sizeof(int) * 2048);
	g_data_out_int    = (int)&(gblobalMemInt[0]);

	int int_base = data_out_int;
	int g_int_base = g_data_out_int;

	sesc_lock_init(&myLock);
	sesc_lock_init(&myLock2);
	sesc_lock_init(&myLock3);
	sesc_barrier_init(&myBarr);
	sesc_barrier_init(&myBarr2);
	sesc_barrier_init(&myBarr3);

TOGGLE_ON(2);
	if(numThreads > 0)
	{
		sesc_init();
		for(j = 0; j < numThreads; j++)
			sesc_spawn(boo, NULL, 0);
	}

	sesc_spawn(boo2, NULL, 0);

	register size_t maxCounter asm("20");
	register size_t counter    asm("21");

	__asm__ __volatile__ ("move %0, $0" :"=r"(counter) );
	__asm__ __volatile__ ("li %0, 1000" :"=r"(maxCounter) );
	__asm__ __volatile__ ("BRANCHES_:");
	__asm__ __volatile__ ("bgt %0, %1, END_BRANCHES_" : :"r"(counter), "r"(maxCounter) );
	__asm__ __volatile__ ("addi %0, 1" :"=r"(counter) );
	__asm__ __volatile__ ("j BRANCHES_");
	__asm__ __volatile__ ("END_BRANCHES_:");

	__asm__ __volatile__ ("LOCK1_:");
	sesc_lock(&myLock);
	__asm__ __volatile__ ( "mfhi  %0" :"=r"(r_out_t3) : );
	__asm__ __volatile__ ( "mtc1 %0, $f8  "	 : "=r"(r_out_t0) :  );
	__asm__ __volatile__ ("abs.s $f2, $f4");
	__asm__ __volatile__ ("abs.s $f4, $f6");
	__asm__ __volatile__ ("abs.s $f2, $f6");
	sesc_unlock(&myLock);

	__asm__ __volatile__ ("LOCK2_:");
	sesc_lock(&myLock2);
	__asm__ __volatile__ ("li %0, 12350" :"=r"(r_out_t0) );
	__asm__ __volatile__ ("sw  %1, 16(%0)    "	 : "=r"(data_out_int) : "r"(r_out_t0) : "memory" );
	__asm__ __volatile__ ("lw  %0, 16(%1)  "	 : "=r"(r_out_t3) : "r"(data_out_int) );
	__asm__ __volatile__ ("nop");
	__asm__ __volatile__ ("li %0, 17352" :"=r"(r_out_t1) );
	__asm__ __volatile__ ("sw  %1, 20(%0)    "	 : "=r"(data_out_int) : "r"(r_out_t1) : "memory" );
	__asm__ __volatile__ ("lw  %0, 20(%1)  "	 : "=r"(r_out_t4) : "r"(data_out_int) );
	sesc_unlock(&myLock2);

// 	__asm__ __volatile__ ("TRANS1_:");
        BEGIN_TRANSACTION(7);
// sesc_lock(&myLock);
	__asm__ __volatile__ ("abs.s $f8, $f10");
	__asm__ __volatile__ ("abs.s $f10, $f8");
	__asm__ __volatile__ ("abs.s $f8, $f10");
        COMMIT_TRANSACTION(7);
// sesc_unlock(&myLock);

	__asm__ __volatile__ ("TRANS2_:");
        BEGIN_TRANSACTION(5);
// sesc_lock(&myLock);
	__asm__ __volatile__ ("move %0, $0" :"=r"(counter) );
	__asm__ __volatile__ ("li %0, 1000" :"=r"(maxCounter) );

	__asm__ __volatile__ ("TRANS_BRANCHES_:");
	__asm__ __volatile__ ("bgt %0, %1, END_TRANS_BRANCHES_" : :"r"(counter), "r"(maxCounter) );
	__asm__ __volatile__ ("addi %0, 1" :"=r"(counter) );
	__asm__ __volatile__ ("j TRANS_BRANCHES_");
	__asm__ __volatile__ ("END_TRANS_BRANCHES_:");
        COMMIT_TRANSACTION(5);
// sesc_unlock(&myLock);

	__asm__ __volatile__ ("END_:");

	__asm__ __volatile__ ("abs.s $f4, $f6");
	__asm__ __volatile__ ("abs.s $f6, $f4");
	__asm__ __volatile__ ("abs.s $f6, $f4");
	__asm__ __volatile__ ("j END_F_");


	__asm__ __volatile__ ("END_F_:");

TOGGLE_ON(3);

	for(j = 0; j < numThreads + 1; j++)
		sesc_wait();

	exit(0);
}

void boo()
{
// int que[1000] = {0};
	register size_t maxCounter asm("18");
	register size_t counter    asm("19");

	register int r_out_t0 asm("8");
	register int r_out_t1 asm("9");
	register int r_out_t2 asm("10");
	register int r_out_t3 asm("11");
	register int r_out_t4 asm("12");
	register int r_out_t5 asm("13");
	register int r_out_t6 asm("14");

	int* memInt;
	register int data_out_int asm("22");
	memInt          = (int*)malloc(sizeof(int) * 2048);
	data_out_int    = (int)&(memInt[0]);

// 	sesc_lock(&myLock3);
// 	__asm__ __volatile__ ("move %0, $0" :"=r"(counter) );
// 	__asm__ __volatile__ ("li %0, 1500" :"=r"(maxCounter) );
// 
// 	__asm__ __volatile__ ("THREAD_TRANS_BRANCHES_:");
// 	BEGIN_TRANSACTION(3);
// 	__asm__ __volatile__ ("bgt %0, %1, END_THREAD_TRANS_BRANCHES_" : :"r"(counter), "r"(maxCounter) );
// 	COMMIT_TRANSACTION(3);
// 	__asm__ __volatile__ ("addi %0, 1" :"=r"(counter) );
// 	__asm__ __volatile__ ("j THREAD_TRANS_BRANCHES_");
// 	__asm__ __volatile__ ("END_THREAD_TRANS_BRANCHES_:");
// 	COMMIT_TRANSACTION(3);
// 	sesc_unlock(&myLock3);

// 	__asm__ __volatile__ ("THREAD_TRANS_BRANCHES_:");
// 	__asm__ __volatile__ ("j END_THREAD_TRANS_BRANCHES_");
// 	__asm__ __volatile__ ("END_THREAD_TRANS_BRANCHES_:");


	__asm__ __volatile__ ("abs.s $f8, $f10");
	__asm__ __volatile__ ("abs.s $f10, $f8");
	__asm__ __volatile__ ("abs.s $f8, $f10");

	__asm__ __volatile__ ("move %0, $0" :"=r"(counter) );
	__asm__ __volatile__ ("li %0, 1500" :"=r"(maxCounter) );

	BEGIN_TRANSACTION(3);
	__asm__ __volatile__ ("THREAD_TRANS_BRANCHES_:");
	__asm__ __volatile__ ("bgt %0, %1, END_THREAD_TRANS_BRANCHES_" : :"r"(counter), "r"(maxCounter) );
	__asm__ __volatile__ ("li %0, 13250" :"=r"(r_out_t0) );
	__asm__ __volatile__ ("sw  %1, 16(%0)    "	 : "=r"(g_data_out_int) : "r"(r_out_t0) : "memory" );
	__asm__ __volatile__ ("lw  %0, 16(%1)  "	 : "=r"(r_out_t3) : "r"(g_data_out_int) );
	__asm__ __volatile__ ("nop");
	__asm__ __volatile__ ("li %0, 18972" :"=r"(r_out_t1) );
	__asm__ __volatile__ ("sw  %1, 20(%0)    "	 : "=r"(g_data_out_int) : "r"(r_out_t1) : "memory" );
	__asm__ __volatile__ ("lw  %0, 20(%1)  "	 : "=r"(r_out_t4) : "r"(g_data_out_int) );
	__asm__ __volatile__ ("addi %0, 1" :"=r"(counter) );
	__asm__ __volatile__ ("j THREAD_TRANS_BRANCHES_");
	__asm__ __volatile__ ("END_THREAD_TRANS_BRANCHES_:");
	COMMIT_TRANSACTION(3);

	__asm__ __volatile__ ("abs.s $f8, $f10");
	__asm__ __volatile__ ("abs.s $f10, $f8");
	__asm__ __volatile__ ("abs.s $f8, $f10");

	__asm__ __volatile__ ("move %0, $0" :"=r"(counter) );
	__asm__ __volatile__ ("li %0, 10" :"=r"(maxCounter) );

	__asm__ __volatile__ ("YAR_:");
	__asm__ __volatile__ ("bgt %0, %1, END_YAR_" : :"r"(counter), "r"(maxCounter) );
	__asm__ __volatile__ ("addi %0, 1" :"=r"(counter) );
	__asm__ __volatile__ ("j YAR_");
	__asm__ __volatile__ ("END_YAR_:");
}

void boo2()
{
int i;
int que[1000];

	register size_t maxCounter asm("18");
	register size_t counter    asm("19");

	register int r_out_t0 asm("8");
	register int r_out_t1 asm("9");
	register int r_out_t2 asm("10");
	register int r_out_t3 asm("11");
	register int r_out_t4 asm("12");
	register int r_out_t5 asm("13");
	register int r_out_t6 asm("14");

	int* memInt;
	register int data_out_int asm("22");
	memInt          = (int*)malloc(sizeof(int) * 2048);
	data_out_int    = (int)&(memInt[0]);

	__asm__ __volatile__ ("abs.s $f8, $f10");
	__asm__ __volatile__ ("abs.s $f10, $f8");
	__asm__ __volatile__ ("abs.s $f8, $f10");

	BEGIN_TRANSACTION(3);
	__asm__ __volatile__ ("li %0, 13250" :"=r"(r_out_t0) );
	__asm__ __volatile__ ("sw  %1, 16(%0)    "	 : "=r"(data_out_int) : "r"(r_out_t0) : "memory" );
	__asm__ __volatile__ ("lw  %0, 16(%1)  "	 : "=r"(r_out_t3) : "r"(data_out_int) );
	__asm__ __volatile__ ("nop");
	__asm__ __volatile__ ("li %0, 18972" :"=r"(r_out_t1) );
	__asm__ __volatile__ ("sw  %1, 20(%0)    "	 : "=r"(data_out_int) : "r"(r_out_t1) : "memory" );
	__asm__ __volatile__ ("lw  %0, 20(%1)  "	 : "=r"(r_out_t4) : "r"(data_out_int) );
	COMMIT_TRANSACTION(3);

	__asm__ __volatile__ ("abs.s $f8, $f10");
	__asm__ __volatile__ ("abs.s $f10, $f8");
	__asm__ __volatile__ ("abs.s $f8, $f10");

	BEGIN_TRANSACTION(9);
	for(i = 0; i < 1000; i++)
		que[i] = i;

	COMMIT_TRANSACTION(9);
}

void boo3()
{
int i;
	register size_t maxCounter asm("18");
	register size_t counter    asm("19");

	register int r_out_t0 asm("8");
	register int r_out_t1 asm("9");
	register int r_out_t2 asm("10");
	register int r_out_t3 asm("11");
	register int r_out_t4 asm("12");
	register int r_out_t5 asm("13");
	register int r_out_t6 asm("14");

	int* memInt;
	register int data_out_int asm("22");
	memInt          = (int*)malloc(sizeof(int) * 2048);
	data_out_int    = (int)&(memInt[0]);

	__asm__ __volatile__ ("abs.s $f8, $f10");
	__asm__ __volatile__ ("abs.s $f10, $f8");
	__asm__ __volatile__ ("abs.s $f8, $f10");

	__asm__ __volatile__ ("abs.s $f8, $f10");
	__asm__ __volatile__ ("abs.s $f10, $f8");
	__asm__ __volatile__ ("abs.s $f8, $f10");
}
