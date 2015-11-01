all: build/slave_test build/interactive.sh

try: all
	./build/interactive.sh

clean:
	rm build/*

build/libslave.c: libslave.hbs.c
	./libslavegen slave_test.json *.js >build/libslave.c

build/slave_test: build/libslave.c slave_test.c
	gcc -std=gnu99 build/libslave.c slave_test.c -o build/slave_test

build/interactive.sh: interactive.hbs.sh build/slave_test
	./libslavegen -i slave_test.json *.js >build/interactive.sh
	chmod a+x build/interactive.sh
