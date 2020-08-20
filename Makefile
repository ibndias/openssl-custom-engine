all: md5-engine.o rfc1321/md5c.o md5test.c
	gcc -shared -o md5-engine.so -lcrypto md5-engine.o rfc1321/md5c.o
	gcc -o md5test md5test.c -lcrypto

install: md5-engine.so
	cp md5-engine.so /usr/lib/x86_64-linux-gnu/engines-1.1/md5-engine.so

rfc1321/md5c.o:
	gcc -fPIC -o rfc1321/md5c.o -c rfc1321/md5c.c

md5-engine.o:
	gcc -fPIC -o md5-engine.o -c md5-engine.c

clean:
	rm -rf *.o *.so rfc1321/*.o