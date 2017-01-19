#include "msg.h"
#include "sem.h"
#include "headers.h"
#define SIZE 255
#define PALLA 0
#define SQUADRAA 1
#define SQUADRAB 2

int scoreA = 0;
int scoreB = 0;
int semID;
int semID;
int semIDB;
int msgID;
int padre;

mex messaggio;
mex messaggio2;
mex messaggio3;
mex messaggio4;

int cron;

int binario(float num){
	srand((unsigned)time(NULL));	
	float max = 100.00;
	return (((float)rand()/(float)(RAND_MAX))*max) < num ? 1 : 0;       
}

void terminazionefiglio(int sig){
	switch (sig){
		case SIGTERM:
			if(getpid()!= padre)
				kill(getpid(), SIGSTOP);
		break;
		
	}
}

void funzione(int sig){
	switch(sig){
		case SIGALRM:
			printf("tempo scaduto, partita finita\n");
			dprintf(cron, "tempo scaduto, partita finita\n");
			printf("risultato finale: squadra A %d punti, squadra B %d punti\n",scoreA,scoreB);
			dprintf(cron,"risultato finale: squadra A %d punti, squadra B %d punti\n",scoreA,scoreB);
			close(cron);
			kill(0,SIGTERM);				
			deleteMsg(msgID);
			deleteSem(semID);			
			kill(0,SIGINT);
		break;
		
		case SIGUSR1:
			scoreA++;
			printf("goooal di %d, punteggio A == %d\n",getpid(), scoreA);
			dprintf(cron,"goooal di %d, punteggio A == %d\n",getpid(), scoreA);
		break;
	
		case SIGUSR2:
			scoreB++;
			printf("goooal di %d, punteggio B == %d\n",getpid(),scoreB);
			dprintf(cron,"goooal di %d, punteggio B == %d\n",getpid(),scoreB);
		break;
		
		default:
			break;		
	}
}

void fato(char stringa[]){
	sprintf(messaggio.text, "%s",stringa);
	send(msgID,&messaggio);
	receive(msgID,&messaggio4,3);
}

