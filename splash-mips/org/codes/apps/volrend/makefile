TARGET = VOLREND
OBJS = main.o file.o option.o map.o normal.o opacity.o octree.o view.o \
       render.o adaptive.o raytrace.o
CFLAGS = -O2 -DFLIP -DPRESHD $(CDEBUG) -Ilibtiff -mips2 -mabi=32 -Wa,-non_shared -mno-abicalls -I../../../../sesc/src/libapp/
CCOPTS = -S -I -I$(SIMDIR) -I$(IDIR) -I/usr/include -DAUG $(CFLAGS)
LDFLAGS = -O2 -lm -Ilibtiff -Llibtiff -ltiff -static -Wl,--script=../../../../sescutils/install/mipseb-linux/lib/ldscripts/mint.x ../../../../default/obj/mipseb_obj/libapp.a
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

main.c:	main.C incl.h anl.h
file.c:	file.C incl.h
option.c: option.C incl.h
map.c: map.C incl.h
normal.c: normal.C incl.h anl.h address.h
opacity.c: opacity.C incl.h anl.h
octree.c: octree.C incl.h anl.h
view.c:	view.C incl.h
render.c: render.C incl.h
adaptive.c: adaptive.C incl.h anl.h
raytrace.c: raytrace.C incl.h address.h

incl.h:	user_options.h const.h my_types.h global.h macros.h address.h
