.PHONY: all, clean

all: librj/librj.a tgtd

tgtd: tgtd.c
	gcc -Wall -L librj -l rj -o $@ $<

librj/librj.a: librj/librj.c
	make -C librj lib

librj/librj.c:
	rm -rf librj || true
	wget https://github.com/Yomin/librj/tarball/master -O librj.tar.gz
	tar -xzf librj.tar.gz
	mv Yomin-librj-* librj
	rm -f librj.tar.gz

clean:
	find . -maxdepth 1 ! -type d \( -perm -111 -or -name "*\.o" \) -exec rm {} \;
ifeq ($(shell [ -d librj ] && echo lib || echo nolib), lib)
	make -C librj clean
endif

distclean:
	find . ! -type d \( -perm -111 -or -name "*\.o" \) -exec rm {} \;
	rm -rf lib* || true
