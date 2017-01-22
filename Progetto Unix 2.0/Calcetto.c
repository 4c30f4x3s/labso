#include "msg.h"
#include "sem.h"
#include "headers.h"
#define SIZE 255	
#define PALLA 0		
#define SQUADRAA 1	
#define SQUADRAB 2	
#define MUTEX  3  
//--Serie di valori rappresentanti:--
int scoreA = 0;		//Punteggio squadra A
int scoreB = 0;		//punteggio squadra B
int semID;			//L'ID del set di semafori
int msgID;			//L'ID della coda di messaggi
int padre;			//Pid del processo padre		

//--Istanze della struttura di tipo mex (messaggio) definita in msg.h--
mex messaggio;		
mex messaggio2;
mex messaggio3;
mex messaggio4;
//mex goleador;

int cron;			//File ID per il file di "telecronaca"

/*Funzione il cui compito è ritonare un valore binario rappresentante l'avvenimento o meno di un evento la cui probabilità
è rappresentata dal parametro num. Il fattore casuale è dato dalle funzioni srand() e rand()*/
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
//Questa funzione si occupa della gestione di segnali
void funzione(int sig){
	switch(sig){
		case SIGALRM:		//Segnale rappresentante la fine della partita
			printf("tempo scaduto, partita finita\n");
			dprintf(cron, "tempo scaduto, partita finita\n");
			printf("risultato finale: squadra A %d punti, squadra B %d punti\n",scoreA,scoreB);
			dprintf(cron,"risultato finale: squadra A %d punti, squadra B %d punti\n",scoreA,scoreB);
			close(cron);	
			kill(0,SIGTERM);//Invio, a tutti i procesi, del segnale di terminazione				
			deleteMsg(msgID);//Cancellazione della coda di messaggi e del set di semafori corrispondenti rispettivamento 
			deleteSem(semID);//al message ID e al semaphore ID passati come parametri
			kill(0,SIGINT);
		break;
		
		case SIGUSR1: 		//Segnale goal squadra A
			scoreA++;
			printf("punteggio A == %d\n", scoreA);
		//	dprintf(cron,"goooal di %d, punteggio A == %d\n **PADRE: %d\n",getpid(), scoreA, padre);
		break;
	
		case SIGUSR2:		//Segnale goal squadra B
			scoreB++;
			printf("punteggio B == %d\n", scoreB);
			//dprintf(cron,"goooal di %d, punteggio B == %d\n **PADRE: %d\n",getpid(), scoreB, padre);
		break;
		
		default:
			break;		
	}
}

	

//Funzione adibita alla comunicazione tra processi, grazie alla quale il processo "fato" puó
//comunicare l'esito degli eventi randomici al giocatore
void fato(char stringa[]){
	sprintf(messaggio.text, "%s",stringa);		//Il parametro stringa é del tipo "infortunio"; "dribbling"; "tiro"
	send(msgID,&messaggio);							//Un messaggio contenente stringa viene inviato sulla coda msgID.
	receive(msgID,&messaggio4,3);					//Essendo ls sys call msgrecive bloccante ora si attende che lo scheduling dia la CPU al processo "fato" righa xxxxxxxxxXXXXXXXXXXXXXXX
}

