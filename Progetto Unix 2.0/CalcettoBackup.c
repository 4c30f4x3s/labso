#include "msg.h"
#include "sem.h"
#include "headers.h"
#define SIZE 255	//Dimensione massima messaggio.text
#define PALLA 0		//Numero del semaforo all'interno del set
#define SQUADRAA 1	//Numero del semaforo all'interno del set
#define SQUADRAB 2	//Numero del semaforo all'interno del set
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

int cron;			//File ID per il file di "telecronaca"

/*Funzione il cui compito è ritonare un valore binario rappresentante l'avvenimento o meno di un evento la cui probabilità
è rappresentata dal parametro num. Il fattore casuale è dato dalle funzioni srand() e rand()*/
int binario(float num){	
	srand((unsigned)time(NULL));	
	float max = 100.00;
	return (((float)rand()/(float)(RAND_MAX))*max) < num ? 1 : 0;       
}

void terminazionefiglio(int sig){		//Funzione adibita alla preparazione dei processi figli (che siano essi squadre, giocatori, o il pallone)
	switch (sig){						//alla terminazione. Opera traminte l'invio di un segnale di STOP il quale precederà il segnale di KILL
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
			printf("goooal di %d, punteggio A == %d\n",getpid(), scoreA);
			dprintf(cron,"goooal di %d, punteggio A == %d\n",getpid(), scoreA);
		break;
	
		case SIGUSR2:		//Segnale goal squadra B
			scoreB++;
			printf("goooal di %d, punteggio B == %d\n",getpid(),scoreB);
			dprintf(cron,"goooal di %d, punteggio B == %d\n",getpid(),scoreB);
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

	semID = createSetSem(keySem,3);	//Le funzioni ritorno l'ID del set dei tre semafori 
	msgID = createMsg(keyMsg);		//e l'ID della coda di messaggi

	setSem(semID,PALLA,1);			//Assegnazione dei valori ai vari semafori presenti nel set
	setSem(semID,SQUADRAA,5);
	setSem(semID,SQUADRAB,5);

	messaggio.type = 1;				//Assegnazione dei tipi ai messaggi, parte necessaria 
	messaggio2.type = 2;				//per far ricevere ad ogni processo il messaggio che gli compete
	messaggio3.type = 3;
	messaggio4.type = 4;

	int pida;							//Dichiarazioni variabili necessarie alla computazione
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
	
	cron = open("cronistoria.txt", O_WRONLY|O_CREAT|O_APPEND|O_TRUNC);	//Questa riga di codice apre il file di "telecronaca" e salva il suo ID in cron

	
	//Le righe di codice seguenti si occupano dell'apertura del file di configurazione
	// e dell'inizializzazione del timer che detta la durata del match
	FILE *conf = fopen("conf.txt","r");		
	fscanf(conf,"%s %d", stringa, &durata);
	printf("%s %d\n", stringa, durata);
	alarm(durata);
	fclose(conf);

	pidA = fork();	//Creazione della prima squadra
	if(!pidA){

	while(true){	//Ciclo infinito rappresentante il comportamento della squadra A
						//Il rcesso squadraA si occupa della creazione dei propri processi figli(giocatori)
		semWait(semID,SQUADRAA);		//operazione di wait sul semaforo inizializzato a 5 necessario per regolare la presenza constante
												//di solo 5 ciocatori in campo, inizializzandolo ad 11, per esempio, si passerebbe da una partita		
												//di calcio a 5 ad una di calcio a 11
	 	pida = fork();	///---------------------------------------------------------------->GIOCATORE<------------------------------------------------------------
		int giocata = 1;	/// definisco la variabile giocata e la inizializzo a 1
		if(pida == 0){		//****************************************
			if(giocata == 1){	///Se gioata != 1 vuol dire che si tratta di un giocatore già in campo che ha tirato!!11!
				printf("entra in squadra A il giocatore %d\n",getpid());
				dprintf(cron, "entra in squadra A il giocatore %d\n",getpid());
			}
		///Arrivato qua il giocatore che ha tirato tornerà a competere per la palla insieme a tutti i suoi colleghi e avversari
			semWait(semID,PALLA);		//wait che regola la mutua esclusione della sezione critica dei processi figli
												//semaforo PALLA del set semID inizializzato a 1 == 1 solo pallone in campo == 1 solo giocatore alla volta può driblare, tirare, infortunarsi
			giocata = 1;			///La variabile giocata viene messa ad uno in modo che il tiratore possa tornare a compiere azioni
			while(true){			///e non fermarsi sul break sul quale si era fermato subito dopo il tiro, prima di riottenere palla
				if(giocata == 0)	///*******************************
					break;			///*******************************
				giocata = 1;		///int giocata = 1;	RINDONDANTE?? ->NO, necessaria per il giocatore che dribla con successo
				while(giocata > 0){

					int ris;									//Tre linee di codice volte alla randomizzazionie del tipo di azione
					srand((unsigned)time(NULL));		//che un giocatore prova a compiere
					ris = rand()%3;	

					switch(ris){							//scelta dell'azione da tentare
						case 0:
							/// giocata = 0;	settando giocata a 0 il giocatore che non si infortuna torna a competere per il pallone, mentre é piú logico che continui a compiere azioni 			
							fato("infortunio");		//Grazie al meccanismo della funzione fato() e del processo "fato" il campo text della struttura mex messaggio4 
							if(strcmp(messaggio4.text, r1)== 0){	//contine un valore binario rappresentante il successo o meno dell'azione
								semSignal(semID,PALLA);		//se il giocatore risulta infortunato vengono incrementai i semafori PALLA e SQUADRAA
								semSignal(semID,SQUADRAA);	//in modo che il primo torni disponibile agli altri processi giocatore, mentr il secondo si attivi per 
																	//creare un novo processo giocatore
								printf("giocatore %d squadra A infortunato\n",getpid());
								dprintf(cron,"giocatore %d squadra A infortunato\n",getpid());
								exit(0);	
							}
						break;
		
						case 1:				//Caso del tiro in porta
							giocata = 0;			///in questo caso va bene che dopo un tiro sbagliato tutti i giocatori possano competere per il pallone
							fato("tiro");			//stesso meccanismo del caso infortunio
							if(strcmp(messaggio4.text, r1)== 0){	//però in questo caso, se avviene il goal il giocatore informa, tramite segnale SIGUSR1		
								kill(padre,SIGUSR1);						//il padre (arbitro) e poi rilascia il semaforo PALLA
								semSignal(semID,PALLA);
							}else{			//invece, se fato() rispondesse con 0 terminerebbe la computazione del case,
								printf("giocatore A %d fallisce il tiro in porta\n",getpid());
								dprintf(cron,"giocatore A %d fallisce il tiro in porta\n",getpid());
							}
						break;	
		///Il gicatore che non esce termina il caso tiro non esce dalla sezione crtica e rimane all'intenro del ciclo wile(true)
						default:
							fato("dribbling");
							if(strcmp(messaggio4.text, r1)== 1){		//se il risultato di fato() per l'evento dribbling è negativo il giocatore perde la palla
								semSignal(semID,PALLA);						//ne libera il semaforo, setta giocata = 1??
								giocata = 1;
							}			
							printf("giocatore A %d tenta di nuovo il tiro\n",getpid());	//l'esito positvo permettere invece al giocatore tenere il palllone 
							dprintf(cron,"giocatore A %d tenta di nuovo il tiro\n",getpid());	//rimanere all'interno del ciclo while(giocata > 0) e ritentare un azione
						break;

						}///esce dallo switch 
					}	///viene testato il valore di giocata ed esce dal ciclo while(giocata > 0)
				}	///ma non esce da questo ciclo rimanendo in sezione critica, come se avesse sempre il pallone fino a che non si infortuna.
			}///anche in caso di goal esce dallo switch, esce dal while ma rimane in sezione critica
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
							printf("giocatore B %d tenta di nuovo il tiro\n",getpid());	
							dprintf(cron,"giocatore B %d tenta di nuovo il tiro\n",getpid());	
						break;

						}
					}
				}
			}
		}		
	}	
	
	/*Il procsso fato si occupa dell'invio dei messaggi riguardanti l'avvenuto compimento (o meno) dell'azione 
	per cui è stato chiamato*/
	
	pidfato = fork();
		if(!pidfato){	
	
			FILE *log = fopen("conf.txt","r");	//Sezione adibita alla lettura del file di configurazione il quale contine i dati su cui fato 
															//deve basarsi per determinare la casualità degli eventi
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
			
			while(true){															//Il processo fato si occupa di ricevere i messaggi invaiti dalla funzione fato();
				sleep(1);															//una volta ricevuto il messaggio ne verifica il contenuto con delle string compare
				receive(msgID,&messaggio2,1);									//ora che sa cosa contiene puó elaborare un risultato casuale binario sulla base delle
				if(strcmp(messaggio2.text, tiro)==0)						//percentuali inserite nel file di configurazione. Ció avviene tramite le chiamate opportune a binario()
					ris = binario(perc_tiro);	
				else if(strcmp(messaggio2.text, infortunio)==0)				
					ris = binario(perc_infortunio);
				else if(strcmp(messaggio2.text, dribbling)==0)				
					ris = binario(perc_dribbling);
				
				sprintf(messaggio3.text,"%d", ris);							//Arrivato a questo punto della computazione é necessario elaborare un messaggio di risposta
				send(msgID,&messaggio3);										//tramite sprintf si scrive l'esito nel campo text di messaggio3, e poi lo si invia sulla coda in modo che la precedente recive
			}																			//(chiamata da fato()) attualemnte bloccata possa riceverlo e permettere al giocatore in questione di riprendere la computazione
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
