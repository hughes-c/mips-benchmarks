TARGET = RADIOSITY

OBJS = rad_main.o patchman.o elemman.o taskman.o modelman.o smallobj.o\
         display.o visible.o rad_tools.o room_model.o 
HHS = model.H parallel.H patch.H radiosity.H task.H
HS = model.h parallel.h patch.h radiosity.h task.h

GLIBDIR = ./glibdumb
GLIB    = glib.a
# look me up in man sigfpe
LIBFPE = -lfpe

MACROS = ../../../../sesc/src/libapp/c.m4.sesc
CCOPTS = -I -float $(CFLAGS)
LDFLAGS = -O2 -lm -static -Wl,--script=../../../../sescutils/install/mipseb-linux/lib/ldscripts/mint.x /home/jpoe/default/obj/mipseb_obj/libapp.a /home/jpoe/sescutils/build-mipseb-linux/obj/gcc-build/mipseb-linux/libstdc++-v3/libmath/.libs/libmath.a
CFLAGS = $(CINCLUDE) -O2 -mips2 -mabi=32 -Wa,-non_shared -mno-abicalls -I../../../../sesc/src/libapp/
CINCLUDE = -I$(GLIBDIR) -I./glibps 

LOADLIBES =  $(GLIBDIR)/$(GLIB) glibps/glibps.a /home/jpoe/sescutils/build-mipseb-linux/obj/gcc-build/mipseb-linux/libstdc++-v3/libmath/.libs/libmath.a ../../../../default/obj/mipseb_obj/libapp.a /home/jpoe/sescutils/install/mipseb-linux/lib/libm.a

CC = mipseb-linux-gcc-3.4.4

x = *

$(TARGET): $(OBJS) 
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) $(LOADLIBES) -o $(TARGET)

.SUFFIXES:
.SUFFIXES:	.o .c .C .h .H

.H.h:	
	m4 -Ulen -Uindex ${MACROS} $*.H > $*.h

.C.c:	
	m4 -Ulen -Uindex $(MACROS) $*.C > $*.c

.c.o:	
	mipseb-linux-gcc-3.4.4 -c $(CFLAGS) $*.c

.C.o:	
	m4 -Ulen -Uindex $(MACROS) $*.C > $*.c
	mipseb-linux-gcc-3.4.4 -c $(CFLAGS) $*.c

$(HS): $(HHS)

radiosity.h: patch.h model.h parallel.h task.h
rad_main.c: rad_main.C $(HS)
patchman.c: patchman.C $(HS)
elemman.c: elemman.C $(HS)
taskman.c: taskman.C $(HS)
modelman.c: modelman.C $(HS)
smallobj.c: smallobj.C $(HS)
display.c: display.C $(HS)
visible.c: visible.C $(HS)
rad_tools.c: rad_tools.C $(HS)
room_model.c: room_model.C $(HS)

clean:
	$(RM) $(CS) $(ASMS) $(SS) $(OBJS) $(HS)
