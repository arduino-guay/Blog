/*****************************************************************************
 *
 *  Vela electronica con Arduino Pro Mini y Control Remoto
 *      Autor: José Daniel Herrera Gomariz
 *      http://arduino-guay.blogspot.com.es
 *
 *   Simula el funcionamiento de una vela sobre una tira de leds tipo NeoPixel
 *   Se controla desde un mando a distancia estandar de los utilizados para las 
 *   bombillas LED de colores.
 *
 *******************************************************************************/
#include <Adafruit_NeoPixel.h>
#include <IRremote.h>

#define PIXEL_PIN 7    // Pin para la tira de leds
#define RECV_PIN  2    // Pin pra el IR 

// Constantes para los códigos del mando a distancia
#define ON                0XFFE01F
#define OFF               0xFF609F
#define BRIGHTNESS_UP     0xFFA05F
#define BRIGHTNESS_DOWN   0xFF20DF
#define FLASH             0xFFF00F
#define STROBE            0xFFE817
#define FADE              0xFFD827
#define SMOOTH            0xFFC837

#define RED               0xFF906F
#define GREEN             0XFF10EF
#define BLUE              0xFF50AF
#define WHITE             0xFFD02F

#define ORANGE            0xFFB04F
#define YELLOW_DARK       0xFFA857
#define YELLOW_MEDIUM     0xFF9867
#define YELLOW_LIGHT      0xFF8877

#define GREEN_LIGHT       0XFF30CF
#define GREEN_BLUE1       0XFF28D7
#define GREEN_BLUE2       0XFF18E7
#define GREEN_BLUE3       0XFF08F7

#define BLUE_RED          0XFF708F
#define PURPLE_DARK       0XFF6897
#define PURPLE_LIGHT      0XFF58A7
#define PINK              0XFF48B7

#define INCREMENTO 10

// Distintos efectos de la vela
#define MODO_COLOR        0
#define MODO_FUEGO        1
#define MODO_ARCOIRIS     2
#define MODO_FADE         3
#define MODO_FUEGO2       4
#define MAX_PIXEL         5

unsigned long rgb = 0;
byte r,g,b,brillo;
// Estado actual
byte estado = MODO_FUEGO2;
byte idx_arcoIris =0;
byte off = 0;

// Parameter 1 = number of pixels in strip
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(10, PIXEL_PIN, NEO_GRB + NEO_KHZ800);

IRrecv irrecv(RECV_PIN);

decode_results results;

// Obtiene un color de toda la gama a partir de un valor de 0 to 255 
// los colores son transiciones de  rojo --> verde --> azul --> rojo
uint32_t CicloColor(byte posicionCiclo) {
  if(posicionCiclo < 85) {
   return strip.Color(posicionCiclo * 3, 255 - posicionCiclo * 3, 0);
  } else if(posicionCiclo < 170) {
   posicionCiclo -= 85;
   return strip.Color(255 - posicionCiclo * 3, 0, posicionCiclo * 3);
  } else {
   posicionCiclo -= 170;
   return strip.Color(0, posicionCiclo * 3, 255 - posicionCiclo * 3);
  }
}

//Obtiene un tono rojo en función del indice
uint32_t TonoRojo(byte indice) {
   return strip.Color(200, indice, 0);
}

// Pone un pixel a un color
void setPixel (byte pixel, uint32_t color) {
   strip.setPixelColor(pixel, color);
}

// Pone toda la tira al mismo color
void setColor (byte r, byte g, byte b) {
  for (byte p=0; p < MAX_PIXEL; p++) {
    strip.setPixelColor(p, strip.Color(r,g,b));
  }
  strip.show(); 
}

// Apaga todos los pixels
void clear() {
  setColor(0,0,0);
}


void setup() {
  // Inicializamos la tira de leds	
  strip.begin();
  // Inicializamos el receptor IR	
  irrecv.enableIRIn(); 
  strip.show(); 
  // Cambiamos la 'semilla' de los aleatorios
  randomSeed(analogRead(0));
  clear();
  brillo = 255;
  r = g = b = 0;
}

