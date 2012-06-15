# ==============================================================================
#
# Defines.common.mk
#
# ==============================================================================


CC       := mipseb-linux-gcc-3.4.4
#CFLAGS   += -g -Wall -pthread
CFLAGS   += -mips2 -mabi=32 -Wa,-non_shared -mno-abicalls -I/research/libapp_test/
CFLAGS	 += -static -Wl,--script=/home/jpoe/sescutils/install/mipseb-linux/lib/ldscripts/mint.x 
# CFLAGS	 += /home/jpoe/default/obj/mipseb_obj/sesc_locks.o  /home/jpoe/default/obj/mipseb_obj/sesc_thread.o
# CFLAGS	 += /home/jpoe/default/obj/mipseb_obj/sesc_events.o  /home/jpoe/default/obj/mipseb_obj/sesc_chud.o
# CFLAGS	 += ../lib/bitmap.o  ../lib/hashtable.o  ../lib/list.o  ../lib/memory.o  ../lib/mt19937ar.o  ../lib/pair.o  
# CFLAGS	 += ../lib/queue.o  ../lib/random.o  ../lib/thread.o
CFLAGS   += -O0
CFLAGS   += -I$(LIB)
CPP      := mipseb-linux-g++
CPPFLAGS += $(CFLAGS)
LD       := mipseb-linux-g++
LIBS     += -lpthread

# Remove these files when doing clean
OUTPUT +=

LIB := ../lib

#STM := ../../tl2

#LOSTM := ../../OpenTM/lostm


# ==============================================================================
#
# End of Defines.common.mk
#
# ==============================================================================
