/*****************************************************************************
 *
 *  Reproductor MP3 basado en VS100X
 *      Autor: José Daniel Herrera Gomariz
 *      http://arduino-guay.blogspot.com.es
 *
 *   Reproduce ficheros mp3 que deben estar en la SD formateada como FAT32
 *
 *   Código basado en la 'Sparkfun MP3 Shield Library'
 *			detalles y ejemplos :	http://www.billporter.info/?p=1270
 *
 *******************************************************************************/

#include "AGMp3.h"
#include "SPI.h"

// Espera a que el chip este disponible
#define  WAIT_DREQ()  while(!digitalRead(MP3_DREQ))

/*
* Inicializa todo
*/
uint8_t AGMp3Player::inicializa(){

  // Configuramos salidas	
  pinMode(MP3_DREQ, INPUT);
  pinMode(MP3_XCS, OUTPUT);
  pinMode(MP3_XDCS, OUTPUT);
  pinMode(MP3_RESET, OUTPUT);
  
  // Desseleccionamos los buses SPI
  digitalWrite(MP3_XCS, HIGH); //Bus de Control 
  digitalWrite(MP3_XDCS, HIGH); //Bus de datos
  digitalWrite(MP3_RESET, LOW); //hardware reset
  
  //Inicializamos la tarjeta SD
  if (!SD.begin(SD_SEL)) 
    return 1;

  //La tarjeta se encarga de inicializar el bus SPI
  
  // Bus SPI  a 1MHz (16MHz / 16 = 1MHz)
  SPI.setClockDivider(SPI_CLOCK_DIV16); 
  SPI.transfer(0xFF); //Escribimos algo en el bus

  //Inicializamos el VS100X
  delay(10);
  digitalWrite(MP3_RESET, HIGH); 
  resetInicial();
  
  // Bus SPI  a 4MHz (16MHz / 4 = 4MHz)
  SPI.setClockDivider(SPI_CLOCK_DIV4); 
  
  return 0;
}

// Modifica el volumen:  0dB máx volumen 
void AGMp3Player::setVolumen(unsigned char izq, unsigned char der){
    escribeRegistro(SCI_VOL, izq, der);
}

// Reproduce un archivo
uint8_t AGMp3Player::tocaMP3(char* fichero){

	if (tocando) return 1;

    // limpiamos los registros
	reset();

	//Abrimos archivo
	if (!(cancion = SD.open(fichero))) 
	   return 2;

    tamano = cancion.size();
	tocando = TRUE;

	// LLenamos el buffer de datos
	rellena();
	  
	//Asociamos el rellenado con la interrupción DREQ en el pin 2
	attachInterrupt(0, rellena, RISING);
	  
	return 0;
}

// Devuelve la posicion actual de 0 a 100
byte AGMp3Player::getPosicion(){
	if(tocando)
		return cancion.position() * 100 / tamano ;
	else	
		return 0 ;
}

// Soft reset
void AGMp3Player::reset() {
	escribeRegistro(SCI_MODE, 0x08, SM_RESET);
	WAIT_DREQ();
	delay(100);
}

// Reset en el arranque
void AGMp3Player::resetInicial(){
	WAIT_DREQ();
	SPI.transfer(0xFF);
	
	// soft reset
	escribeRegistro(SCI_MODE, 0x08, SM_RESET);
	WAIT_DREQ();
	delay(100);

	//Multiplicador  3.0x
	escribeRegistro(SCI_CLOCKF, 0x98, 0x00); 
	escribeRegistro(SCI_AUDATA, 0x1F, 0x40);

	// Volumen decente
	setVolumen(20, 20);
	WAIT_DREQ();

	// reset decode time
	escribeRegistro(SCI_DECODE_TIME,0,0);

	// Borramos la memoria, para limpiar posible basura
	// IMPORTANTE !!! , si no es posible que tás el arranque el buffer de 
	// datos tenga basura y la primera canción no suena como debe:
	// velocidad inadecuada, ruido extraño ....
	digitalWrite(MP3_XDCS, LOW); 
	for (int i=0; i < 1250; i++) { 
		SPI.transfer(0);
	}
	digitalWrite(MP3_XDCS, HIGH); 
	delay(20);
	WAIT_DREQ();
}

