#include "msg.h"
#include "headers.h"

int createMsg(key_t msgKey){
	int msgid;
	if((msgid = msgget(msgKey,0666 | IPC_CREAT)) == -1){
		ERROR;
		return -1;
	}

	return msgid;
}

void send(int msgid, mex* messaggio){
	if(msgsnd(msgid, messaggio, strlen(messaggio->text)+1,0) == -1){
		ERROR;
		return;
	}

}

void receive(int msgid, mex*messaggio, int type){
	if(msgrcv(msgid, messaggio, 255,type,0) == -1){
		ERROR;
		return;
	}
}

void inf(int msgid){
	struct msqid_ds buff;
	if(msgctl(msgid,IPC_STAT, &buff)==-1){
		ERROR;
		return;
	}
	
	printf("ora dell'ultima msgsnd: %ld\n", buff.msg_stime);
	printf("ora dell'ultima msgrcv: %ld\n", buff.msg_rtime);
	printf("ora dell'ultima msgctl: %ld\n", buff.msg_ctime);
	printf("numero messaggi in coda: %ld\n", buff.msg_qnum);
	printf("dimensione in byte coda: %ld\n", buff.msg_qbytes);

}

void deleteMsg(int msgid){
	if(msgctl(msgid,IPC_RMID, NULL)==-1){
		ERROR;
		return;
	}
}




