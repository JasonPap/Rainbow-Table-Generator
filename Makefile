CC = g++
CFLAGS = -o3
STANDARD = -std=c++11
all: rainbow
rainbow: main.o
	$(CC) $(CFLAGS) $(STANDARD) -o rainbow main.o RainbowTable.o blake_ref.o -lpthread 
	rm  -f main.o RainbowTable.o blake_ref.o

main.o: RainbowTable.o
	$(CC) $(STANDARD) -c RT-Generator/main.cpp
	
RainbowTable.o: blake_ref.o
	$(CC) $(STANDARD) -c  RT-Generator/RainbowTable.cpp

blake_ref.o: Blake/blake_ref.cpp
	$(CC) $(STANDARD) -c Blake/blake_ref.cpp

clean:
	rm -f rainbow
