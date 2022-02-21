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

#include <arpa/inet.h>


// En tant que source :
// 1 - creation d'un socket (local)
// 2 - construire @IP à atteindre = SOCKET DISTANT
// 3 - envoi de données au port PORT


// En tant que puit :
// 1 - Creation d'un socket (local)
// 2 - Adressage du socket 
// ** 2.1 - Construction de l'@ (struct sock addr_in adr)
// ** 2.2 - associer @ et socket (= bind())

void construire_message(char *message, char motif, int lg) {
    int i;
    for (i=0;i<lg;i++){
        message[i] = motif;
    }
}

void construire_message2(char *message, char motif, int num, int lg) {
    construire_message(message, motif, lg);
	message[lg]='\0';
	if (lg>5){
		char * nb = malloc(sizeof(char)*6);
		sprintf(nb, "%d", num);
		memset(message,(char)32,5);
		memcpy(message+(5-strlen(nb)), nb, strlen(nb));
		free(nb);
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
void afficher_message_source(int lg_message, int nb_envoi, char *message){
	printf("SOURCE:Envoi n°%i (%i)[%s]\n", nb_envoi, lg_message, message);
}
void afficher_message_puit(int lg_message, int nb_recep, char *message){
	printf("PUITS:Reception n°%i (%i)[%s]\n", nb_recep, lg_message, message);
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

    // Variable pour message
    char * pmesg = malloc(sizeof(char)*(taille_donnee+2));
	int nbOctets;


	while ((c = getopt(argc, argv, "upl:n:s")) != -1) { // ":" signifie que la valeur attend un argument
		switch (c) {
		case 'p': // tsock -p [-options] PORT --> exécution en tant que puit sur le port PORT
			if (source == 1) { // variable en désaccord avec l'option précisée
				printf("usage: cmd [-p|-s][-n ##][-l ##]\n"); 
				exit(1); // On quitte le programme
			}
			source = 0;
			break;

		case 's': // tsock -s [-options] PORT --> execution en tant de source sur le port PORT
			if (source == 0) { // variable en désaccord avec l'option précisée
				printf("usage: cmd [-p|-s][-n ##][-l ##]\n");
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

		case 'l': // -l : représente le nombre d'octets par message
			taille_donnee = atoi(optarg);
			break;


		default:
			printf("usage: cmd [-p|-s][-n ##][-l ##]\n");
			break;
        }
	}

	if (source == -1) {
		printf("usage: cmd [-p|-s][-n ##][-l ##]\n");
		exit(1) ;
	}
    
    if (protocole != 0){
		type = SOCK_STREAM;
		proto = IPPROTO_TCP;
    }
	
	// --- SOURCE --- //
	if(source == 1){ 
		if (protocole == 0){
			printf("SOURCE:lg_mesg_emis=%i,port=%i,nb_envois=%i,TP=udp,dest=%s\n", taille_donnee, port, nb_message, host);
		}
		else{
			printf("SOURCE:lg_mesg_emis=%i,port=%i,nb_envois=%i,TP=tcp,dest=%s\n", taille_donnee, port, nb_message, host);
		}

		////--- CREATION DU SOCKET LOCAL --- ////
		int sock = socket(domaine, type, proto); // Renvoie -1 SI erreur SINON renvoie une representation interne du socket)
		// utile socket --> http://manpagesfr.free.fr/man/man2/socket.2.html
		if(sock == -1){
			printf("erreur socket (local)");
			exit(1);
		}

		//// --- CONSTRUCTION SOCKET DISTANT + @ --- ////
		// déclaration 
		struct hostent *hp;
		struct sockaddr_in adr_distant;
        int lg_adr_distant = sizeof(adr_distant);

		// affectation domaine et port
		memset((char*)&adr_distant, 0, sizeof(adr_distant)); // A COMPLETER
		adr_distant.sin_family = AF_INET; // domaine internet
		adr_distant.sin_port = port; // port port

		// affection @IP 
		if((hp = gethostbyname(host)) == NULL){
			printf("erreur gethostbyname\n");
			exit(1);
		}
		memcpy((char*)&(adr_distant.sin_addr.s_addr), hp->h_addr, hp->h_length);

        if(protocole == 0){ // Si on utilise le protocole UDP
            //// --- EMISSION DU MESSAGE --- //
            char * message = malloc(sizeof(char)*(taille_donnee+1));
            for (int i=0;i<nb_message;i++){
				construire_message2(message, 'a'+(i%26), i, taille_donnee);

                afficher_message_source(taille_donnee, i+1, message);
                //Envoi du message 
                sendto(sock,message,taille_donnee, 0,(const struct sockaddr *)&adr_distant, lg_adr_distant);
			}
		free(message);
        }

        else{ // Si on utilise le protocole TCP
            int etabConnexion = connect(sock,(const struct sockaddr *)&adr_distant, lg_adr_distant);
            if(etabConnexion == -1){
                printf("echec de l'etablissement de la connexion \n");
                exit(1);
            }  

            int i = 0;
            while (( nbOctets!= -1) && (i < nb_message)){
                construire_message2(pmesg, 'a'+(i%26), i, taille_donnee);
                afficher_message_source(taille_donnee, i+1, pmesg);
                nbOctets = write(sock,pmesg,taille_donnee);
                i++;
              }
        }
	}

	// --- PUIT --- //
	else{ 

		//// --- CREATION DU SOCKET LOCAL --- ////
		int sock = socket(domaine, type, proto); // Renvoie -1 SI erreur SINON renvoie une representation interne du socket)
		// utile socket --> http://manpagesfr.free.fr/man/man2/socket.2.html
		if(sock == -1){
			printf("erreur socket (local)");
			exit(1);
		}

		//// --- CONSTRUCTION @ SOCKET LOCAL  --- //// 

		// déclaration 
		struct sockaddr_in adr_local;
		int lg_adr_local = sizeof(adr_local);

		// affectation domaine et port
		memset((char*)&adr_local, 0, sizeof(adr_local));
		adr_local.sin_family = AF_INET; // domaine internet
		adr_local.sin_port = port; // port port
		adr_local.sin_addr.s_addr = INADDR_ANY; // on récupère sur toutes nos cartes réseaux (Wi-Fi et Ethernet (vive ethernet))

		// associer @ et socket
		int bind_addr = bind(sock,(const struct sockaddr *)&adr_local,lg_adr_local);

		if(bind_addr == -1){
			printf("echec du bind");
			exit(1);
		}

        struct sockaddr_in adr_em;
        int lg_adr_em = sizeof(adr_em);

		// --- UDP --- //
        if(protocole == 0){ 
			printf("PUITS:lg_mesg-lu=%i, port=%i, nb_receptions=%s, TP=udp\n",taille_donnee, port, "infini");
            //reception
			int i=1;
            while ((nbOctets=recvfrom(sock, pmesg, taille_donnee, 0, (struct sockaddr*)&adr_em, &lg_adr_em)) != -1){
                afficher_message_puit(nbOctets, i, pmesg);
				i++;
            }
                
                //fermeture
            if (close(sock)==-1) {
                printf("échec de destruction du socket\n");
                exit(1);
            }
        }    

		// --- TCP --- //
        else{ 
			printf("PUITS:lg_mesg-lu=%i, port=%i, nb_receptions=%s, TP=tcp\n",taille_donnee, port, "infini");
            int ecoute = listen(sock,1);
            if(ecoute == -1){
                printf("Erreur ecoute\n");
                exit(1);
            }

            int acceptation = accept(sock,(struct sockaddr*)&adr_em, &lg_adr_em); // socket dédié à la récéption des données
            if(acceptation == -1){
                printf("echec de l'etablissement de la connexion");
                exit(1);
            }

			//Affiche l'acceptation de la connection avec l'adresse IP de la source
            printf("PUITS:connexion avec %s acceptee\n", inet_ntoa(adr_em.sin_addr));

			// Lecture des messages
			int i =1;
            while ((nbOctets=read(acceptation, pmesg, taille_donnee)) > 0){
                afficher_message_puit(nbOctets, i, pmesg);
				i++;
            }
			
			// Fermeture
			printf("fin de la connexion\n");
            if (close(sock)==-1) {
                printf("echec de destruction du socket\n");
                exit(1);
            }
        }

	}


}