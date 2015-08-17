PRGNAME=redis-server.a
CCOPT= -std=c99 -pedantic -O2 -Wall -W -lxenstore
DEBUG=-g -rdynamic -ggdb
OBJ= adlist.o ae.o anet.o dict.o redis.o sds.o zmalloc.o lzf_c.o lzf_d.o pqsort.o zipmap.o sha1.o
CCLINK= -lm
all: redis-server.a

# Deps (use make dep to generate this)

adlist.o: adlist.c adlist.h zmalloc.h
ae.o: ae.c ae.h zmalloc.h config.h ae_kqueue.c ae_epoll.c
ae_kqueue.o: ae_kqueue.c
anet.o: anet.c fmacros.h anet.h
dict.o: dict.c fmacros.h dict.h zmalloc.h
redis.o: redis.c fmacros.h config.h redis.h ae.h sds.h anet.h dict.h \
  adlist.h zmalloc.h lzf.h pqsort.h zipmap.h staticsymbols.h sha1.h release.h
sds.o: sds.c sds.h zmalloc.h
zmalloc.o: zmalloc.c config.h
lzf_c.o: lzf_c.c lzfP.h
lzf_d.o: lzf_d.c lzfP.h
pqsort.o: pqsort.c
zipmap.o: zipmap.c zmalloc.h
sha1.o:sha1.c sha1.h


redis-server.a: $(OBJ)
	$(AR) cr $@ $^	
clean:
	rm -rf $(PRGNAME) *.o *.gcda *.gcno *.gcov
