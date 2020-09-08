App.o: App.c App.h
	gcc -c App.c -o App.o

Compress.o: Compress.c App.h
	gcc -c Compress.c -o Compress.o

Differentiate.o: Differentiate.c App.h
	gcc -c Differentiate.c -o Differentiate.o

Filter.o: Filter.c App.h
	gcc -c Filter.c -o Filter.o

Scale.o: Scale.c App.h
	gcc -c Scale.c -o Scale.o

all: App.o Compress.o Differentiate.o Filter.o Scale.o
	gcc App.o Compress.o Differentiate.o Filter.o Scale.o -o App

clean:
	rm -f App.o Compress.o Differentiate.o Filter.o Scale.o App Output.bin