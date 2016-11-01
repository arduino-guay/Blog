/*****************************************************************************
 *
 *  Reproductor MP3 basado en VS100X
 *      Autor: José Daniel Herrera Gomariz
 *      http://arduino-guay.blogspot.com.es
 *
 *   Reproduce ficheros mp3 que deben estar en la SD formateada como FAT32
 *   Utiliza un módulo basado en un chip VS100X y un LCD 20x4
 *
 *******************************************************************************/
#include <SPI.h>
#include <SD.h>

#include <AGMp3.h>
#include <LiquidCrystal.h>

// Pines para el LCD
#define LCD_RS  3
#define LCD_Enable 4
#define LCD_D4  A0
#define LCD_D5  A1
#define LCD_D6  A2
#define LCD_D7  A3

//Pines Encoder
#define ENC_A 8
#define ENC_B 10
//Pines botones
#define BOTON_ADELANTE 0
#define BOTON_ATRAS 1

// Minimo volumen dB
#define MIN_VOL 60

// Máximo nº de entradas en el catálogo
#define MAX_ENTRADAS  15
// Nº de caracteres en cada entrada
#define MAX_LONG 20
#define MAX_LONGMAS1 21

// Definición de caracteres para el progreso de la canción
byte bar1[8] = { B10001, B10001, B10001,  B10001,  B10001,  B10001,  B10001,  B10001 };
byte bar2[8] = { B11001, B11001, B11001,  B11001,  B11001,  B11001,  B11001,  B11001 };
byte bar3[8] = { B11101, B11101, B11101,  B11101,  B11101,  B11101,  B11101,  B11101 };
byte bar4[8] = { B11111, B11111, B11111,  B11111,  B11111,  B11111,  B11111,  B11111 };
byte bar5[8] = { B11111, B11111, B11111,  B11111,  B11111,  B11111,  B11111,  B11111 };
byte barV[8] = { B00001, B00001, B00001,  B00001,  B00001,  B00001,  B00001,  B00001 };

// Inicialización del cristal líquido
LiquidCrystal lcd(LCD_RS,LCD_Enable,A0,A1,A2,A3);

//Creamos el objeto MP3
AGMp3Player MP3player;

//Arcvhivo de catálogo
File fTexto;
char nombreFichero[31];

byte temp;
byte result;

//Variables con el artista, album y pista seleccionados
char strArtista[MAX_LONGMAS1];
char strAlbum[MAX_LONGMAS1];
char strTitulo[MAX_LONGMAS1];

// Variables para conocer la entrada del catalogo actual y anterior, así como el máximo de entradas
byte entrada = 0;
byte entradaA = 0;
byte maxEntradas =  0;

// Variales para controlar el encoder
byte posAnt = 0;
byte encoderAAnt = LOW;

// volumen actual
byte vol = 20;

// Catalogo de entradas
char catalogo[MAX_ENTRADAS][MAX_LONGMAS1];

byte nivel, artista, album, pista = 0;
char strLinea [30];

void errorTarjeta() {
   lcd.setCursor(0,4);
   lcd.print("Error en tarjeta SD o no presente.");
}

void mostrarProgreso() {
	byte pos = MP3player.getPosicion() ;
	if ( pos == posAnt )
	   return;
	posAnt = pos;
	byte pos10 = pos/10 ;

	lcd.setCursor(0,4);
	for (byte i=0; i < 10; i++)
	   lcd.write(5);
	lcd.setCursor(0,4);
	for (byte i=0; i < pos10; i++)
	   lcd.write(4);
	lcd.write((pos - 10*pos10)/2);

	lcd.setCursor(12,4);
	lcd.print("Vol:");
	lcd.print(map(vol,0,MIN_VOL,10,0),DEC);
	//lcd.print(FreeRam(),DEC);
	lcd.print(" ");
}

// Debouncing paraa los botones
int botonPulsado (byte boton) {
  if (digitalRead(boton) == LOW) {
	  delay(100);
	  return digitalRead(boton) == LOW;
  }
  return 0;
}

// lee una linea de texto del fichero
char* leerLinea(File myFile, char* linea) {
  byte idx = 0;
  uint16_t letra= myFile.read();
  // eliminamos retornos de carro y lf
  while (myFile.available() && (letra == 13 || letra == 10))
    letra= myFile.read();
  while (myFile.available() && (letra != 13)) {
     if (letra == 'ñ') {
		 letra = 'n' +125;
	 }
     linea[idx] = letra;
     letra = myFile.read();
     idx++;
  }
  linea[idx] = 0;
  return linea; 
}

// lee un fichero de catalogo y lo almacena en la variable catalogo
void leerFichero(char* nombre) {
    byte indice = 0; 
	byte len = 0;
    memset(strLinea,0,sizeof(strLinea));
    memset(catalogo,0,sizeof(catalogo));
    if (fTexto = SD.open(nombre)) {
      while ( fTexto.available() ) {
         leerLinea(fTexto, strLinea);
		 len = min(strlen(strLinea),MAX_LONG);
		 if ( len > 0 ) {
			strncpy(catalogo[indice],strLinea,len);
			indice++;
		 }
	  }
	  fTexto.close();	 
	  maxEntradas = indice;
	  entradaA = -1;
	  entrada = 0;
	  encoderAAnt = HIGH;
    }
    else {
		errorTarjeta();
	}
}

// Rutina para leer el encoder y modificar la entrada del catalogo
void leerEncoder() {
	byte n = digitalRead(ENC_A);
   if ((encoderAAnt == LOW) && (n == HIGH)) {
     if (digitalRead(ENC_B) == LOW) {
		 entrada = max(entrada-1,0);
     } else {
		 entrada = min(entrada+1,maxEntradas-1);
     }
   } 
   encoderAAnt = n;
}

