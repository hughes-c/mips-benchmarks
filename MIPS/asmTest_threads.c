
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

	slock_t myLock;
	slock_t myLock2, myLock3;
	sbarrier_t myBarr;
	sbarrier_t myBarr2, myBarr3;

int main(int argc, char *argv[])
{
	size_t j;
	int numThreads = atoi(argv[1]);
	register int r_zero asm("0");

	sesc_lock_init(&myLock);
	sesc_lock_init(&myLock2);
	sesc_lock_init(&myLock3);
	sesc_barrier_init(&myBarr);
	sesc_barrier_init(&myBarr2);
	sesc_barrier_init(&myBarr3);

	printf("\nThreads %d\n", numThreads);
	fflush(stdout);
TOGGLE_ON(2);
	if(numThreads > 0)
	{
		sesc_init();
		for(j = 0; j < numThreads; j++)
			sesc_spawn(boo, NULL, 0);
	}

	register size_t maxCounter asm("20");
	register size_t counter    asm("21");
	__asm__ __volatile__ ("move %0, $0" :"=r"(counter) );
	__asm__ __volatile__ ("li %0, 1000" :"=r"(maxCounter) );
	__asm__ __volatile__ ("BRANCHES_:");
	__asm__ __volatile__ ("bgt %0, %1, END_BRANCHES_" : :"r"(counter), "r"(maxCounter) );
	__asm__ __volatile__ ("addi %0, 1" :"=r"(counter) );
	__asm__ __volatile__ ("j BRANCHES_");
	__asm__ __volatile__ ("END_BRANCHES_:");

	if(numThreads > 0)
// 		sesc_barrier(&myBarr, numThreads);
		sesc_wait();

	__asm__ __volatile__ ("LOCK1_:");
	sesc_lock(&myLock);
	__asm__ __volatile__ ("abs.s $f2, $f4");
	__asm__ __volatile__ ("abs.s $f4, $f6");
	__asm__ __volatile__ ("abs.s $f2, $f6");
	sesc_unlock(&myLock);

	__asm__ __volatile__ ("LOCK2_:");
	sesc_lock(&myLock2);
	__asm__ __volatile__ ("addi $21, 6");
	__asm__ __volatile__ ("nop");
	__asm__ __volatile__ ("addi $21, 6");
	__asm__ __volatile__ ("nop");
	__asm__ __volatile__ ("addi $21, 6");
	sesc_unlock(&myLock2);

// 	__asm__ __volatile__ ("TRANS1_:");
//         BEGIN_TRANSACTION(7);
sesc_lock(&myLock);
	__asm__ __volatile__ ("abs.s $f8, $f10");
	__asm__ __volatile__ ("abs.s $f10, $f8");
	__asm__ __volatile__ ("abs.s $f8, $f10");
//         COMMIT_TRANSACTION(7);
sesc_unlock(&myLock);

	__asm__ __volatile__ ("TRANS2_:");
//         BEGIN_TRANSACTION(5);
sesc_lock(&myLock);
	__asm__ __volatile__ ("move %0, $0" :"=r"(counter) );
	__asm__ __volatile__ ("li %0, 1000" :"=r"(maxCounter) );

	__asm__ __volatile__ ("TRANS_BRANCHES_:");
	__asm__ __volatile__ ("bgt %0, %1, END_TRANS_BRANCHES_" : :"r"(counter), "r"(maxCounter) );
	__asm__ __volatile__ ("addi %0, 1" :"=r"(counter) );
	__asm__ __volatile__ ("j TRANS_BRANCHES_");
	__asm__ __volatile__ ("END_TRANS_BRANCHES_:");
//         COMMIT_TRANSACTION(5);
sesc_unlock(&myLock);

	__asm__ __volatile__ ("END_:");

	__asm__ __volatile__ ("abs.s $f4, $f6");
	__asm__ __volatile__ ("abs.s $f6, $f4");
	__asm__ __volatile__ ("abs.s $f6, $f4");
	__asm__ __volatile__ ("j END_F_");


	__asm__ __volatile__ ("END_F_:");

TOGGLE_ON(3);
sesc_wait();
sesc_wait();
sesc_wait();
sesc_wait();

	exit(0);
}

void boo()
{
	register size_t maxCounter asm("18");
	register size_t counter    asm("19");

// 	sesc_lock(&myLock3);
	BEGIN_TRANSACTION(3);
	__asm__ __volatile__ ("move %0, $0" :"=r"(counter) );
	__asm__ __volatile__ ("li %0, 1000" :"=r"(maxCounter) );

	__asm__ __volatile__ ("THREAD_TRANS_BRANCHES_:");
	__asm__ __volatile__ ("bgt %0, %1, END_THREAD_TRANS_BRANCHES_" : :"r"(counter), "r"(maxCounter) );
	__asm__ __volatile__ ("addi %0, 1" :"=r"(counter) );
	__asm__ __volatile__ ("j THREAD_TRANS_BRANCHES_");
	__asm__ __volatile__ ("END_THREAD_TRANS_BRANCHES_:");

	COMMIT_TRANSACTION(3);
// 	sesc_unlock(&myLock3);

	__asm__ __volatile__ ("abs.s $f8, $f10");
	__asm__ __volatile__ ("abs.s $f10, $f8");
	__asm__ __volatile__ ("abs.s $f8, $f10");

// 	sesc_lock(&myLock3);
	BEGIN_TRANSACTION(5);
	__asm__ __volatile__ ("move %0, $0" :"=r"(counter) );
	__asm__ __volatile__ ("li %0, 1000" :"=r"(maxCounter) );

	__asm__ __volatile__ ("THREAD_TRANS_BRANCHES2_:");
	__asm__ __volatile__ ("bgt %0, %1, END_THREAD_TRANS_BRANCHES2_" : :"r"(counter), "r"(maxCounter) );
	__asm__ __volatile__ ("abs.s $f4, $f6");
	__asm__ __volatile__ ("addi %0, 1" :"=r"(counter) );
	__asm__ __volatile__ ("j THREAD_TRANS_BRANCHES2_");
	__asm__ __volatile__ ("END_THREAD_TRANS_BRANCHES2_:");

	COMMIT_TRANSACTION(5);

}
