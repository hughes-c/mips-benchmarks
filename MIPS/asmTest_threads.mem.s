	.file	1 "asmTest_threads.mem.c"
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

	.text
	.align	2
	.globl	main
	.ent	main
	.type	main, @function
main:
	.frame	$fp,56,$31		# vars= 16, regs= 6/0, args= 16, gp= 0
	.mask	0xc0710000,-4
	.fmask	0x00000000,0
	addiu	$sp,$sp,-56	 #,,
	sw	$31,52($sp)	 #,
	sw	$fp,48($sp)	 #,
	sw	$22,44($sp)	 #,
	sw	$21,40($sp)	 #,
	sw	$20,36($sp)	 #,
	sw	$16,32($sp)	 #,
	move	$fp,$sp	 #,
	sw	$4,56($fp)	 # argc, argc
	sw	$5,60($fp)	 # argv, argv
	lw	$2,60($fp)	 # argv, argv
	lw	$4,4($2)	 #,
	jal	atoi	 #
	move	$3,$2	 # tmp183,
	lui	$2,%hi(numThreads)	 # tmp184,
	sw	$3,%lo(numThreads)($2)	 # tmp183, numThreads
	li	$4,8192			# 0x2000	 #,
	jal	malloc	 #
	sw	$2,20($fp)	 # tmp185, memInt
	lw	$22,20($fp)	 # data_out_int, memInt
	lui	$16,%hi(gblobalMemInt)	 # tmp186,
	li	$4,8192			# 0x2000	 #,
	jal	malloc	 #
	sw	$2,%lo(gblobalMemInt)($16)	 # tmp187, gblobalMemInt
	lui	$2,%hi(gblobalMemInt)	 # tmp188,
	lw	$23,%lo(gblobalMemInt)($2)	 # g_data_out_int, gblobalMemInt
	sw	$22,24($fp)	 # data_out_int, int_base
	sw	$23,28($fp)	 # g_data_out_int, g_int_base
	lui	$2,%hi(myLock)	 # tmp189,
	addiu	$4,$2,%lo(myLock)	 #, tmp189,
	jal	sesc_lock_init	 #
	lui	$2,%hi(myLock2)	 # tmp190,
	addiu	$4,$2,%lo(myLock2)	 #, tmp190,
	jal	sesc_lock_init	 #
	lui	$2,%hi(myLock3)	 # tmp191,
	addiu	$4,$2,%lo(myLock3)	 #, tmp191,
	jal	sesc_lock_init	 #
	lui	$2,%hi(myBarr)	 # tmp192,
	addiu	$4,$2,%lo(myBarr)	 #, tmp192,
	jal	sesc_barrier_init	 #
	lui	$2,%hi(myBarr2)	 # tmp193,
	addiu	$4,$2,%lo(myBarr2)	 #, tmp193,
	jal	sesc_barrier_init	 #
	lui	$2,%hi(myBarr3)	 # tmp194,
	addiu	$4,$2,%lo(myBarr3)	 #, tmp194,
	jal	sesc_barrier_init	 #
#APP
	.word 0x74000000+2
#NO_APP
	lui	$2,%hi(numThreads)	 # tmp195,
	lw	$2,%lo(numThreads)($2)	 # numThreads, numThreads
	blez	$2,$L3	 #, numThreads,
	jal	sesc_init	 #
	sw	$0,16($fp)	 #, j
$L4:
	lui	$2,%hi(numThreads)	 # tmp197,
	lw	$3,%lo(numThreads)($2)	 # numThreads, numThreads
	lw	$2,16($fp)	 # j, j
	sltu	$2,$2,$3	 # tmp200, j, numThreads
	beq	$2,$0,$L3	 #, tmp200,
	lui	$2,%hi(boo)	 # tmp201,
	addiu	$4,$2,%lo(boo)	 #, tmp201,
	move	$5,$0	 #,
	move	$6,$0	 #,
	jal	sesc_spawn	 #
	lw	$2,16($fp)	 # j, j
	addiu	$2,$2,1	 # tmp203, j,
	sw	$2,16($fp)	 # tmp203, j
	j	$L4	 #
$L3:
	lui	$2,%hi(boo2)	 # tmp204,
	addiu	$4,$2,%lo(boo2)	 #, tmp204,
	move	$5,$0	 #,
	move	$6,$0	 #,
	jal	sesc_spawn	 #
#APP
	move $21, $0	 # counter
	li $20, 1000	 # maxCounter
	BRANCHES_:
	bgt $21, $20, END_BRANCHES_	 # counter, maxCounter
	addi $21, 1	 # counter
	j BRANCHES_
	END_BRANCHES_:
	LOCK1_:
#NO_APP
	lui	$2,%hi(myLock)	 # tmp205,
	addiu	$4,$2,%lo(myLock)	 #, tmp205,
	jal	sesc_lock	 #
#APP
	mfhi  $11	 # r_out_t3
	mtc1 $8, $f8  	 # r_out_t0
	abs.s $f2, $f4
	abs.s $f4, $f6
	abs.s $f2, $f6
#NO_APP
	lui	$2,%hi(myLock)	 # tmp206,
	addiu	$4,$2,%lo(myLock)	 #, tmp206,
	jal	sesc_unlock	 #
#APP
	LOCK2_:
#NO_APP
	lui	$2,%hi(myLock2)	 # tmp207,
	addiu	$4,$2,%lo(myLock2)	 #, tmp207,
	jal	sesc_lock	 #
#APP
	li $8, 12350	 # r_out_t0
	sw  $8, 16($22)    	 # r_out_t0, data_out_int
	lw  $11, 16($22)  	 # r_out_t3, data_out_int
	nop
	li $9, 17352	 # r_out_t1
	sw  $9, 20($22)    	 # r_out_t1, data_out_int
	lw  $12, 20($22)  	 # r_out_t4, data_out_int
#NO_APP
	lui	$2,%hi(myLock2)	 # tmp208,
	addiu	$4,$2,%lo(myLock2)	 #, tmp208,
	jal	sesc_unlock	 #
#APP
	.word 0x70000000+7
	abs.s $f8, $f10
	abs.s $f10, $f8
	abs.s $f8, $f10
	.word 0x7C000000+7
	TRANS2_:
	.word 0x70000000+5
	move $21, $0	 # counter
	li $20, 1000	 # maxCounter
	TRANS_BRANCHES_:
	bgt $21, $20, END_TRANS_BRANCHES_	 # counter, maxCounter
	addi $21, 1	 # counter
	j TRANS_BRANCHES_
	END_TRANS_BRANCHES_:
	.word 0x7C000000+5
	END_:
	abs.s $f4, $f6
	abs.s $f6, $f4
	abs.s $f6, $f4
	j END_F_
	END_F_:
	.word 0x74000000+3
#NO_APP
	sw	$0,16($fp)	 #, j
$L12:
	lui	$2,%hi(numThreads)	 # tmp209,
	lw	$2,%lo(numThreads)($2)	 # numThreads, numThreads
	addiu	$3,$2,1	 # tmp210, numThreads,
	lw	$2,16($fp)	 # j, j
	sltu	$2,$2,$3	 # tmp213, j, tmp210
	beq	$2,$0,$L13	 #, tmp213,
	jal	sesc_wait	 #
	lw	$2,16($fp)	 # j, j
	addiu	$2,$2,1	 # tmp215, j,
	sw	$2,16($fp)	 # tmp215, j
	j	$L12	 #
$L13:
	move	$4,$0	 #,
	jal	exit	 #
	.end	main
	.align	2
	.globl	boo
	.ent	boo
	.type	boo, @function
boo:
	.frame	$fp,48,$31		# vars= 8, regs= 5/0, args= 16, gp= 0
	.mask	0xc04c0000,-8
	.fmask	0x00000000,0
	addiu	$sp,$sp,-48	 #,,
	sw	$31,40($sp)	 #,
	sw	$fp,36($sp)	 #,
	sw	$22,32($sp)	 #,
	sw	$19,28($sp)	 #,
	sw	$18,24($sp)	 #,
	move	$fp,$sp	 #,
	li	$4,8192			# 0x2000	 #,
	jal	malloc	 #
	sw	$2,16($fp)	 # tmp181, memInt
#APP
	abs.s $f8, $f10
	abs.s $f10, $f8
	abs.s $f8, $f10
	move $19, $0	 # counter
	li $18, 1500	 # maxCounter
	.word 0x70000000+3
	THREAD_TRANS_BRANCHES_:
	bgt $19, $18, END_THREAD_TRANS_BRANCHES_	 # counter, maxCounter
	li $8, 13250	 # r_out_t0
	sw  $8, 16($23)    	 # r_out_t0, g_data_out_int
	lw  $11, 16($23)  	 # r_out_t3, g_data_out_int
	nop
	li $9, 18972	 # r_out_t1
	sw  $9, 20($23)    	 # r_out_t1, g_data_out_int
	lw  $12, 20($23)  	 # r_out_t4, g_data_out_int
	addi $19, 1	 # counter
	j THREAD_TRANS_BRANCHES_
	END_THREAD_TRANS_BRANCHES_:
	.word 0x7C000000+3
	abs.s $f8, $f10
	abs.s $f10, $f8
	abs.s $f8, $f10
	move $19, $0	 # counter
	li $18, 10	 # maxCounter
	YAR_:
	bgt $19, $18, END_YAR_	 # counter, maxCounter
	addi $19, 1	 # counter
	j YAR_
	END_YAR_:
#NO_APP
	move	$sp,$fp	 #,
	lw	$31,40($sp)	 #,
	lw	$fp,36($sp)	 #,
	lw	$22,32($sp)	 #,
	lw	$19,28($sp)	 #,
	lw	$18,24($sp)	 #,
	addiu	$sp,$sp,48	 #,,
	j	$31	 #
	.end	boo
	.align	2
	.globl	boo2
	.ent	boo2
	.type	boo2, @function
boo2:
	.frame	$fp,4048,$31		# vars= 4016, regs= 3/0, args= 16, gp= 0
	.mask	0xc0400000,-8
	.fmask	0x00000000,0
	addiu	$sp,$sp,-4048	 #,,
	sw	$31,4040($sp)	 #,
	sw	$fp,4036($sp)	 #,
	sw	$22,4032($sp)	 #,
	move	$fp,$sp	 #,
	li	$4,8192			# 0x2000	 #,
	jal	malloc	 #
	sw	$2,4024($fp)	 # tmp181, memInt
#APP
	abs.s $f8, $f10
	abs.s $f10, $f8
	abs.s $f8, $f10
	.word 0x70000000+3
	li $8, 13250	 # r_out_t0
	sw  $8, 16($22)    	 # r_out_t0, data_out_int
	lw  $11, 16($22)  	 # r_out_t3, data_out_int
	nop
	li $9, 18972	 # r_out_t1
	sw  $9, 20($22)    	 # r_out_t1, data_out_int
	lw  $12, 20($22)  	 # r_out_t4, data_out_int
	.word 0x7C000000+3
	abs.s $f8, $f10
	abs.s $f10, $f8
	abs.s $f8, $f10
	.word 0x70000000+9
#NO_APP
	sw	$0,16($fp)	 #, i
$L22:
	lw	$2,16($fp)	 # i, i
	slt	$2,$2,1000	 # tmp183, i,
	beq	$2,$0,$L23	 #, tmp183,
	lw	$2,16($fp)	 # i, i
	sll	$3,$2,2	 # tmp187, i,
	addiu	$2,$fp,16	 # tmp191,,
	addu	$3,$3,$2	 # tmp185, tmp187, tmp191
	lw	$2,16($fp)	 # i, i
	sw	$2,8($3)	 # i, que
	lw	$2,16($fp)	 # i, i
	addiu	$2,$2,1	 # tmp190, i,
	sw	$2,16($fp)	 # tmp190, i
	j	$L22	 #
$L23:
#APP
	.word 0x7C000000+9
#NO_APP
	move	$sp,$fp	 #,
	lw	$31,4040($sp)	 #,
	lw	$fp,4036($sp)	 #,
	lw	$22,4032($sp)	 #,
	addiu	$sp,$sp,4048	 #,,
	j	$31	 #
	.end	boo2
	.align	2
	.globl	boo3
	.ent	boo3
	.type	boo3, @function
boo3:
	.frame	$fp,40,$31		# vars= 8, regs= 3/0, args= 16, gp= 0
	.mask	0xc0400000,-8
	.fmask	0x00000000,0
	addiu	$sp,$sp,-40	 #,,
	sw	$31,32($sp)	 #,
	sw	$fp,28($sp)	 #,
	sw	$22,24($sp)	 #,
	move	$fp,$sp	 #,
	li	$4,8192			# 0x2000	 #,
	jal	malloc	 #
	sw	$2,20($fp)	 # tmp181, memInt
#APP
	abs.s $f8, $f10
	abs.s $f10, $f8
	abs.s $f8, $f10
	abs.s $f8, $f10
	abs.s $f10, $f8
	abs.s $f8, $f10
#NO_APP
	move	$sp,$fp	 #,
	lw	$31,32($sp)	 #,
	lw	$fp,28($sp)	 #,
	lw	$22,24($sp)	 #,
	addiu	$sp,$sp,40	 #,,
	j	$31	 #
	.end	boo3

	.comm	myLock,8,4

	.comm	myLock2,8,4

	.comm	myLock3,8,4

	.comm	myBarr,8,4

	.comm	myBarr2,8,4

	.comm	myBarr3,8,4

	.comm	gblobalMemInt,4,4

	.comm	numThreads,4,4
	.ident	"GCC: (GNU) 3.4.4 20050120 (prerelease)"