// Simula el fuego variando el numero, tono de rojo, brillo de los pixels y tiempo
void fuego () {
    byte cuantos = random(0,6);
    byte lum = random(100,255);
    uint32_t color = TonoRojo(50+random(1,180)); 
    for(int i=0; i <= cuantos; i++) {
      setPixel(i, color);
      strip.setBrightness(lum);
      strip.show();
      delay(random(0,10));
    }
    strip.setBrightness(brillo);
    delay(100);
}

// Simula el fuego variando el tono de rojo, brillo de los pixels y tiempo
void fuego2 () {
    uint32_t color = TonoRojo(random(100,150));
    for (byte p=0; p < MAX_PIXEL; p++) {
      strip.setPixelColor(p, color);
    }
    strip.setBrightness(random(50,255));
    strip.show();
    delay(random(20,300));
    strip.setBrightness(brillo);
}

// Crea un efecto arcoIris, los pixel tiene colores distintos
void arcoIris() {
  uint16_t i;

  idx_arcoIris = (idx_arcoIris+1) & 255; 
  for(i=0; i < MAX_PIXEL; i++) {
    setPixel(i, CicloColor((i*30+idx_arcoIris) & 255));
  }
  strip.show();
  delay(20);
}

// Va cambiado de color pasando por todos , todos los pixel iguales
void fade() {
  idx_arcoIris = (idx_arcoIris+1) & 255; 
  for (byte p=0; p < MAX_PIXEL; p++) {
    strip.setPixelColor(p, CicloColor(idx_arcoIris));
  }
  strip.show();
  delay(100);
}

void captura () {
  if (irrecv.decode(&results)) {
    if ( results.value != 0xFFFFFFFF) {
      switch (results.value) {
           case BRIGHTNESS_UP : 
               if ( brillo < 255-INCREMENTO ) {
                 brillo+=INCREMENTO; 
               }
               strip.setBrightness(brillo);
               break; 
           case BRIGHTNESS_DOWN : 
               if ( brillo > INCREMENTO ) {
                 brillo-=INCREMENTO; 
               }
               strip.setBrightness(brillo);
               break; 
           case OFF :
               strip.setBrightness(0);
    		   strip.show();
               off = 1;
               break;    
           case ON :
               strip.setBrightness(brillo);
               off = 0;
               break;    
           case STROBE :
               estado = MODO_FUEGO;
               break;    
           case FLASH :
               estado = MODO_ARCOIRIS;
               break;    
           case FADE :
               estado = MODO_FADE;
               break;    
           case SMOOTH :
               estado = MODO_FUEGO2;
               break;    
           case RED           : RGB(0x00FF0000); break;
           case GREEN         : RGB(0x0000FF00); break;
           case BLUE          : RGB(0x000000FF); break;
           case WHITE         : RGB(0x00FFFFFF); break;
           case ORANGE        : RGB(0x00FF7F00); break;
           case YELLOW_DARK   : RGB(0x00FFAA00); break;
           case YELLOW_MEDIUM : RGB(0x00FFD400); break;
           case YELLOW_LIGHT  : RGB(0x00FFFF00); break;
           case GREEN_LIGHT   : RGB(0x0000FFAA); break;
           case GREEN_BLUE1   : RGB(0x0000FFFF); break;
           case GREEN_BLUE2   : RGB(0x0000AAFF); break;
           case GREEN_BLUE3   : RGB(0x000055FF); break;
           case BLUE_RED      : RGB(0x00000080); break;
           case PURPLE_DARK   : RGB(0x003F0080); break;
           case PURPLE_LIGHT  : RGB(0x007A00BF); break;
           case PINK          : RGB(0x00FF00FF); break;
      }
      if ( estado == MODO_COLOR ) {
          setColor(r,g,b);
      }
    }
    irrecv.resume(); // Receive the next value
  }
}


void RGB(unsigned long valor) {
   r = valor >> 16; 
   g = (valor >> 8) & 0xFF; 
   b = valor & 0xFF;
   estado = MODO_COLOR;
}

void loop() {
  captura();
  captura();
  captura();
  captura();
  captura();
  captura();
  if ( !off ) {
	  switch ( estado) {
	    case  MODO_ARCOIRIS:  arcoIris(); break;
	    case  MODO_FUEGO:     fuego(); break;
	    case  MODO_FUEGO2:    fuego2(); break;
	    case  MODO_FADE :     fade(); break;
	  }
  }
}

