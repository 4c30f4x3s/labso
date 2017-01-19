#include <sys/sem.h>

#define NUMSEM 1 /*Numero di semafori */

//crea un set di semafori con la chiave associata contenente un semaforo
int createSem(key_t semKey);

//crea un set di semafori con la chiave associata contenente num semafori
int createSetSem(key_t semKey,int num);

//imposta il valore del semaforo semnum del set semid al campo val
void setSem(int semid, int semnum, int val);

//restituisce il valore corrente del semaforo semnum del set semid
int getSem(int semid, int semnum);

//controlla che il valore corrente el semaforo semnum del set semid sia pari a 0
int chekSem(int semid, int semnum);

//decrementa di 1 il valore corrente del semaforo semnum del set semid
void semWait(int semid, int semnum);

//aumenta di 1 il valore corrente del semaforo semnum del set semid
void semSignal(int semid, int semnum);

//rimuove il set di semafori semid
void deleteSem(int semid);
