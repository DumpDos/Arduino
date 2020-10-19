// Décommentez cette ligne pour passer en mode DEBUG
// Script original par zigazou : http://forum.snootlab.com/viewtopic.php?f=30&t=688
// Modifié par DumpDos
//#define DEBUG

#ifdef DEBUG
 #define DEBUG_PRINT(x)  Serial.println (x)
 #define DEBUG_PRINT_(x)  Serial.print (x)
 #define DEBUG_PRINTD(x)  Serial.println (x, DEC)
#else
 #define DEBUG_PRINT(x)
 #define DEBUG_PRINT_(x)
 #define DEBUG_PRINTD(x)
#endif

// Bibliothèques
#include <SD.h>
#include "SoftwareSerialExtended.h"


// Broches du port série logiciel
#define RXPIN 2
#define TXPIN 3

// Broche du lecteur de carte SD
#define CHIPSELECT 10

// Définition du port série logiciel
SoftwareSerialExtended minitelSerial(RXPIN, TXPIN); // (RX/TX)

// Déclaration des variables
int page_depart = 0;
int start_page = 170;
int Nbfile = 0;
int fichier = 0;
int touche = 0;
String ip_port = "IP:PORT";


// Garde trace des noms des fichiers de page et commandes en cours
String pageActuelle;
String commandesActuelles;
long inactivite;

typedef union {
  char raw[32];
  struct {
    char type;
    long touche;
    char page[27];
  } action;
} Enregistrement;

// ************************************************************************
// Fonction lireTouche
// Lit une éventuelle touche tapée sur le Minitel
// Si aucune touche n'a été frappée, la fonction retourne la valeur 0
// ************************************************************************
long lireTouche() {
  long c;
  touche = 0;

  minitelSerial.listen();
  while(minitelSerial.available() > 0 && (c = minitelSerial.read()) != -1) {
    touche = (touche << 8) + c;

    // Petit délai pour voir si le minitel n’a pas d’autres caractères à envoyer
    DEBUG_PRINT (touche);
    delay(20);
  }  

  return touche;
}

// ************************************************************************
// Fonction afficherPage
// Envoit une page complète au Minitel
// ************************************************************************

void afficherPage(int page) {
  
  // Copie le nom de la page
  pageActuelle = String(page);

  // Génère le nom du fichier de commandes à partir du nom de fichier de la page
  commandesActuelles = pageActuelle;

  // Déclaration des variables
  String vidtex = "vidte";
  String vdt = ".vdt";
  File vdtFile;

  DEBUG_PRINT_ ("Affichage d'une page : ");
  DEBUG_PRINT (page);
  
  // Ouvre la page à afficher
  if(vdtFile = SD.open(vidtex+page+vdt, FILE_READ)) {
      long touche;

    // Affiche la page caractère par caractère
    int16_t octet;
    while((octet = vdtFile.read()) != -1) minitelSerial.write((char)octet);

    // Ferme le fichier
    DEBUG_PRINT ("Fermeture du fichier VDT");
    vdtFile.close();
  } else {
    minitelSerial.write(0x07);
    DEBUG_PRINT ("Fichier non trouve : ");
    DEBUG_PRINT (pageActuelle.c_str());
    minitelSerial.println("Fichier non trouve : ");
    minitelSerial.println(pageActuelle.c_str());
  }
}

// ************************************************************************
// Fonction calculNbFichier
// Calcul le nombre de fichiers présents sur la carte SD.
// ************************************************************************

void calculNbFicher(File directory, int indent) {

  // ouvre le premier fichier
  File entry = directory.openNextFile();
  while (entry) {

    // Incerementation du nombre de fichier
    if (entry.isDirectory()) {
      calculNbFicher(entry, indent + 1);
    } 

    // Calcule du nombre de fichier
    else {
      Nbfile = Nbfile + 1;
    }

    // Fermeture fichier
    entry.close();
    entry = directory.openNextFile();
  }

  // retour au répertoire de départ
  directory.rewindDirectory();
}

// ************************************************************************
// Fonction effacerEcran
// Efface l'écran et le statut du minitel
// ************************************************************************

void effacerEcran () {
  
  DEBUG_PRINT ("Effacement ecran");
  minitelSerial.write(0x1f);
  minitelSerial.write(0x40);
  minitelSerial.write(0x41);
  minitelSerial.print("                                    ");
  minitelSerial.write(0x0c);
}

// ************************************************************************
// Fonction attendreTouche
// Attend l'appuie d'une touche particulière
// ************************************************************************

long attendreTouche (int x){
    
    debutfonction:
    lireTouche();
    
    if (touche == x) {
      delay(1000);
      return true;
    } else {
    goto debutfonction;
    }
    
}

// ************************************************************************
//
// Initialisation
//
// ************************************************************************

