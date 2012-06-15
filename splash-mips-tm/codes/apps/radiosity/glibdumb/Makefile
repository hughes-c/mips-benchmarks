# 
#  Device independent graphics package GLIB.
#  NULL graphic device version
#

OBJS   = glib.o
TARGET = glib.a
PANELDIR = /usr/local/panel
CCOPT  = -O 
INCLUDE_DIRS = 
CFLAGS = $(CCOPT) $(CCDEFS) $(INCLUDE_DIRS) -mips2 -mabi=32 -Wa,-non_shared -mno-abicalls -I../../../../sesc/src/libapp/
LDFLAGS =  -L${PANELDIR}/lib -lgl -lpanel -lm -static -Wl,--script=../../../../../sescutils/install/mipseb-linux/lib/ldscripts/mint.x ../../../../../default/obj/mipseb_obj/libapp.a /home/jpoe/sescutils/build-mipseb-linux/obj/gcc-build/mipseb-linux/libstdc++-v3/libmath/.libs/libmath.a


all:	$(TARGET)

clean:
	touch dummy~
	rm -f core *.o *~ $(TARGET)

lint:
	lint $(INCLUDE_DIRS) $(SRCS)


$(TARGET): glib.o
	mipseb-linux-ar crv $(TARGET) glib.o

testlib:
	mipseb-linux-gcc-3.4.4 -O -o testlib testlib.c $(TARGET) $(LDFLAGS)

