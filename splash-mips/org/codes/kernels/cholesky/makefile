TARGET = CHOLESKY
OBJS = amal.o assign.o bfac.o bksolve.o block2.o fo.o mf.o numLL.o \
     parts.o seg.o solve.o tree.o util.o malloc.o 

CFLAGS = -O2 -w -mips2 -mabi=32 -Wa,-non_shared -mno-abicalls -I../../../../sesc/src/libapp/
LDFLAGS = -lm -static -Wl,--script=../../../../sescutils/install/mipseb-linux/lib/ldscripts/mint.x ../../../../default/obj/mipseb_obj/libapp.a
MACROS = ../../../../sesc/src/libapp/c.m4.sesc


x = *

$(TARGET): $(OBJS)
	mipseb-linux-gcc-3.4.4 $(OBJS) $(CFLAGS) -o $(TARGET) $(LDFLAGS)

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

matrix.h: matrix.H
amal.c: matrix.h
block2.c: matrix.h
numLL.c: matrix.h
tree.c: matrix.h
assign.c: matrix.h
fo.c: matrix.h
parts.c: matrix.h
util.c: matrix.h
bfac.c: matrix.h
malloc.c: matrix.h
seg.c: matrix.h
bksolve.c: matrix.h
mf.c: matrix.h
solve.c: matrix.h