// Rutina para leer el encoder y modificar el Volumen
void leerEncoderVol() {
   byte n = digitalRead(ENC_A);
   if ((encoderAAnt == LOW) && (n == HIGH)) {
     if (digitalRead(ENC_B) == LOW) {
         vol = min(vol+5,MIN_VOL);
		 MP3player.setVolumen(vol,vol);
     } else {
         vol = max(vol-5,0);
		 MP3player.setVolumen(vol,vol);
     }
   } 
   encoderAAnt = n;
}

void setup() {

  pinMode(9, OUTPUT);
  pinMode(BOTON_ADELANTE, INPUT);
  pinMode(BOTON_ATRAS, INPUT);
  pinMode(ENC_A,INPUT);
  pinMode(ENC_B,INPUT);
  digitalWrite(ENC_A,HIGH);
  digitalWrite(ENC_B,HIGH);
  digitalWrite(BOTON_ADELANTE,HIGH);
  digitalWrite(BOTON_ATRAS,HIGH);

  if ( MP3player.inicializa() != 0 ) {
	  errorTarjeta();
  }

  MP3player.setVolumen(vol, vol);

  lcd.begin(16, 4);

  memset(strArtista,0,MAX_LONGMAS1); 
  memset(strArtista,0,MAX_LONGMAS1);
  memset(strAlbum,0,MAX_LONGMAS1);
 
  lcd.createChar(0,bar1);
  lcd.createChar(1,bar2);
  lcd.createChar(2,bar3);
  lcd.createChar(3,bar4);
  lcd.createChar(4,bar5);
  lcd.createChar(5,barV);

  leerFichero("artistas.txt");

}

void printLCD (byte col, byte fila, byte numEntrada) {
    if ( numEntrada <= maxEntradas-1) {
		char linea[MAX_LONGMAS1];
		memset(linea,0,sizeof(linea));
		strncpy(linea,catalogo[numEntrada],20-col);
		lcd.setCursor(col,fila);
		lcd.print(linea);
	}
}

void mostrarCatalogo(byte forzar) {
  if ( (entrada != entradaA) || forzar) {
      lcd.clear(); 
	  switch(nivel) {
		 case 0: 
			lcd.write('z'+4);
			printLCD (1,0,entrada);
    	    printLCD (1,1,entrada+1);
		    printLCD (1,2,entrada+2);
		    printLCD (1,3,entrada+3);
		 break;
		 case 1: 
			lcd.print(strArtista);
			lcd.setCursor(0,1);
			lcd.print(' ');
			lcd.write('z'+4);
			printLCD (2,1,entrada);
		    printLCD (2,2,entrada+1);
		    printLCD (2,3,entrada+2);
		 break;
		 case 2: 
			lcd.print(strArtista);
			lcd.setCursor(0,1);
			lcd.print(strAlbum);
			lcd.setCursor(0,2);
			lcd.print(' ');
			lcd.write('z'+4);
			printLCD (2,2,entrada);
			printLCD (2,3,entrada+1);
		 break;
		 case 3: 
			lcd.print(strArtista);
			lcd.setCursor(0,1);
			lcd.print(strAlbum);
			lcd.setCursor(0,2);
			lcd.print(strTitulo);
		 break;
		}
		entradaA = entrada;
  }
}

void actualizar(byte adelante) {
    byte result;
	switch(nivel) {
		case 0: 
			leerFichero("artistas.txt");
			if (!adelante) {
				entrada = artista-1;
			}
		break;
		case 1: 
			if (adelante) {
				artista = entrada+1;
			    strcpy(strArtista,catalogo[entrada]);
			} 
			sprintf(nombreFichero, "ARTIST%02d/album.txt", artista);
			leerFichero(nombreFichero);
			if (!adelante) {
				entrada = album-1;
			}
		break;
		case 2: 
			if (adelante) {
				album = entrada+1;
				strcpy(strAlbum,catalogo[entrada]);
			} 
			sprintf(nombreFichero, "ARTIST%02d/ALBUM%02d/track.txt", artista, album);
			leerFichero(nombreFichero);
			if (!adelante) {
				entrada = pista-1;
			}
		break;
		case 3: 
			pista = entrada+1; 
			strcpy(strTitulo,catalogo[entrada]);
			sprintf(nombreFichero, "ARTIST%02d/ALBUM%02d/track%03d.mp3", artista, album, pista);
			result = MP3player.tocaMP3(nombreFichero);
			if(result == 0) {
				errorTarjeta();
			}
		break;
	}
    mostrarCatalogo(true);
}

void loop() {
	if (botonPulsado(BOTON_ADELANTE) ) {
		if ( nivel < 3) {
			nivel++;
		    actualizar(1);
		}
		else if (MP3player.estaTocando()) {
			MP3player.paraMp3();
		}
	}
	if (botonPulsado(BOTON_ATRAS) ) {
		if ( MP3player.estaTocando() ) {
			MP3player.paraMp3();
		}
		if ( nivel > 0) {
			nivel--;
		}
		actualizar(0);
	}
	if ( (nivel == 3) &&  !MP3player.estaTocando() ) {
		if ( entrada < maxEntradas ) {
			entrada++;
		} else {
			nivel = 2;
		}
		actualizar(0);
	}
	if (MP3player.estaTocando()) {
		leerEncoderVol();
	} else {
		leerEncoder();
	}
    mostrarCatalogo(false);
    mostrarProgreso();
}

