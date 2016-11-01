/*******************************************************************************************
 *
 *  Caleisdoscopio 
 *      Autor: José Daniel Herrera Gomariz
 *      Proyecto completo en: http://arduino-guay.blogspot.com.es/2014/06/caleidoscopio-con-arduino-y-tiras-de.html
 *
 *   Caleidoscopio basado en un Arduino y una tira de leds rgb direccionable (WS2812 ,WS2812b o similar).
 *   Utiliza la librería Neopixel de Adafruit : https://github.com/adafruit/Adafruit_NeoPixel
 ********************************************************************************************/

#include <Adafruit_NeoPixel.h>

#define PIN 7
#define RETARDO 500

// Parameter 1 = number of pixels in strip
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(60, PIN, NEO_GRB + NEO_KHZ800);
byte color = 0;

// Radio 0-11, pos 0-4
void setPixel (byte radio, byte pos, uint32_t color) {
    if ( radio >= 0 && radio <= 11 && pos >=0 && pos <= 4 ) { 
      strip.setPixelColor(radio*5 + pos, color);
    }
}

void ponBola (byte radio, byte pos, byte patron, uint32_t color) {
    setPixel(radio, pos, color);
    if ( patron == 1) {
      setPixel(radio-1, pos, color);
      setPixel(radio+1, pos, color);
      setPixel(radio, pos-1, color);
      setPixel(radio, pos+1, color);
    } else if ( patron == 2) { 
      setPixel(radio-1, pos, color);
      setPixel(radio+1, pos, color);
    } else if ( patron == 3) { 
      setPixel(radio, pos-1, color);
      setPixel(radio, pos+1, color);
    } 
}

// Radio 0-2, pos 0-4
void ponPixel4 (byte radio, byte pos, byte tamano, uint32_t color) {
    ponBola(radio, pos, tamano, color);
    ponBola(5-radio, pos, tamano, color);
    ponBola(6+radio, pos, tamano, color);
    ponBola(11-radio, pos, tamano, color);
}

// Radio 0-1, pos 0-4
void ponPixel3 (byte radio, byte pos, byte tamano, uint32_t color) {
    if ( radio == 0 ) {
      ponBola(0, pos, tamano, color);
      ponBola(3, pos, tamano, color);
      ponBola(4, pos, tamano, color);
      ponBola(7, pos, tamano, color);
      ponBola(8, pos, tamano, color);
      ponBola(11, pos, tamano, color);
    } else {
      ponBola(1, pos, tamano, color);
      ponBola(2, pos, tamano, color);
      ponBola(5, pos, tamano, color);
      ponBola(6, pos, tamano, color);
      ponBola(9, pos, tamano, color);
      ponBola(10, pos, tamano, color);
    }
}

void  rotar4 () {
uint32_t tmp;  
    for (byte p=0; p < 5; p++) {
         tmp = strip.getPixelColor(p);
         ponPixel4(0, p, 0, strip.getPixelColor(p+5));
         ponPixel4(1, p, 0, strip.getPixelColor(p+10));
         ponPixel4(2, p, 0, tmp);
    } 
}

void  avanzar4 () {
byte tmp;  
    for (byte r=0; r < 3; r++) {
       ponPixel4(r, 4, 0, strip.getPixelColor(r*5 + 3));
       ponPixel4(r, 3, 0, strip.getPixelColor(r*5 + 2));
       ponPixel4(r, 2, 0, strip.getPixelColor(r*5 + 1));
       ponPixel4(r, 1, 0, strip.getPixelColor(r*5));
    }  
}

void  rotar3 () {
uint32_t tmp;  
    for (byte p=0; p < 4; p++) {
         tmp = strip.getPixelColor(p);
         ponPixel3(0, p, 0, strip.getPixelColor(p+5));
         ponPixel3(1, p, 0, strip.getPixelColor(p+10));
         ponPixel3(2, p, 0, tmp);
    } 
}

void  avanzar3 () {
byte tmp;  
    for (byte r=0; r < 2; r++) {
       ponPixel3(r, 4, 0, strip.getPixelColor(r*5 + 3));
       ponPixel3(r, 3, 0, strip.getPixelColor(r*5 + 2));
       ponPixel3(r, 2, 0, strip.getPixelColor(r*5 + 1));
       ponPixel3(r, 1, 0, strip.getPixelColor(r*5));
    }  
}

void clear() {
  for (byte p=0; p < 60; p++) {
    strip.setPixelColor(p, strip.Color(0,0,0));
  }
}

void ponBarra(byte radio, byte valor,uint32_t color  ) {
  for (byte v=0; v <= valor ; v++) {
    setPixel(radio,v,color);
  }
}

void setup() {
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  randomSeed(analogRead(0));
  for (byte p=0; p < 60; p++) {
    strip.setPixelColor(p, strip.Color(0,0,255));
  }
  strip.show(); // Initialize all pixels to 'off'
}

void loop() {
  color = random(0, 11)*25;
  ponPixel4(random(0, 3),random(0, 5),random(0, 4),color ? CicloColor(color) : 0);
  ponPixel4(random(0, 3),random(0, 5),random(0, 4),color == 250 ? 0 : CicloColor(255-color));
  strip.show();
  delay(RETARDO);
  
  byte rotar = random(0, 4);
  for (byte r=0; r < rotar ; r++) { 
    rotar4();
    strip.show();
    delay(RETARDO);
  }
  
  byte avanzar = random(0, 4);
  for (byte a=0; a < avanzar ; a++) { 
    avanzar4();
    strip.show();
    delay(RETARDO);
  }
  
}

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

