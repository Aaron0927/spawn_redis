XEN_ROOT = $(CURDIR)/../..

include $(XEN_ROOT)/Config.mk

CLIPRGNAME = redis-cli.a
CCOPT=-std=c99 -pedantic -O2 -Wall -W
DEBUG=-g -rdynamic -ggdb
CLIOBJ = anet.o sds.o adlist.o redis-cli.o zmalloc.o linenoise.o
CCLINK=-lm
all: redis-cli.a

# Deps (use make dep to generate this)

anet.o: anet.c fmacros.h anet.h
sds.o: sds.c sds.h zmalloc.h
adlist.o: adlist.c adlist.h zmalloc.h
redis-cli.o: redis-cli.c fmacros.h anet.h sds.h adlist.h zmalloc.h linenoise.h
zmalloc.o: zmalloc.c config.h
linenoise.o: linenoise.c fmacros.h

redis-cli.a: $(CLIOBJ)
	$(AR) cr $@ $^
clean:
	rm -rf $(CLIPRGNAME) *.o *.gcad *.gcno *.gcov
