
all: 
	@cc src/basted.c -lX11 -o basted-nopimize
	@cc src/basted.c -lX11 -O1 -o basted-o1
	@cc src/basted.c -lX11 -O2 -o basted-o2
	@cc src/basted.c -lX11 -O3 -o basted-o3
	@cc src/basted.c -lX11 -flto -O3 -o basted-o3-flto
	@cc src/basted.c -lX11 -Os -o basted-smol

O1:
	@cc src/basted.c -lX11 -O1 -o basted

O2:
	@cc src/basted.c -lX11 -O2 -o basted

O3:
	@cc src/basted.c -lX11 -O3 -o basted

O3-flto:
	@cc src/basted.c -lX11 -O3 -flto -o basted

Os:
	@cc src/basted.c -lX11 -Os -o basted

install:
	@echo "idk how to do this yet. Just put it in smth like /usr/local/bin for the moment."



