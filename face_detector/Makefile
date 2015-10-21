all:build

build: 
	make -C src build

rebuild: clean build

install: 
	make -C src build
	make -C src install

example: install
	make -C example build

clean:
	make -C src clean