int main(void){
	
	int keySem = ftok("/tmp", '1');
	int keySemA = ftok("/tmp", '2');
	int keySemB = ftok("/tmp", '3');
	int keyMsg = ftok("/tmp", '4');

	semID = createSetSem(keySem,3);
	msgID = createMsg(keyMsg);

	setSem(semID,PALLA,1);
	setSem(semID,SQUADRAA,5);
	setSem(semID,SQUADRAB,5);

	messaggio.type = 1;
	messaggio2.type = 2;
	messaggio3.type = 3;
	messaggio4.type = 4;

	int pida;
	int pidb;
	int pidA;
	int pidB;
	int pidw;
	int pidfato;
	int status;
	int durata = 0;
	padre = getpid();

	struct sigaction nuovo, nuovo1,nuovo2,nuovo3;
	nuovo.sa_handler = &funzione;
	nuovo1.sa_handler = &funzione;
	nuovo2.sa_handler = &funzione;
	nuovo3.sa_handler = &terminazionefiglio;

	sigemptyset(&nuovo.sa_mask);
	sigemptyset(&nuovo1.sa_mask);
	sigemptyset(&nuovo2.sa_mask);
	sigemptyset(&nuovo3.sa_mask);

	nuovo.sa_flags = 0;
	nuovo1.sa_flags = SA_RESTART;
	nuovo2.sa_flags = SA_RESTART;
	nuovo3.sa_flags = 0;
	
	if(sigaction(SIGALRM, &nuovo,NULL)==-1) ERROR;
	if(sigaction(SIGUSR1, &nuovo1,NULL)== -1) ERROR;
	if(sigaction(SIGUSR2, &nuovo2,NULL)== -1) ERROR;
	if(sigaction(SIGTERM, &nuovo3,NULL)== -1) ERROR;
	
	char r1[] = "1";
	char r2[] = "0";
	char stringa[SIZE];
	
	cron = open("cronistoria.txt", O_WRONLY|O_CREAT|O_APPEND|O_TRUNC);

	FILE *conf = fopen("conf.txt","r");
	fscanf(conf,"%s %d", stringa, &durata);
	printf("%s %d\n", stringa, durata);
	alarm(durata);
	fclose(conf);

	pidA = fork();
	if(!pidA){

	while(true){	
		semWait(semID,SQUADRAA);
	 	pida = fork();
		if(pida == 0){
			printf("entra in squadra A il giocatore %d\n",getpid());
			dprintf(cron, "entra in squadra A il giocatore %d\n",getpid());

			semWait(semID,PALLA);
			while(true){

				int giocata = 1;
				while(giocata > 0){

					int ris;
					srand((unsigned)time(NULL));	
					ris = rand()%3;

					switch(ris){
						case 0:
							giocata = 0;
							fato("infortunio");
							if(strcmp(messaggio4.text, r1)== 0){			
								semSignal(semID,PALLA);
								semSignal(semID,SQUADRAA);
								printf("giocatore %d squadra A infortunato\n",getpid());
								dprintf(cron,"giocatore %d squadra A infortunato\n",getpid());
								exit(0);	
							}
						break;
		
						case 1:
							giocata = 0;
							fato("tiro");
							if(strcmp(messaggio4.text, r1)== 0){			
								kill(padre,SIGUSR1);
								semSignal(semID,PALLA);
							}else
								printf("giocatore A %d fallisce il tiro in porta\n",getpid());
								dprintf(cron,"giocatore A %d fallisce il tiro in porta\n",getpid());
						break;	
			
						default:
							fato("dribbling");
							if(strcmp(messaggio4.text, r1)== 1){		
								semSignal(semID,PALLA);
								giocata = 1;
							}			
							printf("giocatore A %d ha effettuato un dribbling con successo\n",getpid());	
							dprintf(cron,"giocatore A %d ha effettuato un dribbling con successo\n",getpid());	
						break;

						}
					}
				}
			}
		}		
	}	
	
	pidB = fork();
	if(!pidB){

	while(true){	
		semWait(semID,SQUADRAB);
	 	pidb = fork();
		if(pidb == 0){
			printf("entra in squadra B il giocatore %d\n",getpid());
			dprintf(cron,"entra in squadra B il giocatore %d\n",getpid());
			semWait(semID,PALLA);
			while(true){

				int giocataB = 1;
				while(giocataB > 0){

					int ris;
					srand((unsigned)time(NULL));	
					ris = rand()%3;

					switch(ris){
						case 0:
							giocataB = 0;
							fato("infortunio");
							if(strcmp(messaggio4.text, r1)== 0){			
								semSignal(semID,PALLA);
								semSignal(semID,SQUADRAB);
								printf("giocatore %d squadra B infortunato\n",getpid());	
								dprintf(cron,"giocatore %d squadra B infortunato\n",getpid());	
								exit(0);	
							}
						break;
		
						case 1:
							giocataB = 0;
							fato("tiro");
							if(strcmp(messaggio4.text, r1)== 0){			
								kill(padre,SIGUSR2);
								semSignal(semID,PALLA);
							}else
								printf("giocatore B %d fallisce il tiro in porta\n",getpid());
								dprintf(cron,"giocatore B %d fallisce il tiro in porta\n",getpid());
						break;	
			
						default:
							fato("dribbling");
							if(strcmp(messaggio4.text, r1)== 1){		
								semSignal(semID,PALLA);
								giocataB = 1;
							}			
							printf("giocatore B %d ha effettuato un dribbling con successo\n",getpid());	
							dprintf(cron,"giocatore B %d ha effettuato un dribbling con successo\n",getpid());	
						break;

						}
					}
				}
			}
		}		
	}	
	
	pidfato = fork();
		if(!pidfato){	
	
			FILE *log = fopen("conf.txt","r");

			int ris;
			char tiro[SIZE];
			char infortunio[SIZE];
			char dribbling[SIZE];
			float perc_tiro;
			float perc_infortunio;
			float perc_dribbling;

			fscanf(log,"%s %f", tiro, &perc_tiro);
			fscanf(log,"%s %f", infortunio, &perc_infortunio);			
			fscanf(log,"%s %f", dribbling, &perc_dribbling);
			dprintf(cron, "percentuale tiro: %.2f, percentuale infortunio: %.2f,percentuale dribbling: %.2f, \n", perc_tiro, perc_infortunio, perc_dribbling);
			
			fclose(log);
			
			while(true){
				sleep(1);
				receive(msgID,&messaggio2,1);
				if(strcmp(messaggio2.text, tiro)==0)
					ris = binario(perc_tiro);	
				else if(strcmp(messaggio2.text, infortunio)==0)				
					ris = binario(perc_infortunio);
				else if(strcmp(messaggio2.text, dribbling)==0)				
					ris = binario(perc_dribbling);
				
				sprintf(messaggio3.text,"%d", ris);
				send(msgID,&messaggio3);
			}
		}
	
	while((pidw = wait(&status))>0){
				if(WIFEXITED(status)){
					printf("giocatore %d ritirato \n",pidw);
					dprintf(cron,"giocatore %d ritirato \n",pidw);
				}else{
					printf("giocatore %d terminato non correttamente\n" ,pidw);
					dprintf(cron,"giocatore %d terminato non correttamente\n" ,pidw);
				}
	}
	return(0);
}