int main(void){
	//Creazione delle chiavi necessari all'invocazione dei metodi definiti in sem.c e msg.cabs
	//(non fondamentali, sostituibili con IPC_PRIVATE)
	int keySem = ftok("/tmp", 'S');	
//	int keySemA = ftok("/tmp", '2');
//	int keySemB = ftok("/tmp", '3');
	int keyMsg = ftok("/tmp", 'K');
	printf("Sono il proc arbitro, PID == %d\n", getpid());
	semID = createSetSem(keySem,4);	//Le funzioni ritorno l'ID del set dei tre semafori  /// 4 sem in tutto
	msgID = createMsg(keyMsg);		//e l'ID della coda di messaggi

	setSem(semID,PALLA,1);			//Assegnazione dei valori ai vari semafori presenti nel set
	setSem(semID,SQUADRAA,5);
	setSem(semID,SQUADRAB,5);
	setSem(semID, MUTEX, 1);   	
	
	messaggio.type = 1;				//Assegnazione dei tipi ai messaggi, parte necessaria 
	messaggio2.type = 2;				//per far ricevere ad ogni processo il messaggio che gli compete
	messaggio3.type = 3;
	messaggio4.type = 4;

	int pida = 0;							//Dichiarazioni variabili necessarie alla computazione
	int pidb = 0;
	int pidA = 0;
	int pidB = 0;
	int pidw = 0;
	int pidfato = 0;
	int status;
	int durata = 1;
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
	
	cron = open("cronistoria.txt", O_WRONLY|O_CREAT|O_APPEND|O_TRUNC);	//Questa riga di codice apre il file di "telecronaca" e salva il suo ID in cron

	
	//Le righe di codice seguenti si occupano dell'apertura del file di configurazione
	// e dell'inizializzazione del timer che detta la durata del match

	char tiro[SIZE];
			char infortunio[SIZE];
			char dribbling[SIZE];
			float perc_tiro;
			float perc_infortunio;
			float perc_dribbling;


	FILE *conf = fopen("conf.txt","r");		
	fscanf(conf,"%s %d", stringa, &durata);
	fscanf(conf,"%s %f", tiro, &perc_tiro);
	fscanf(conf,"%s %f", infortunio, &perc_infortunio);			
	fscanf(conf,"%s %f", dribbling, &perc_dribbling);

	
	printf("%s %d\n", stringa, durata);
	dprintf(cron, " %s %d\n", stringa, durata);
	alarm(durata);
	fclose(conf);

	//printf("\n\n HO ESEGUITO LA FORK DI squadraA %d \n\n", getpid());

	pidA = fork();
	if(!pidA){
		//printf("\n****%d***\n", getpid());
		//printf("Sono il processo squadra A, PID == %d\n", getpid());

	while(true){	
				
		semWait(semID, SQUADRAA);		
	 	pida = fork();	
		int giocata = 1;
	 
	
		///--------------------------------------GIOCATORE--------------------------------------------
		if(pida == 0){		
			printf("entra in squadra A il giocatore %d\n",getpid());
			dprintf(cron, "entra in squadra A il giocatore %d\n",getpid());	
	
			
	
			while(true){
				if(getSem(semID, SQUADRAA) == 0 && getSem(semID, SQUADRAB) == 0){			
				semWait(semID, PALLA);
				printf("il giocatore A: %d ha preso il pallone\n", getpid());
				
				giocata = 1;					
				while(giocata > 0){
					
					int ris;									
					srand((unsigned)time(NULL));		
					ris = rand()%3;	
				
					switch(ris){							
						case 0:
							
							fato("infortunio");		
							if(strcmp(messaggio4.text, r1)== 0){			
								printf("giocatore %d squadra A infortunato\n",getpid());
								dprintf(cron,"giocatore %d squadra A infortunato\n",getpid());
								semSignal(semID,PALLA);		
								semSignal(semID,SQUADRAA);
								exit(0);	
							}
						break;
		
						case 1:				
							giocata = 0;		
							fato("tiro");			
							if(strcmp(messaggio4.text, r1)== 0){	

							printf(" _____   _____\n|       |     |   %C%C   |    \n|  ___  |     |  %c__%c  |\n|_____| |_____| %c    %c |____ di A: %d", (char)47, (char)92, (char)47, (char)92,(char)47, (char)92, getpid()); //stringa per GOAL_ascii art

							dprintf(cron, " _____   _____\n|       |     |   %C%C   |    \n|  ___  |     |  %c__%c  |\n|_____| |_____| %c    %c |____ di A: %d", (char)47, (char)92, (char)47, (char)92,(char)47, (char)92, getpid());
								/*printf(" di A: %d\n ",getpid());
								dprintf(cron,"goooal di %d\n", getpid());*/	
								kill(padre,SIGUSR1);	
							}else{		
								printf("giocatore A %d fallisce il tiro in porta\n",getpid());
								dprintf(cron,"giocatore A %d fallisce il tiro in porta\n",getpid());
							}
							semSignal(semID, PALLA);
						break;	
		
						default:
							fato("dribbling");
							//printf("%s\n", messaggio4.text);
							if(strcmp(messaggio4.text, r1) != 0){		///dribbling sbagliato perde palla
								giocata = 0;
								printf("giocatore A %d perde palla\n",getpid());
								dprintf(cron,"giocatore A %d perde palla\n",getpid());	
								semSignal(semID,PALLA);						
							}			
							else{
							printf("giocatore A %d dribla con successo\n",getpid());
							dprintf(cron,"giocatore A %d dribla con successo\n",getpid());	
							}
						break;

						}///esce dallo switch 
					}	///viene testato il valore di giocata ed esce dal ciclo while(giocata > 0)
				}	//chiusura IF sul test dei semafori squadra ==0
			}
		}///anche in caso di goal esce dallo switch, esce dal while ma rimane in sezione critica
			//printf("%d\n", wait(&status));
	}		
	}	
	

	pidB = fork();
	if(!pidB){
		//printf("Sono il processo squadra B, PID == %d\n", getpid());

	while(true){	
		
		semWait(semID,SQUADRAB);
	 	pidb = fork();
		int giocataB = 1; 
	
		///--------------------------------------GIOCATORE--------------------------------------------
		if(pidb == 0 && getpid() != padre){
			printf("entra in squadra B il giocatore %d\n",getpid());
			dprintf(cron,"entra in squadra B il giocatore %d\n",getpid());
			
			while(true){
				if(getSem(semID, SQUADRAB) == 0 && getSem(semID, SQUADRAA) == 0){
				semWait(semID, PALLA);	
				printf("palla al giocatore B: %d\n", getpid());
			   giocataB = 1;		
				while(giocataB > 0){
					
					int ris;
					srand((unsigned)time(NULL));	
					ris = rand()%3;
					
					switch(ris){
						case 0:
	
							fato("infortunio");
							if(strcmp(messaggio4.text, r1)== 0){			
								printf("giocatore %d squadra B infortunato\n",getpid());	
								dprintf(cron,"giocatore %d squadra B infortunato\n",getpid());	
								semSignal(semID,PALLA);
								semSignal(semID,SQUADRAB);
								exit(0);	
							}
						break;
		
						case 1:
							giocataB = 0;			
							fato("tiro");
							if(strcmp(messaggio4.text, r1)== 0){	

				
							printf(" _____   _____\n|       |     |   %C%C   |    \n|  ___  |     |  %c__%c  |\n|_____| |_____| %c    %c |____ di A: %d", (char)47, (char)92, (char)47, (char)92,(char)47, (char)92, getpid()); //stringa per GOAL_ascii art

							dprintf(cron, " _____   _____\n|       |     |   %C%C   |    \n|  ___  |     |  %c__%c  |\n|_____| |_____| %c    %c |____ di B: %d", (char)47, (char)92, (char)47, (char)92,(char)47, (char)92, getpid());
								/*printf("goooal di B: %d\n",getpid());
								dprintf(cron,"goooal di B %d\n",getpid());*/
								kill(padre,SIGUSR2);
							}else{
								printf("giocatore B %d fallisce il tiro in porta\n",getpid());
								dprintf(cron,"giocatore B %d fallisce il tiro in porta\n",getpid());
							}
							semSignal(semID,PALLA);
						break;	
			
						default:
							fato("dribbling");
							if(strcmp(messaggio4.text, r1)!= 0){		
								giocataB = 0;
								printf("giocatore B %d perde palla\n",getpid());
								dprintf(cron,"giocatore B %d perde palla\n",getpid());
								semSignal(semID,PALLA);								
							}			
							else{
								printf("giocatore B %d dribla con successo\n",getpid());
								dprintf(cron,"giocatore B %d dribla con successo\n",getpid());
							}
						break;

						}
					}
				}//IF FAIR PLAY
			}
		}
	}		
	}	
	
	/*Il procsso fato si occupa dell'invio dei messaggi riguardanti l'avvenuto compimento (o meno) dell'azione 
	per cui è stato chiamato*/
	
	pidfato = fork();
		if(!pidfato && getpid() != padre){	
		//printf("/*%d*\n", getpid());
			FILE *log = fopen("conf.txt","r");	//Sezione adibita alla lettura del file di configurazione il quale contine i dati su cui fato 
															//deve basarsi per determinare la casualità degli eventi
			int ris;
			/*char tiro[SIZE];
			char infortunio[SIZE];
			char dribbling[SIZE];
			float perc_tiro;
			float perc_infortunio;
			float perc_dribbling;*/

			/*fscanf(log,"%s %f", tiro, &perc_tiro);	///Prima lettura necessaria a "conusmare" la prima riga
			fscanf(log,"%s %f", tiro, &perc_tiro);
			fscanf(log,"%s %f", infortunio, &perc_infortunio);			
			fscanf(log,"%s %f", dribbling, &perc_dribbling);*/


			dprintf(cron, "percentuale tiro: %.2f, percentuale infortunio: %.2f,percentuale dribbling: %.2f, \n", perc_tiro, perc_infortunio, perc_dribbling);
			
			fclose(log);
			
			while(true){															//Il processo fato si occupa di ricevere i messaggi invaiti dalla funzione fato();
				sleep(1);															//una volta ricevuto il messaggio ne verifica il contenuto con delle string compare
				receive(msgID,&messaggio2,1);
				//printf("messaggio ricevuto dal fato: %s\n", messaggio2.text);									//ora che sa cosa contiene puó elaborare un risultato casuale binario sulla base delle
				if(strcmp(messaggio2.text, tiro)==0){					//percentuali inserite nel file di configurazione. Ció avviene tramite le chiamate opportune a binario()
					ris = binario(perc_tiro);	
					//printf("Ris bin tiro == %d\n", ris);
				}
				else if(strcmp(messaggio2.text, infortunio)==0){			
					ris = binario(perc_infortunio);
					//printf("Ris bin infortunio == %d\n", ris);
				}
				else if(strcmp(messaggio2.text, dribbling)==0){			
					ris = binario(perc_dribbling);
					//printf("Ris bin dribling == %d\n", ris);
				}
				
				sprintf(messaggio3.text,"%d", ris);							//Arrivato a questo punto della computazione é necessario elaborare un messaggio di risposta
				send(msgID,&messaggio3);										//tramite sprintf si scrive l'esito nel campo text di messaggio3, e poi lo si invia sulla coda in modo che la precedente recive
			}																			//(chiamata da fato()) attualemnte bloccata possa riceverlo e permettere al giocatore in questione di riprendere la computazione
		}

	while(true){}
//	pause();
	return(0);
}
