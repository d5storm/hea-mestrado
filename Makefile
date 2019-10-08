OPTIONS = -c
DEBUG = -g 
COMP = g++ -O4 -std=c++11 

all: obj/Mining.o obj/hea.o 
	$(COMP) -o bin/HEA obj/hea.o obj/Mining.o -lm

obj/Mining.o: source/Mining.cpp source/Mining.h
	$(COMP) $(OPTIONS) source/Mining.cpp -o obj/Mining.o

obj/hea.o: source/hea.cpp include/*
	$(COMP) $(OPTIONS) source/hea.cpp -o obj/hea.o 

	
clean:
	rm -f HEA *~ *.o obj/*.o
