	.file	1 "asmTest_threads.c"
	.section .mdebug.abi32
	.previous

 # -G value = 0, Arch = mips2, ISA = 2
 # GNU C version 3.4.4 20050120 (prerelease) (mipseb-linux)
 #	compiled by GNU C version 3.4.6 20060404 (Red Hat 3.4.6-8).
 # GGC heuristics: --param ggc-min-expand=100 --param ggc-min-heapsize=131072
 # options passed:  -I/home/cmhug/sescutils/libapp/ -mips2 -mabi=32
 # -mno-abicalls -auxbase -fverbose-asm
 # options enabled:  -feliminate-unused-debug-types -fpeephole
 # -ffunction-cse -fkeep-static-consts -fpcc-struct-return -fgcse-lm
 # -fgcse-sm -fgcse-las -fsched-interblock -fsched-spec
 # -fsched-stalled-insns -fsched-stalled-insns-dep -fbranch-count-reg
 # -fcommon -fverbose-asm -fargument-alias -fzero-initialized-in-bss
 # -fident -fmath-errno -ftrapping-math -mgas -meb -mbranch-likely
 # -mexplicit-relocs -mabi=32 -mips2 -mno-flush-func_flush_cache
 # -mflush-func=_flush_cache

	.rdata
	.align	2
$LC0:
	.ascii	"\n"
	.ascii	"Threads %d\n\000"
	.text
	.align	2
	.globl	main
	.ent	main
	.type	main, @function
main:
	.frame	$fp,40,$31		# vars= 8, regs= 4/0, args= 16, gp= 0
	.mask	0xc0300000,-4
	.fmask	0x00000000,0
	addiu	$sp,$sp,-40	 #,,
	sw	$31,36($sp)	 #,
	sw	$fp,32($sp)	 #,
	sw	$21,28($sp)	 #,
	sw	$20,24($sp)	 #,
	move	$fp,$sp	 #,
	sw	$4,40($fp)	 # argc, argc
	sw	$5,44($fp)	 # argv, argv
	lw	$2,44($fp)	 # argv, argv
	lw	$4,4($2)	 #,
	jal	atoi	 #
	sw	$2,20($fp)	 # tmp183, numThreads
	lui	$2,%hi(myLock)	 # tmp184,
	addiu	$4,$2,%lo(myLock)	 #, tmp184,
	jal	sesc_lock_init	 #
	lui	$2,%hi(myLock2)	 # tmp185,
	addiu	$4,$2,%lo(myLock2)	 #, tmp185,
	jal	sesc_lock_init	 #
	lui	$2,%hi(myLock3)	 # tmp186,
	addiu	$4,$2,%lo(myLock3)	 #, tmp186,
	jal	sesc_lock_init	 #
	lui	$2,%hi(myBarr)	 # tmp187,
	addiu	$4,$2,%lo(myBarr)	 #, tmp187,
	jal	sesc_barrier_init	 #
	lui	$2,%hi(myBarr2)	 # tmp188,
	addiu	$4,$2,%lo(myBarr2)	 #, tmp188,
	jal	sesc_barrier_init	 #
	lui	$2,%hi(myBarr3)	 # tmp189,
	addiu	$4,$2,%lo(myBarr3)	 #, tmp189,
	jal	sesc_barrier_init	 #
	lui	$2,%hi($LC0)	 # tmp190,
	addiu	$4,$2,%lo($LC0)	 #, tmp190,
	lw	$5,20($fp)	 # numThreads, numThreads
	jal	printf	 #
	lui	$2,%hi(stdout)	 # tmp191,
	lw	$4,%lo(stdout)($2)	 # stdout, stdout
	jal	fflush	 #
#APP
	.word 0x74000000+2
#NO_APP
	lw	$2,20($fp)	 # numThreads, numThreads
	blez	$2,$L3	 #, numThreads,
	jal	sesc_init	 #
	sw	$0,16($fp)	 #, j
$L4:
	lw	$2,20($fp)	 # numThreads, numThreads
	lw	$3,16($fp)	 # j, j
	sltu	$2,$3,$2	 # tmp195, j, numThreads
	beq	$2,$0,$L3	 #, tmp195,
	lui	$2,%hi(boo)	 # tmp196,
	addiu	$4,$2,%lo(boo)	 #, tmp196,
	move	$5,$0	 #,
	move	$6,$0	 #,
	jal	sesc_spawn	 #
	lw	$2,16($fp)	 # j, j
	addiu	$2,$2,1	 # tmp198, j,
	sw	$2,16($fp)	 # tmp198, j
	j	$L4	 #
$L3:
#APP
	move $21, $0	 # counter
	li $20, 1000	 # maxCounter
	BRANCHES_:
	bgt $21, $20, END_BRANCHES_	 # counter, maxCounter
	addi $21, 1	 # counter
	j BRANCHES_
	END_BRANCHES_:
#NO_APP
	lw	$2,20($fp)	 # numThreads, numThreads
	blez	$2,$L7	 #, numThreads,
	jal	sesc_wait	 #
$L7:
#APP
	LOCK1_:
#NO_APP
	lui	$2,%hi(myLock)	 # tmp200,
	addiu	$4,$2,%lo(myLock)	 #, tmp200,
	jal	sesc_lock	 #
#APP
	abs.s $f2, $f4
	abs.s $f4, $f6
	abs.s $f2, $f6
#NO_APP
	lui	$2,%hi(myLock)	 # tmp201,
	addiu	$4,$2,%lo(myLock)	 #, tmp201,
	jal	sesc_unlock	 #
#APP
	LOCK2_:
#NO_APP
	lui	$2,%hi(myLock2)	 # tmp202,
	addiu	$4,$2,%lo(myLock2)	 #, tmp202,
	jal	sesc_lock	 #
#APP
	addi $21, 6
	nop
	addi $21, 6
	nop
	addi $21, 6
#NO_APP
	lui	$2,%hi(myLock2)	 # tmp203,
	addiu	$4,$2,%lo(myLock2)	 #, tmp203,
	jal	sesc_unlock	 #
	lui	$2,%hi(myLock)	 # tmp204,
	addiu	$4,$2,%lo(myLock)	 #, tmp204,
	jal	sesc_lock	 #
#APP
	abs.s $f8, $f10
	abs.s $f10, $f8
	abs.s $f8, $f10
#NO_APP
	lui	$2,%hi(myLock)	 # tmp205,
	addiu	$4,$2,%lo(myLock)	 #, tmp205,
	jal	sesc_unlock	 #
#APP
	TRANS2_:
#NO_APP
	lui	$2,%hi(myLock)	 # tmp206,
	addiu	$4,$2,%lo(myLock)	 #, tmp206,
	jal	sesc_lock	 #
#APP
	move $21, $0	 # counter
	li $20, 1000	 # maxCounter
	TRANS_BRANCHES_:
	bgt $21, $20, END_TRANS_BRANCHES_	 # counter, maxCounter
	addi $21, 1	 # counter
	j TRANS_BRANCHES_
	END_TRANS_BRANCHES_:
#NO_APP
	lui	$2,%hi(myLock)	 # tmp207,
	addiu	$4,$2,%lo(myLock)	 #, tmp207,
	jal	sesc_unlock	 #
#APP
	END_:
	abs.s $f4, $f6
	abs.s $f6, $f4
	abs.s $f6, $f4
	j END_F_
	END_F_:
	.word 0x74000000+3
#NO_APP
	jal	sesc_wait	 #
	jal	sesc_wait	 #
	jal	sesc_wait	 #
	jal	sesc_wait	 #
	move	$4,$0	 #,
	jal	exit	 #
	.end	main
	.align	2
	.globl	boo
	.ent	boo
	.type	boo, @function
boo:
	.frame	$fp,16,$31		# vars= 0, regs= 3/0, args= 0, gp= 0
	.mask	0x400c0000,-8
	.fmask	0x00000000,0
	addiu	$sp,$sp,-16	 #,,
	sw	$fp,8($sp)	 #,
	sw	$19,4($sp)	 #,
	sw	$18,0($sp)	 #,
	move	$fp,$sp	 #,
#APP
	.word 0x70000000+3
	move $19, $0	 # counter
	li $18, 1000	 # maxCounter
	THREAD_TRANS_BRANCHES_:
	bgt $19, $18, END_THREAD_TRANS_BRANCHES_	 # counter, maxCounter
	addi $19, 1	 # counter
	j THREAD_TRANS_BRANCHES_
	END_THREAD_TRANS_BRANCHES_:
	.word 0x7C000000+3
	abs.s $f8, $f10
	abs.s $f10, $f8
	abs.s $f8, $f10
	.word 0x70000000+5
	move $19, $0	 # counter
	li $18, 1000	 # maxCounter
	THREAD_TRANS_BRANCHES2_:
	bgt $19, $18, END_THREAD_TRANS_BRANCHES2_	 # counter, maxCounter
	abs.s $f4, $f6
	addi $19, 1	 # counter
	j THREAD_TRANS_BRANCHES2_
	END_THREAD_TRANS_BRANCHES2_:
	.word 0x7C000000+5
#NO_APP
	move	$sp,$fp	 #,
	lw	$fp,8($sp)	 #,
	lw	$19,4($sp)	 #,
	lw	$18,0($sp)	 #,
	addiu	$sp,$sp,16	 #,,
	j	$31	 #
	.end	boo

	.comm	myLock,8,4

	.comm	myLock2,8,4

	.comm	myLock3,8,4

	.comm	myBarr,8,4

	.comm	myBarr2,8,4

	.comm	myBarr3,8,4
	.ident	"GCC: (GNU) 3.4.4 20050120 (prerelease)"
