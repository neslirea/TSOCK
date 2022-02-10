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
        printf("\n");
    } 
}

void main (int argc, char **argv)
{
	int c;
	extern char *optarg;
	extern int optind;
	int nb_message = -1; /* Nb de messages à envoyer ou à recevoir, par défaut : 10 en émission, infini en réception */
	int source = -1 ; /* 0=puits, 1=source */
    int protocole = -1; /* 0 = UDP sinon TCP */
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
        printf("le protocole utilise est UDP");
    }
    else{
        printf("le protocole utilise est TCP")
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
}