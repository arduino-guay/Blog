/*******************************************************************************************
 *
 *  Arbol de Navidad Shield  
 *      Autor: José Daniel Herrera Gomariz
 *      http://arduino-guay.blogspot.com.es/2013/12/arbol-de-navidad-shield-para-arduino.html
 *
 *   Shield con forma de Árbol de Navidad, basado en leds WS2812b o WS2812
 *   Realiza un show de colores mientras suena 'Jingle Bells'
 *   Utiliza la librería Neopixel de Adafruit : https://github.com/adafruit/Adafruit_NeoPixel
 ********************************************************************************************/

// Cosntantes de las notas
#define NOTE_B0  31
#define NOTE_C1  33
#define NOTE_CS1 35
#define NOTE_D1  37
#define NOTE_DS1 39
#define NOTE_E1  41
#define NOTE_F1  44
#define NOTE_FS1 46
#define NOTE_G1  49
#define NOTE_GS1 52
#define NOTE_A1  55
#define NOTE_AS1 58
#define NOTE_B1  62
#define NOTE_C2  65
#define NOTE_CS2 69
#define NOTE_D2  73
#define NOTE_DS2 78
#define NOTE_E2  82
#define NOTE_F2  87
#define NOTE_FS2 93
#define NOTE_G2  98
#define NOTE_GS2 104
#define NOTE_A2  110
#define NOTE_AS2 117
#define NOTE_B2  123
#define NOTE_C3  131
#define NOTE_CS3 139
#define NOTE_D3  147
#define NOTE_DS3 156
#define NOTE_E3  165
#define NOTE_F3  175
#define NOTE_FS3 185
#define NOTE_G3  196
#define NOTE_GS3 208
#define NOTE_A3  220
#define NOTE_AS3 233
#define NOTE_B3  247
#define NOTE_C4  262
#define NOTE_CS4 277
#define NOTE_D4  294
#define NOTE_DS4 311
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_FS4 370
#define NOTE_G4  392
#define NOTE_GS4 415
#define NOTE_A4  440
#define NOTE_AS4 466
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_CS5 554
#define NOTE_D5  587
#define NOTE_DS5 622
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_FS5 740
#define NOTE_G5  784
#define NOTE_GS5 831
#define NOTE_A5  880
#define NOTE_AS5 932
#define NOTE_B5  988
#define NOTE_C6  1047
#define NOTE_CS6 1109
#define NOTE_D6  1175
#define NOTE_DS6 1245
#define NOTE_E6  1319
#define NOTE_F6  1397
#define NOTE_FS6 1480
#define NOTE_G6  1568
#define NOTE_GS6 1661
#define NOTE_A6  1760
#define NOTE_AS6 1865
#define NOTE_B6  1976
#define NOTE_C7  2093
#define NOTE_CS7 2217
#define NOTE_D7  2349
#define NOTE_DS7 2489
#define NOTE_E7  2637
#define NOTE_F7  2794
#define NOTE_FS7 2960
#define NOTE_G7  3136
#define NOTE_GS7 3322
#define NOTE_A7  3520
#define NOTE_AS7 3729
#define NOTE_B7  3951
#define NOTE_C8  4186
#define NOTE_CS8 4435
#define NOTE_D8  4699
#define NOTE_DS8 4978

// Notas de Jingle Bells
int melodia[] = {
  NOTE_D4, NOTE_B4, NOTE_A4, NOTE_G4, NOTE_D4, NOTE_D4, NOTE_D4,
  NOTE_D4, NOTE_B4, NOTE_A4, NOTE_G4, NOTE_E4,
  NOTE_E4, NOTE_C5, NOTE_B4, NOTE_A4, NOTE_FS4,
  NOTE_D5, NOTE_D5, NOTE_C5, NOTE_A4, NOTE_B4, NOTE_D4,
  NOTE_D4, NOTE_B4, NOTE_A4, NOTE_G4, NOTE_D4, NOTE_D4, NOTE_D4,
  NOTE_D4, NOTE_B4, NOTE_A4, NOTE_G4, NOTE_E4, NOTE_E4,
  NOTE_E4, NOTE_C5, NOTE_B4, NOTE_A4, NOTE_D5, NOTE_D5, NOTE_D5, NOTE_D5,
  NOTE_E5, NOTE_D5, NOTE_C5, NOTE_A4, NOTE_G4,
  NOTE_B4, NOTE_B4, NOTE_B4, NOTE_B4, NOTE_B4, NOTE_B4,
  NOTE_B4, NOTE_D5, NOTE_G4, NOTE_A4, NOTE_B4,
  NOTE_C5, NOTE_C5, NOTE_C5, NOTE_C5, NOTE_C5, NOTE_B4, NOTE_B4, NOTE_B4, NOTE_B4,
  NOTE_B4, NOTE_A4, NOTE_A4, NOTE_B4, NOTE_A4, NOTE_D5,
  NOTE_B4, NOTE_B4, NOTE_B4, NOTE_B4, NOTE_B4, NOTE_B4,
  NOTE_B4, NOTE_D5, NOTE_G4, NOTE_A4, NOTE_B4,
  NOTE_C5, NOTE_C5, NOTE_C5, NOTE_C5, NOTE_C5, NOTE_B4, NOTE_B4, NOTE_B4, NOTE_B4,
  NOTE_D5, NOTE_D5, NOTE_C5, NOTE_A4, NOTE_G4 };

