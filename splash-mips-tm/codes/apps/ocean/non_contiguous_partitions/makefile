TARGET = OCEAN
OBJS = main.o jacobcalc.o laplacalc.o \
	slave1.o slave2.o multi.o
CFLAGS = -O2 -w -mips2 -mabi=32 -Wa,-non_shared -mno-abicalls -I../../../../../sesc/src/libapp/ 
LDFLAGS = -lm -static -Wl,--script=../../../../sescutils/install/mipseb-linux/lib/ldscripts/mint.x ../../../../../default/obj/mipseb_obj/libapp.a
MACROS = ../../../../../sesc/src/libapp/c.m4.sesc

x = *

$(TARGET): $(OBJS)
	mipseb-linux-gcc-3.4.4 $(OBJS) -o $(TARGET) $(LDFLAGS)

.SUFFIXES:
.SUFFIXES:	.o .c .C .h .H

.H.h: 
	m4 ${MACROS} $*.H > $*.h

.C.c: 
	m4 $(MACROS) $*.C > $*.c

.c.o: 
	mipseb-linux-gcc-3.4.4 -c $(CFLAGS) $*.c

.C.o: 
	m4 $(MACROS) $*.C > $*.c
	mipseb-linux-gcc-3.4.4 -c $(CFLAGS) $*.c

decs.h: decs.H
jacobcalc.c: decs.h
main.c: decs.h
slave1.c: decs.h
laplacalc.c: decs.h
multi.c : decs.h
slave2.c: decs.h