// Para la canción
void AGMp3Player::paraMp3(){
  
	if(!tocando)
		return;
  
	//desactiva la interrupción
	detachInterrupt(0);
	tocando = FALSE;

	reset();
	
	cancion.close(); 
}

// Devuelve si esta reproduciendo o no
uint8_t AGMp3Player::estaTocando(){
	return tocando;
}

// Escribe en un registro del chip
void escribeRegistro(unsigned char direccion, unsigned char hsb, unsigned char lsb){

	//Si esta reproduciendo desactiva la interrupción
	if(tocando) {
		detachInterrupt(0);
	}
	
	//Espera a que el chip esté disponible
	WAIT_DREQ();
	//Seleccionamos  control
	digitalWrite(MP3_XCS, LOW); 

	// EL SCI consistse en byte de instruccion, byte de direccion,  y  16-bit de datos.
	SPI.transfer(0x02); //Instrucción de escritura
	SPI.transfer(direccion);
	SPI.transfer(hsb);
	SPI.transfer(lsb);
	WAIT_DREQ();
	//Deseleccionamos Control
	digitalWrite(MP3_XCS, HIGH); 
	
	// Si estaba reproduciendo activamos la interrupción 
	if(tocando)	{
		//Mira a ver si está preparado para mas
		rellena();
		//Asocia la interrupción patilla 2 (DREQ) a la rutina de relleno
		attachInterrupt(0, rellena, RISING);
	}
	
}

//Lee un registro de 16 bits del VS10xx 
unsigned int leeRegistro (unsigned char direccion){
  
	// Desactiva la interrupción si está tocando
	if(tocando)
		detachInterrupt(0);
	  
	WAIT_DREQ(); //Espera a que esté disponible
	digitalWrite(MP3_XCS, LOW); //Seleciiona el bus de control

	// EL SCI consistse en byte de instruccion, byte de direccion,  y  16-bit de datos.
	SPI.transfer(0x03);  //lle la instrucción
	SPI.transfer(direccion);

	char response1 = SPI.transfer(0xFF); //Lee el primer byte
	WAIT_DREQ(); //Espera a que esté disponible
	char response2 = SPI.transfer(0xFF); //Lee el segundo byte
	WAIT_DREQ(); //Espera a que esté disponible

	digitalWrite(MP3_XCS, HIGH); //Deselecciona el bus de  Control

	unsigned int resultvalue = response1 << 8;
	resultvalue |= response2;
	return resultvalue;
  
	//Si estaba tocando volvemos a dejarlo como estaba. 
	if(tocando)	{
		// Mira si está listo para rellenar el buffer
		rellena();

		// asociamos la interrupción  DREQ  en el pin 2
		attachInterrupt(0, rellena, RISING);
	}
}

//Rellena el buffer del VS10xx con datos nuevos
static void rellena() {
  while(digitalRead(MP3_DREQ)){
		//Intentamos leer 32 bytes de la SD 
        if(!cancion.read(mp3BufferDatos, sizeof(mp3BufferDatos))) { 
			cancion.close(); 
			tocando=FALSE;
			
			//desactivamos interrupcion
			detachInterrupt(0);
			
			//Soft reset para limpiar el buffer. 
			escribeRegistro(SCI_MODE, 0x48, SM_RESET);
			// No hay más datos
			break;
        }
      
      //Una vez el chip esta disponible, cargamos 32 bytes en el buffer
	  // Seleccionamos el bus de datos
      digitalWrite(MP3_XDCS, LOW);

      for(int y = 0 ; y < sizeof(mp3BufferDatos) ; y++) {
        SPI.transfer(mp3BufferDatos[y]); // Enviamos datos
      }
	  // Deseleccionamos el bus de datos
      digitalWrite(MP3_XDCS, HIGH); 
  }
}








































