/* librairie standard ... */
#include <stdlib.h>
/* pour getopt */
#include <unistd.h>
/* déclaration des types de base */
#include <sys/types.h>
/* constantes relatives aux domaines, types et protocoles */
#include <sys/socket.h>
/* constantes et structures propres au domaine UNIX */
#include <sys/un.h>
/* constantes et structures propres au domaine INTERNET */
#include <netinet/in.h>
/* structures retournées par les fonctions de gestion de la base de
données du réseau */
#include <netdb.h>
/* pour les entrées/sorties */
#include <stdio.h>
/* pour la gestion des erreurs */
#include <errno.h>


// En tant que source :
// 1 - creation d'un socket (local)
// 2 - construire @IP à atteindre = SOCKET DISTANT
// 3 - envoi de données au port PORT

void construire_message(char *message, char motif, int lg) {
    int i;
    for (i=0;i<lg;i++){
        message[i] = motif;
    }
}

void afficher_message(char *message, int lg){
    int i;
    printf("message construit : ");
    for (i=0;i<lg;i++){ 
        printf("%c", message[i]);
    } 
	printf("\n");
}

void main (int argc, char **argv)
{
	int c;
   	extern char *optarg;
   	extern int optind;
  	int port = htons(atoi(argv[argc-1]));
	char* host = argv[argc-2];
   	int nb_message = 10; /* Nb de messages à envoyer ou à recevoir, par défaut : 10 en émission, infini en réception */
   	int taille_donnee = 30;
   	int source = -1 ; /* 0=puits, 1=source */
   	int protocole = -1; /* 0 = UDP sinon TCP */

	// Variables construction socket
	int domaine = AF_INET;
	int type;
	int proto;

	while ((c = getopt(argc, argv, "upn:s")) != -1) {
		switch (c) {
		case 'p': // tsock -p [-options] PORT --> exécution en tant que puit sur le port PORT
			if (source == 1) { // variable en désaccord avec l'option précisée
				printf("usage: cmd [-p|-s][-n ##]\n"); 
				exit(1); // On quitte le programme
			}
			source = 0;
			break;

		case 's': // tsock -s [-options] PORT --> execution en tant de source sur le port PORT
			if (source == 0) { // variable en désaccord avec l'option précisée
				printf("usage: cmd [-p|-s][-n ##]\n");
				exit(1); // On quitte le programme
			}
			source = 1;
			break;

		case 'n': // -n ## : définit le nombre de messages soit à émettre pour la source (par défaut : 10), soit à lire pour le puits (par défaut : infini)
			nb_message = atoi(optarg);
			break;
        
        case 'u': // -u : utilise le service du protocole UDP; par défaut, le protocole TCP est utilisé
            protocole = 0;
			type = SOCK_DGRAM;
			proto = IPPROTO_UDP;
            break;

		default:
			printf("usage: cmd [-p|-s][-n ##]\n");
			break;
        }
	}

	if (source == -1) {
		printf("usage: cmd [-p|-s][-n ##]\n");
		exit(1) ;
	}

	if (source == 1){
		printf("on est dans le source\n");
    }
	else{
		printf("on est dans le puit\n");
    }
    
    if (protocole == 0){
        printf("le protocole utilise est UDP \n");
    }
    else{
        printf("le protocole utilise est TCP \n");
    }

	if (nb_message != -1) {
		if (source == 1)
			printf("nb de tampons à envoyer : %d\n", nb_message);
		else
			printf("nb de tampons à recevoir : %d\n", nb_message);
	} 
    else {
		if (source == 1) {
			nb_message = 10 ;
			printf("nb de tampons à envoyer = 10 par défaut\n");
		} 
        else{
		    printf("nb de tampons à envoyer = infini\n");
        }
	}

	//// --- CREATION DU SOCKET LOCAL --- ////
	int sock = socket(domaine, type, proto); // Renvoie -1 SI erreur SINON renvoie une representation interne du socket)
	// utile socket --> http://manpagesfr.free.fr/man/man2/socket.2.html
	if(sock == -1){
		printf("erreur socket (local)");
		exit(1);
	}

	printf("sock = %d \n", sock);

	//// --- CONSTRUCTION SOCKET DISTANT + @ --- //// 
	// déclaration 
	struct hostent *hp;
	struct sockaddr_in adr_distant;

	// affectation domaine et port
	memset((char*)&adr_distant, 0, sizeof(adr_distant));
	adr_distant.sin_family = AF_INET; // domaine internet
	adr_distant.sin_port = port; // port port

	// affection @IP 
	if((hp = gethostbyname(host)) == NULL){
		printf("erreur gethostbyname\n");
		exit(1);
	}
	memcpy((char*)&(adr_distant.sin_addr.s_addr), hp->h_addr, hp->h_length);

	//// --- EMISSION DU MESSAGE --- //
	int lg_adr_distant = sizeof(adr_distant);
	char * message = malloc(sizeof(char)*(taille_donnee+1));
   	for (int i=0;i<nb_message;i++){
       	construire_message(message, (char)65+i, taille_donnee);
		printf("\n");
		//printf("%s",message);
       	afficher_message(message, taille_donnee);
       	//Envoi du message 
		sendto(sock,message,taille_donnee, 0,(const struct sockaddr *)&adr_distant, lg_adr_distant);
	   	}
   	free(message);


}