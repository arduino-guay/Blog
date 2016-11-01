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
#ifndef AGMP3_h
#define AGMP3_h

#include <SPI.h>
#include <SD.h>


static File cancion;
static uint8_t tocando;

//buffer para los bytes del mp3
static uint8_t mp3BufferDatos[32];

/***************************************
* OJO modificar estos valores según el módulo
* utilizado
****************************************/
#define MP3_XCS 6   // CS Registros
#define MP3_XDCS 7  // CS Datos (BSYNC)
#define MP3_DREQ 2  // Interrupción chip disponible
#define MP3_RESET 5 // Reset activo bajo
#define SD_SEL 9    // CD tarjeta SD 

#define SPI_MOSI 11 //SPI
#define SPI_MISO 12 //SPI
#define SPI_CLK  13 //SPI

//Registros SCI VS10xx
#define SCI_MODE 0x00
#define SCI_STATUS 0x01
#define SCI_BASS 0x02
#define SCI_CLOCKF 0x03
#define SCI_DECODE_TIME 0x04
#define SCI_AUDATA 0x05
#define SCI_WRAM 0x06
#define SCI_WRAMADDR 0x07
#define SCI_HDAT0 0x08
#define SCI_HDAT1 0x09
#define SCI_AIADDR 0x0A
#define SCI_VOL 0x0B
#define SCI_AICTRL0 0x0C
#define SCI_AICTRL1 0x0D
#define SCI_AICTRL2 0x0E
#define SCI_AICTRL3 0x0F

//Mascaras VS10xx SCI_MODE 
#define SM_RESET 0x04
#define SM_CANCEL 0x08
#define SM_DIFF      0
#define SM_SETTOZERO    1
#define SM_OUTOFWAV    3
#define SM_PDOWN    4
#define SM_TESTS    5
#define SM_STREAM    6
#define SM_PLUSV    7
#define SM_DACT      8
#define SM_SDIORD    9
#define SM_SDISHARE    10
#define SM_SDINEW    11
#define SM_ADPCM    12
#define SM_ADPCM_HP    13

#define TRUE  1
#define FALSE  0

// Rutina estatica que se asociará a la interrupción para el 
// rellenado del buffer
static void rellena();
void escribeRegistro(unsigned char addressbyte, unsigned char highbyte, unsigned char lowbyte);
unsigned int leeRegistro (unsigned char addressbyte);

/* *****************************
* Clase principal 
*******************************/
class AGMp3Player {
	public:
		uint8_t inicializa();
		void setVolumen(uint8_t, uint8_t);
		uint8_t tocaMP3(char*);
		void paraMp3();
		uint8_t estaTocando();
		byte getPosicion();

	private:
		void resetInicial();
		void reset();
		unsigned long tamano;
};


#endif