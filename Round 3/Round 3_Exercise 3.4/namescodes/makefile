PROGRAM = namescodes

SRCS    =       namescodes.c error.c
OBJS    =       namescodes.o error.o

CC      =       gcc
LINKER  =       gcc

#64 bits mode:
CFLAGS = -Wall -O3 -fomit-frame-pointer -c -std=gnu99 -ffast-math
#32 bits mode:
#CFLAGS  = -Wall -O3 -fomit-frame-pointer -c -std=gnu99 -ffast-math -m32

#64 bits mode:
LDFLAGS        =
#32 bits mode:
#LDFLAGS = -m32
LIBS    = -lm
#DFLAGS = -O0 -g -std=gnu99 -lefence
DFLAGS  = -O0 -g -std=gnu99
GFLAGS = -O3 -pg -std=gnu99 -ffast-math

COMPILE = $(CC) $(CFLAGS)

LINK    = $(LINKER) $(OBJS) $(LDFLAGS) $(LIBS)
all: $(PROGRAM)

$(PROGRAM): $(OBJS)
	$(LINK) -o $(PROGRAM)

depend:
	makedepend -Y -- $(CFLAGS) -- $(SRCS)

.c.o$<:
	$(COMPILE) $*.c

clean:
	rm -f *.o $(PROGRAM) core



# DO NOT DELETE
namescodes.o: error.h 
apufunktiot.o: error.h
error.o: error.h
