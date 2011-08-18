all:
	mkdir -p build
	cd build && cmake .. && make

build:
	./.doit -p --local

clean:
	rm -rf build

