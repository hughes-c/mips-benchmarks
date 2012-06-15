TARGET = FFT
OBJS = fft.o
CFLAGS = -O2 -w -mips2 -mabi=32 -Wa,-non_shared -mno-abicalls -I/home/jpoe/sesc/src/libapp/
LDFLAGS = -lm -static -Wl,--script=/home/jpoe/sescutils/install/mipseb-linux/lib/ldscripts/mint.x /home/jpoe/default/obj/mipseb_obj/libapp.a
MACROS = /home/jpoe/sesc/src/libapp/c.m4.sesc

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

clean:
	rm -f $(TARGET) $(OBJS) $(CFILES)
