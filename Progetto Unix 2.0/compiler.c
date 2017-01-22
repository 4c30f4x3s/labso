

all: Cal



Cal: Calcetto.o msg.o sem.o 
	gcc Calcetto.o msg.o sem.o -o Cal

Calcetto.o: Calcetto.c
	gcc -c Calcetto.c

msg.o: msg.c
	gcc -c msg.c

sem.o: sem.c
	gcc -c sem.c

clean: 
	rm -rf *o Cal

CC=gcc
	