void setup() {
  
  Serial.begin(57600);
  while (!Serial);
  
  // Test si la carte SD est disponible et lisible
  DEBUG_PRINT ("Tentative de lecture de la carte SD");
  if (!SD.begin(CHIPSELECT)) {
    minitelSerial.println("Impossible de lire la carte SD");
    DEBUG_PRINT ("Impossible de lire la carte SD");
    return;
  }

  
  // calcul du nombre de fichiers présents sur la carte
  DEBUG_PRINT ("Calcul du nombre de fichiers...");
  calculNbFicher(SD.open("/"), 0);
  fichier = (Nbfile-3);
  DEBUG_PRINT_ ("Nombre de fichiers : ");
  DEBUG_PRINT (fichier);

  DEBUG_PRINT ("Initialisation de la liaison serie avec le Minitel");
  minitelSerial.begin(1200, 7);

  // Initialisation de l'affichage
  delay(1000);
  effacerEcran ();
  
  #ifdef DEBUG
  DEBUG_PRINT ("Affichage des messages de debugging");
  minitelSerial.print(fichier);
  minitelSerial.print(" fichiers videotexte sur la carte");
  delay(3000);
  effacerEcran ();
  #endif

  
  DEBUG_PRINT ("Affichage des credits");
  afficherPage(0);
  attendreTouche(4929);
  afficherPage(1);
  afficherPage(2);
}

void decimal (int x){
  touche = (x-48);
}

// ************************************************************************
// Menu interrupteur
// ************************************************************************

void inter() {
    
    // Initialisation des variables
    String inter0 = "/json.htm?type=command&param=switchlight&idx=";
    String inter1 = "&switchcmd=Toggle";
    String inter2 = "";
    inter2 = inter0;
    
    // Affichage page 3
    afficherPage(3);
    
    // Retour boucle
    boucle1:
 
    // Boucle d'attente touche minitel 
    lireTouche();
 
    // Touche "ENVOI"
    if (touche == 4929) {
      
      inter2 = (ip_port+inter2+inter1);
      Serial.print (inter2);
      afficherPage(2);
      
     // Touche "RETOUR"
    } else if (touche == 4930) {
      
      afficherPage(2);
      
     // Pavé numérique
    } else if (touche >= 48 && touche <= 57) {
      
      decimal(touche);
      inter2 = (inter2+touche);
      goto boucle1;
      
     // Retour début boucle
    } else {
      
      goto boucle1;
    }
}

// ************************************************************************
// Menu variateur
// ************************************************************************

void varia() {
    
    // Initialisation des variables
    String varia0 = "/json.htm?type=command&param=switchlight&idx=";
    String varia1 = "&switchcmd=Set%20Level&level=";
    String varia2 = "";
    varia2 = varia0;
    
    // Affichage page 4
    afficherPage(4);
    
    // Retour boucle
    boucle2:
 
    // Boucle d'attente touche minitel 
    lireTouche();
 
    // Touche "ENVOI"
    if (touche == 4929) {
     
      varia2 = (ip_port+varia2);
      Serial.print (varia2);
      afficherPage(2);
     
     // Touche "RETOUR"
    } else if (touche == 4930) {
     
      afficherPage(2);
     
     // Touche "SUITE"
    } else if (touche == 4936) {
     
      minitelSerial.write(0x1f);
      minitelSerial.write(0x4c);
      minitelSerial.write(0x4e);
      varia2 = (varia2+varia1);
      goto boucle2;
     
     // Pavé numérique
    } else if (touche >= 48 && touche <= 57) {
     
      decimal(touche);
      varia2=(varia2+touche);
      goto boucle2;
     
     // Retour début boucle
    } else {
     
      goto boucle2;
    }
}

// ************************************************************************
// Menu variateur
// ************************************************************************

void scene() {

    // Initialisation des variables
    String scene0 = "/json.htm?type=command&param=switchscene&idx=";
    String scene1 = "&switchcmd=On";
    String scene2 = "";
    scene2 = scene0;
    
    // Affichage page 5
    afficherPage(5);
 
    // Retour boucle
    boucle3:
 
    // Boucle d'attente touche minitel 
    lireTouche();
 
    // Touche "ENVOI"
    if (touche == 4929) {
     
      scene2 = (ip_port+scene2+scene1);
      Serial.print (scene2);
      afficherPage(2);
     
     // Touche "RETOUR"
    } else if (touche == 4930) {
      
      afficherPage(2);
     
     // Pavé numérique
    } else if (touche >= 48 && touche <= 57) {
     
      decimal(touche);
      scene2 = (scene2+touche);
      goto boucle3;
     
     // Retour début boucle
    } else {
     
      goto boucle3;
    }
}

// ************************************************************************
//
// Boucle principale
//
// ************************************************************************

void loop() {

  lireTouche();
  
  if (touche == 49) {
    inter();
  }
  else if (touche == 50) {
    varia();
  }
  else if (touche == 51) {
    scene();
  }
  else if (touche == 52) {
    afficherPage(6);
  }
  else if (touche == 4930) {
    afficherPage(2);
  }
  
}
