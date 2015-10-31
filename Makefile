all: build/slave build/interactive.sh

try: all
	./build/interactive.sh

clean:
	rm build/*

build/slave: build/libslave.c enslaved.c
	gcc -std=gnu99 build/libslave.c enslaved.c -o build/slave

build/libslave.c: libslave.hbs.c
	hbs libslave.hbs.c enslaved.json *.js >build/libslave.c

build/interactive.sh: interactive.hbs.sh
	hbs interactive.hbs.sh enslaved.json *.js >build/interactive.sh
	chmod a+x build/interactive.sh
