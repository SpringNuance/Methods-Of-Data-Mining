PROGRAM	= kingfisher


SRCS	=	kingfisher.c trie.c bitmatrice.c measures.c apufunktiot.c error.c glob.c brules.c constr.c
OBJS	=	kingfisher.o trie.o bitmatrice.o measures.o apufunktiot.o error.o glob.o brules.o constr.o

CC	=	gcc
LINKER	=	gcc

#IF YOU WANT TO USE 32 BITS MODE: MAKE CHANGES HERE:
#64 bits mode:
CFLAGS	= -Wall -Wno-unused-but-set-variable -O3 -fomit-frame-pointer -c -std=gnu99 -ffast-math -fno-finite-math-only -fcommon

#32 bits mode:
#CFLAGS	= -Wall -Wno-unused-but-set-variable -O3 -fomit-frame-pointer -c -std=gnu99 -ffast-math -fno-finite-math-only -m32 -mpopcnt 

#64 bits mode:
LDFLAGS	=
#32 bits mode:
#LDFLAGS	= -m32

LIBS	= -lm
#DFLAGS	= -O0 -g -std=gnu99 -lefence
DFLAGS	= -O0 -g -std=gnu99

GFLAGS = -O3 -pg -std=gnu99 -ffast-math -fno-finite-math-only


COMPILE	= $(CC) $(CFLAGS)

LINK	= $(LINKER) $(OBJS) $(LDFLAGS) $(LIBS)

all: $(PROGRAM)

$(PROGRAM): $(OBJS)
	$(LINK) -o $(PROGRAM)

depend:
	makedepend -Y -- $(CFLAGS) -- $(SRCS)

.c.o$<:
	$(COMPILE) $*.c

clean:
	rm -f *.o $(PROGRAM) core

#for debugging 
bug:	
	$(CC) $(DFLAGS) kingfisher.c trie.c bitmatrice.c measures.c apufunktiot.c error.c glob.c brules.c constr.c -o $(PROGRAM) $(LIBS)

analysis:
	$(CC) $(GFLAGS) kingfisher.c trie.c bitmatrice.c measures.c apufunktiot.c error.c glob.c brules.c constr.c -o $(PROGRAM) $(LIBS)


# DO NOT DELETE

kingfisher.o: bitmatrice.h error.h ibitvector.h trie.h measures.h apufunktiot.h
kingfisher.o: glob.h brules.h constr.h
trie.o: glob.h trie.h bitmatrice.h error.h ibitvector.h measures.h
trie.o: apufunktiot.h
bitmatrice.o: bitmatrice.h error.h ibitvector.h
measures.o: measures.h trie.h bitmatrice.h error.h ibitvector.h glob.h
apufunktiot.o: error.h
error.o: error.h
glob.o: glob.h trie.h bitmatrice.h error.h ibitvector.h
brules.o: measures.h trie.h bitmatrice.h error.h ibitvector.h brules.h glob.h
constr.o: constr.h