// Duracion notas : 4 = cuarto de nota, 8 = octavo de nota, etc.:
int duracionNotas[] = {
  8, 8, 8, 8, 3, 16, 16,
  8, 8, 8, 8, 2,
  8, 8, 8, 8, 2, 
  8, 8, 8, 8, 3, 8,
  8, 8, 8, 8, 3, 16, 16,
  8, 8, 8, 8, 3, 8,
  8, 8, 8, 8, 8, 8, 8, 8,
  8, 8, 8, 8, 2,
  8, 8, 4, 8, 8, 4,
  8, 8, 6, 16, 2,
  8, 8, 6, 16, 8, 8, 8, 16, 16,
  8, 8, 8, 8, 4, 4,
  8, 8, 4, 8, 8, 4, 8, 8, 6, 16, 2,
  8, 8, 6, 16, 8, 8, 8, 16, 16,
  8, 8, 8, 8, 2 };



#include <Adafruit_NeoPixel.h>

#define PIN_LED A1
#define PIN_ZUMBADOR A0
#define PIN_EMISOR 2      // para obtener 5v pues el conector está ocupado por el árbol  
#define PIN_RECEPTOR 7    // para decidir si suena o no la melodía

#define MAX_LED 20        // leds del árbol
#define NUMPIXELS 10      // Pixels durante la melodía 

// Parameter 1 = number of pixels in strip
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)

Adafruit_NeoPixel strip = Adafruit_NeoPixel(MAX_LED, PIN_LED, NEO_GRB + NEO_KHZ800);

// Comienzo y longitud de cada fila de leds en el arbol
byte filas[8][2] = {{0,3},{3,2},{5,3},{8,2},{10,3},{13,2},{15,3},{18,2}};
byte color = 0;
byte pixel[MAX_LED];

// Envia los datos a los leds
void refresca () {
    for (byte p=0; p < MAX_LED; p++) {
        strip.setPixelColor(p, Wheel(pixel[p]));
    }
    strip.show();
}

void setup() {
  strip.begin();
  strip.show(); // todos los pixels apagados
  pinMode(PIN_EMISOR,OUTPUT);
  pinMode(PIN_RECEPTOR,INPUT);
  digitalWrite(PIN_EMISOR,HIGH);
  randomSeed(analogRead(0));
}

// http://arduino.cc/en/Tutorial/tone
void playNota (int nota) {
    // to calculate the note duration, take one second 
    // divided by the note type.
    //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
    int noteDuration = 1800/duracionNotas[nota];
    if ( digitalRead(PIN_RECEPTOR) ) {
      tone(PIN_ZUMBADOR, melodia[nota],noteDuration);
    }
  
    // to distinguish the notes, set a minimum time between them.
    // the note's duration + 30% seems to work well:
    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);
    // stop the tone playing:
    noTone(PIN_ZUMBADOR);
}

void loop() {
  // Toca el Jingle Bells 
  // Entre cada nota movemos los leds
  for (int nota = 0; nota < 100; nota++) {
    playNota(nota);
    
    // Apagamos los leds
    for (byte p=0; p < MAX_LED; p++) {
      pixel[p] = 0;
    }
    
    // Encendemos unos cuantos con colores aleatorios
    for (byte p=0; p < NUMPIXELS; p++) {
        byte nPix = random(0,MAX_LED);
        byte color = random(0,255);
        while (pixel[nPix]!= 0) { 
          nPix = random(0,MAX_LED);
        }
        pixel[nPix] = color;
    }
    refresca ();
  }
  // Varios efectos 
  ponFilas(strip.Color(255, 0, 0), 150); // Red
  ponFilas(strip.Color(0, 255, 0), 150); // Green
  ponFilas(strip.Color(0, 0, 255), 150); // Blue
  ponFilas(strip.Color(255, 0, 255), 150); // Purple
  rainbow(20);
  rainbowCycle(20);
}

// Enciende una fila del arbol de un color
void ponFilas (uint32_t c, uint8_t wait) {
  for(byte i=0; i < 8; i++) {
    for(uint16_t i=0; i<strip.numPixels(); i++) {
        strip.setPixelColor(i, 0);
    }
    for(byte j=0; j < filas[i][1]; j++) {
      strip.setPixelColor(filas[i][0]+j, c);
    }
    strip.show();
    delay(wait);
  }
}

// --------------------------------
// Adafruit strandtest example
// --------------------------------

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  if(WheelPos == 0) {
    return 0;
  } else if(WheelPos < 85) {
   return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if(WheelPos < 170) {
   WheelPos -= 85;
   return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
   WheelPos -= 170;
   return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, c);
      strip.show();
      delay(wait);
  }
}

void rainbow(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i+j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
      // Modificación para los destellos blancos 10% de probabilidad
      if ( random(0,100) > 90 ) {
          strip.setPixelColor(i,strip.Color(255,255,255));
      }
    }
    strip.show();
    delay(wait);
  }
}

