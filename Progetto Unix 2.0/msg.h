#include <sys/msg.h>

typedef struct mex{
	long type;	
	char text[255];
}mex;

//crea una coda di messaggi restituendo l'ID interno
int createMsg(key_t semKey);

//invia un messaggio sulla coda msgid
void send(int msgid, mex* messaggio);

//riceve un messggio dalla coda
void receive(int msgid, mex*messaggio, int type);

//stampa a video e aggiorna le informazioni contenute
// nella struttura dati associata msgqid_ds
void inf(int msgid);

//rimuove la coda di messaggi msgid
void deleteMsg(int msgid);
