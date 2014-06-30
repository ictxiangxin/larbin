CC = gcc
RANLIB = ranlib
AR = ar
CFLAGS = $(AUTOCFLAGS) $(WARNS) $(WERROR) $(DIRCFLAGS) $(XCFLAGS)
LDFLAGS = $(AUTOLDFLAGS) $(DIRLDFLAGS) $(XLDFLAGS)
LDLIBS = $(XLIBS)
AUTOCFLAGS = -g -O2
WARNS = -Wall -Wmissing-prototypes -Wwrite-strings -Wstrict-prototypes -Wcast-qual -Wpointer-arith -Wno-pointer-sign
#WERROR = -Werror

TARGETS = libadns.a

LIBOBJS = types.o event.o query.o reply.o general.o setup.o transmit.o parse.o poll.o check.o

ALLOBJS = $(LIBOBJS)

all: $(TARGETS)

libadns.a: $(LIBOBJS)
	rm -f $@
	$(AR) cqv $@.new $(LIBOBJS)
	$(RANLIB) $@.new
	mv -f $@.new $@

$(LIBOBJS):	adns.h internal.h config.h

clean:
	rm -f *.o

distclean: clean
	rm -f $(TARGETS)
