all:
	mkdir -p bin
	make -C src

clean:
	make -C src $@