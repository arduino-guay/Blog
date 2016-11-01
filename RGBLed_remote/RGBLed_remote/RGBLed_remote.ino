/*
*  Control remoto de LED RGB 
*  Ejemplo de como hacer variar el color y el brillo con un led RGB  
*  con un mando a distancia.
*  Se utiliza un receptor de infrarrojos del tipo TSOP1738 
*  Autor: Jose Daniel Herrera
*  Fecha: 28/08/2012
*  http://arduino-guay.blogspot.com.es
*/

#include <IRremote.h>

int RECV_PIN = 8;
int R_PIN = 9;
int G_PIN = 6;
int B_PIN = 10;

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

unsigned long rgb = 0;
byte r,g,b;

IRrecv irrecv(RECV_PIN);

decode_results results;


void setup()
{
  irrecv.enableIRIn(); // Inicializamos el receptor
  pinMode(R_PIN, OUTPUT);   
  pinMode(G_PIN, OUTPUT);   
  pinMode(B_PIN, OUTPUT);   
}


void variar (byte* color, char valor) {
    if (valor > 0) {
        if ( *color + valor <= 255) {
            *color += valor;
        } else {
            *color = 255;
        }
    } else { 
        if (*color + valor >= 0) {
            *color += valor;
        } else {
            *color = 0;
        }
  }
}

void RGB(unsigned long valor) {
   r = valor >> 16; 
   g = (valor >> 8) & 0xFF; 
   b = valor & 0xFF; 
}

void loop() {
  if (irrecv.decode(&results)) {
    if ( results.value != 0xFFFFFFFF) {
      switch (results.value) {
           case BRIGHTNESS_UP : 
               variar (&r, INCREMENTO);
               variar (&g, INCREMENTO);
               variar (&b, INCREMENTO);
               break; 
           case BRIGHTNESS_DOWN : 
               variar (&r, -INCREMENTO);
               variar (&g, -INCREMENTO);
               variar (&b, -INCREMENTO);
               break; 
           case OFF :
               r = g = b = 0;
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
      analogWrite(R_PIN,r);
      analogWrite(G_PIN,g);
      analogWrite(B_PIN,b);
    }
    irrecv.resume(); // Receive the next value
  }
}