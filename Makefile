CFLAGS = -Wall -g -O3 -ffast-math -march=core2 -msse3 -mfpmath=sse -I . `pkg-config --cflags glib-2.0 gsl`
LIBS = -lm `pkg-config --libs glib-2.0 gsl`
# XXX
CLANG=/opt/local/bin/clang

all: cbuf.bc

%.o: %.c *.h
	gcc $(CFLAGS) -c -o $@ $<

%.s: %.c *.h
	gcc -fverbose-asm -S $<

%.bc: %.c
	$(CLANG) -emit-llvm -c $< -o $@

clean:
	rm -f *.o *.bc
