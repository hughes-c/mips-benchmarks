# 
#  CS348C  Radiosity     Device independent graphics package GLIB.
#                        SUN/Xview version makefile
#
#

OBJS   = glibps.o
TARGET = glibps.a

INCLUDE_DIRS =

CCOPT  = -O 
CFLAGS = $(CCOPT) $(CCDEFS) $(INCLUDE_DIRS) -mips2 -mabi=32 -Wa,-non_shared -mno-abicalls -I../../../../../sesc/src/libapp/
LDFLAGS =  -lm -static -Wl,--script=../../../../../sescutils/install/mipseb-linux/lib/ldscripts/mint.x ../../../../../default/obj/mipseb_obj/libapp.a /home/jpoe/sescutils/build-mipseb-linux/obj/gcc-build/mipseb-linux/libstdc++-v3/libmath/.libs/libmath.a


all:	$(TARGET)

clean:
	touch dummy~
	rm -f core *.o *~ $(TARGET)

lint:
	lint $(INCLUDE_DIRS) $(SRCS)


$(TARGET): $(OBJS)
	mipseb-linux-ar crv $(TARGET) $(OBJS)
#	ranlib $(TARGET)

testlib:
	mipseb-linux-gcc-3.4.4 -g -o testlib testlib.c $(TARGET) $(LDFLAGS)

