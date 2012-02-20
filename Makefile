.PHONY: all, debug, clean, distclean, touch

CFLAGS = -Wall
INCLUDES = -Ilibrj
LIBS = -Llibrj -lrj -lncurses

all: debug1 =
all: debug2 =
all: touch librj/librj.a tgtd

debug: debug1 = -ggdb
debug: debug2 = debug
debug: touch librj/librj.a tgtd

tgtd: tgtd.c
	gcc $< -o $@ $(CFLAGS) $(debug1) -DNDEBUG $(INCLUDES) $(LIBS)

librj/librj.a: librj/rj.c
	make -C librj $(debug2)

librj/rj.c:
	rm -rf librj || true
	wget https://github.com/Yomin/librj/tarball/master -O librj.tar.gz
	tar -xzf librj.tar.gz
	mv Yomin-librj-* librj
	rm -f librj.tar.gz

clean:
	find -L . -maxdepth 1 ! -type d \( -perm -111 -or -name "*\.o" \) -exec rm {} \;
ifeq ($(shell [ -d librj ] && echo lib || echo nolib), lib)
	make -C librj clean
endif

distclean:
	find . ! -type d \( -perm -111 -or -name "*\.o" \) -exec rm {} \;
	rm -rf lib* || true

touch:
	$(shell [ -f debug -a -z "$(debug1)" ] && { touch tgtd.c; rm debug; rm lib*/*.a; })
	$(shell [ ! -f debug -a -n "$(debug1)" ] && { touch tgtd.c; touch debug; rm lib*/*.a; })
