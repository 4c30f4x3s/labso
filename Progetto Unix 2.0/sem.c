#include "sem.h"
#include "headers.h"

int createSem(key_t semKey){
	int semid;
    if((semid = semget(semKey,NUMSEM,0666 | IPC_CREAT)) == -1){
        ERROR;
        return -1;
    }
    return semid;    
}

int createSetSem(key_t semKey,int num){
	int semid;
    if((semid = semget(semKey,num,0666 | IPC_CREAT)) == -1){
        ERROR;
        return -1;
    }
    return semid;    
}

void setSem(int semid, int semnum, int val){
     if(semctl(semid, semnum, SETVAL, val)==-1){
			ERROR;
			return;
		}
}


int getSem(int semid, int semnum){
	int val;
	if(val = (semctl(semid,semnum,GETVAL))==-1)
		ERROR;
		
	return val;
}

void checkSem(int semid, int semnum){
     struct sembuf cmd;
  	  cmd.sem_num = semnum;
     cmd.sem_op = 0;
     cmd.sem_flg = 0;
     if(semop(semid, &cmd, 1)==-1)
			ERROR;
		
}

void semWait(int semid, int semnum){
     struct sembuf cmd;
     cmd.sem_num = semnum;
     cmd.sem_op = -1;
     cmd.sem_flg = 0;
     if(semop(semid, &cmd, 1)==-1)
			ERROR;
}


void semSignal(int semid, int semnum){
     struct sembuf cmd;
     cmd.sem_num = semnum;
     cmd.sem_op = 1;
     cmd.sem_flg = 0;
     if(semop(semid, &cmd, 1)==-1)
			ERROR;
}

void deleteSem(int semid){
     if(semctl(semid, 0, IPC_RMID, 0)==-1)
			ERROR;
}

