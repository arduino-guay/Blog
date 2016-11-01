/*******************************************************************************************
 *
 *  Reloj Cromático Steampunk con Arduino 
 *      Autor: José Daniel Herrera Gomariz
 *      Proyecto completo en: http://arduino-guay.blogspot.com.es/2015/11/reloj-cromatico-steampunk-con-arduino.html
 *
 *   Reloj que utiliza colores para los dígitos basado en Arduino.
 * 	 Este reloj, utiliza un módulo RTC DS3231, de alta estabilidad y precisión, y un módulo medidor de humedad DTH22. 
 *   Incluye un sensor de luminosidad LDR para ajustar el brillo de los leds WS2812 encargados de los dígitos.
 ********************************************************************************************/

#include <Adafruit_NeoPixel.h>
#include <Wire.h>
#include "Sodaq_DS3231.h"
#include <DHT.h>
#include <IRremote.h>

// Códigos mando a adistancia
#define KON                0XFFE01F
#define KOFF               0xFF609F
#define KBRIGHTNESS_UP     0xFFA05F
#define KBRIGHTNESS_DOWN   0xFF20DF
#define KFLASH             0xFFF00F
#define KSTROBE            0xFFE817
#define KFADE              0xFFD827
#define KSMOOTH            0xFFC837

#define KRED               0xFF906F
#define KGREEN             0XFF10EF
#define KBLUE              0xFF50AF
#define KWHITE             0xFFD02F

#define KORANGE            0xFFB04F
#define KYELLOW_DARK       0xFFA857
#define KYELLOW_MEDIUM     0xFF9867
#define KYELLOW_LIGHT      0xFF8877

#define KGREEN_LIGHT       0XFF30CF
#define KGREEN_BLUE1       0XFF28D7
#define KGREEN_BLUE2       0XFF18E7
#define KGREEN_BLUE3       0XFF08F7

#define KBLUE_RED          0XFF708F
#define KPURPLE_DARK       0XFF6897
#define KPURPLE_LIGHT      0XFF58A7
#define KPINK              0XFF48B7

#define PIN_LED 8   // Pin control de leds 
#define PIN_DTH 2   // Sensor Humedad/temperatura
#define PIN_IR  7   // Receptor IR
#define PIN_SONIDO  3 // Modulo sonido
#define PIN_BRILLO  A0


#define LAMPARA_HORA  5 
#define LAMPARA_FECHA  6 
#define LAMPARA_CLIMA  9 
#define LAMPARA_AJUSTE  10 
#define VALOR_BRILLO  100
#define VALOR_NO_BRILLO  0

#define DHTTYPE DHT22 

#define MOOD_LED 0  // nº del led mood
#define TIEMPO_DATO 1000
#define VUELTAS_DATO 10

#define CAMBIO_HORA 66
#define CAMBIO_FECHA 55
#define CAMBIO_CLIMA 22
#define MAX_TIEMPO_DESPIERTO 30

#undef  DEBUG 


// 4-6 5-2 3-1
byte digitos[3][2] = { {1,3}, {2,5}, {6,4} };
byte digitosl[6] = {1,3,2,5,6,4};
byte digito =0;
byte numero[6];
byte brillo = 255;
byte ajuste = false;
byte fade = false;
byte idx_arcoIris =0;
byte tDespierto = 0; // segundos despierto

DHT dht(PIN_DTH, DHTTYPE);
IRrecv irrecv(PIN_IR);
decode_results results;

Adafruit_NeoPixel strip = Adafruit_NeoPixel(7, PIN_LED, NEO_GRB + NEO_KHZ800);

// Colores de los dígitos
uint32_t colores[10] = {
     strip.Color(10,10,10),    // 0 - Apagado
     strip.Color(80,30,20),    // 1 - Marrón
     strip.Color(255,0,0),     // 2 - Rojo
     strip.Color(175,50,0),    // 3 - Naranja
     strip.Color(255,215,0),   // 4 - Amarillo
     strip.Color(0,255,0),     // 5 - Verde
     strip.Color(0,0,255),     // 6 - Azul
     strip.Color(63,0,128),    // 7 - Violeta
     strip.Color(40,165,65),   // 8 - Cian
     strip.Color(255,255,255)  // 9 - Blanco
};

void _debug () {
} 

// Maximo brillo A0 >> 1024
// Mínimo brillo A0 >> 512
// Modifica el brillo en función de la luz ambiental
// Si hay un aumento de + de un 10% "despierta"
void controlaBrillo() {
    int valor = analogRead(PIN_BRILLO);
    byte nbrillo = map(valor, 100, 1024, 80, 255);
    strip.setBrightness(nbrillo);
    // Si el brillo aumenta + de 10%
    if ( nbrillo - brillo > 25 ) {
      despierta();
    }
    brillo = nbrillo;
    #ifdef DEBUG_ 
      Serial.print(valor,DEC);
      Serial.print(":");
      Serial.println(brillo,DEC);
    #endif  
}

void apagaLamparas () {
    analogWrite(LAMPARA_HORA,VALOR_NO_BRILLO);
    analogWrite(LAMPARA_FECHA,VALOR_NO_BRILLO);
    analogWrite(LAMPARA_CLIMA,VALOR_NO_BRILLO);
    analogWrite(LAMPARA_AJUSTE,VALOR_NO_BRILLO);
}

void enciendeLampara (byte lampara) {
    apagaLamparas();
    analogWrite(lampara,VALOR_BRILLO);
}

void ponDigitoDesp (byte digito, byte valor ) {
      int vel = 100;
      byte decenas  = valor / 10;
      byte unidades = valor % 10;
      for (byte d = 0; d < digito;  d++ ) {
          strip.setPixelColor(digitos[d][0], 0);
          strip.setPixelColor(digitos[d][1], 0);
      }
      strip.show(); 
      for (byte d = 0; d <= digito;  d++ ) {
          strip.setPixelColor(digitos[d][0], colores[decenas]);
          strip.show(); 
          delay(vel);
          strip.setPixelColor(digitos[d][0], 0);
          strip.setPixelColor(digitos[d][1], colores[decenas]);
          strip.show(); 
          delay(vel);
          if ( d < digito ) {
              strip.setPixelColor(digitos[d][1], 0);
          }
      }
      for (byte d = 0; d < digito;  d++ ) {
          strip.setPixelColor(digitos[d][0], colores[unidades]);
          strip.show(); 
          delay(vel);
          strip.setPixelColor(digitos[d][0], 0);
          strip.setPixelColor(digitos[d][1], colores[unidades]);
          strip.show(); 
          delay(vel);
          strip.setPixelColor(digitos[d][1], 0);
      }
      strip.setPixelColor(digitos[digito][0], colores[unidades]);
      strip.show(); 
}

void ponDigito (byte digito, byte valor ) {
      byte decenas  = valor / 10;
      byte unidades = valor % 10;
      strip.setPixelColor(digitos[digito][0], colores[unidades]);
      strip.setPixelColor(digitos[digito][1], colores[decenas]);
}

void ponDigitosDesp (byte valor1,  byte valor2, byte valor3) {
    ponDigitoDesp(2,valor1);
    ponDigitoDesp(1,valor2);
    ponDigitoDesp(0,valor3);
    strip.show(); 
    captura();
}

void ponDigitos (byte valor1,  byte valor2, byte valor3) {
    ponDigito(2,valor1);
    ponDigito(1,valor2);
    ponDigito(0,valor3);
    strip.show(); 
    captura();
}

void actualizaAjuste() {
   byte d1  = numero[0]*10 + numero[1];
   byte d2  = numero[2]*10 + numero[3];
   byte d3  = numero[4]*10 + numero[5];
   ponDigitos(d1,d2,d3);
}

void parpadea(byte digitosColor) {
    ponDigitos(0,0,0);
    strip.show(); 
    delay(100);
    ponDigitos(digitosColor,digitosColor,digitosColor);
    strip.show(); 
    delay(100);
    ponDigitos(0,0,0);
    strip.show(); 
    delay(100);
    ponDigitos(digitosColor,digitosColor,digitosColor);
    strip.show(); 
    delay(100);
}
  
void cambiaDigitos (byte valor1,  byte valor2, byte valor3) {
    ponDigitos(valor1,valor2,valor3);
}

void duerme () {
    #ifdef DEBUG
      Serial.println("A dormir ....");
    #endif  
    apagaLamparas();
    for(int i=1 ; i < 7; i++) {
      strip.setPixelColor(i,0);
    }
    strip.show();
}

void despierta () {
    #ifdef DEBUG
      Serial.println("despierta");
    #endif  
    tDespierto = 0;
    strip.setBrightness(brillo);
}
  
void setup() {
    pinMode(LAMPARA_HORA, OUTPUT);
    pinMode(LAMPARA_FECHA,OUTPUT);
    pinMode(LAMPARA_CLIMA,OUTPUT);
    pinMode(LAMPARA_AJUSTE,OUTPUT);

    Serial.begin(57600);
    strip.begin();
    strip.show(); 
    strip.setBrightness(brillo);
    Wire.begin();
    rtc.begin();
    dht.begin();
    irrecv.enableIRIn();
}

void ponHora() {
   if (digito < 5 ) {
       return;
   }
   DateTime now = rtc.now(); 
   byte dia = now.date();
   byte mes = now.month();
   byte ano = now.year(); 
   byte diaSemana = now.dayOfWeek();
   
   byte hora     = numero[0]*10 + numero[1];
   byte minutos  = numero[2]*10 + numero[3];
   byte segundos = numero[4]*10 + numero[5];
   DateTime dt(ano, mes, dia, hora, minutos, segundos, diaSemana);
   rtc.setDateTime(dt);
   ajuste = false;
}

void ponFecha() {
   if (digito < 5 ) {
       return;
   }
   DateTime now = rtc.now(); 
   byte hora     = now.hour();
   byte minutos  = now.minute();
   byte segundos = now.second(); 
   
   byte dia  = numero[0]*10 + numero[1];
   byte mes  = numero[2]*10 + numero[3];
   byte ano  = numero[4]*10 + numero[5];
   DateTime dt(ano, mes, dia, hora, minutos, segundos, 1);
   rtc.setDateTime(dt);
   ajuste = false;
}

void hora () {
    enciendeLampara(LAMPARA_HORA);
    strip.setPixelColor(MOOD_LED, strip.Color(0,0,255));
    parpadea(CAMBIO_HORA);
    DateTime now = rtc.now(); 
    ponDigitosDesp(now.hour(), now.minute(), now.second());
    for ( int i = 0; i < 20 ; i++ ) {
      now = rtc.now(); 
      ponDigitos(now.hour(), now.minute(), now.second());
      delay(TIEMPO_DATO);
      if ( ajuste || fade ) {
        return;
      }
    }
}

void fecha() {
    if ( ajuste || fade ) {
        return;
    }
    enciendeLampara(LAMPARA_FECHA);
    strip.setPixelColor(MOOD_LED, strip.Color(0,255,0));
    DateTime now = rtc.now(); 
    parpadea(CAMBIO_FECHA);
    ponDigitosDesp(now.date(), now.month(), now.year() - 2000);
    for ( int i = 0; i < VUELTAS_DATO ; i++ ) {
        now = rtc.now(); 
        ponDigitos(now.date(), now.month(), now.year() - 2000);
        delay(TIEMPO_DATO);
    }
}

void clima () {
    if ( ajuste || fade ) {
        return;
    }
    enciendeLampara(LAMPARA_CLIMA);
    strip.setPixelColor(MOOD_LED, strip.Color(255,0,0));
    float t = dht.readTemperature();
    float h = dht.readHumidity(); 
    float hic = dht.computeHeatIndex(t, h); 
    parpadea(CAMBIO_CLIMA);
    ponDigitosDesp((byte)round(t), (byte)round(h),(byte)round(hic));
    for ( int i = 0; i < VUELTAS_DATO ; i++ ) {
        delay(TIEMPO_DATO);
        captura(); 
    }
}

void loop() {
    if ( tDespierto < MAX_TIEMPO_DESPIERTO ) {
        for (byte p=0; p < 255; p++) {
            arcoIris();
        }
        if ( !fade)  {
            hora();
            fecha();
            clima();
        }
        tDespierto ++;
        #ifdef DEBUG
            Serial.print("tDespierto:");
            Serial.println(tDespierto,DEC);
        #endif  
        if ( tDespierto >= MAX_TIEMPO_DESPIERTO ) {
            duerme();
        }
    } else {
      // Si detectamos algún código IR despertamos
      if (irrecv.decode(&results)) {
          #ifdef DEBUG
              Serial.println("despertando...");
          #endif
          despierta();  
          irrecv.resume(); 
      }
      // Si hay un gran cambio de brillo despertamos
      controlaBrillo();
      // Miesntras duerme el led de mood hace piruetas
      idx_arcoIris = (idx_arcoIris+1) & 255; 
      strip.setPixelColor(0, CicloColor(idx_arcoIris & 255));
      strip.show();
      delay(20);
    }
}
 
void ajustarHoraFecha() {
    enciendeLampara(LAMPARA_AJUSTE);
    strip.setPixelColor(MOOD_LED, strip.Color(255,0,255));
    digito = 0;
    for (byte i=0; i < 6; i++) {
      numero[digito] = 0;
    } 
    actualizaAjuste();
    while (ajuste) {
        capturaAjuste();
    }
}

void captura () {
  controlaBrillo();
  if ( ajuste ) {
      return;
  }
  if (irrecv.decode(&results)) {
    if ( results.value != 0xFFFFFFFF) {
      //Serial.println(results.value);
      switch (results.value) {
           case KBRIGHTNESS_UP : 
               if ( brillo < 255 ) {
                 brillo +=25; 
                 strip.setBrightness(brillo);
                #ifdef DEBUG
                   Serial.print("Brillo:");
                   Serial.println(brillo,DEC);
                #endif  
               }
               break; 
           case KBRIGHTNESS_DOWN : 
               if ( brillo > 0 ) {
                 brillo -= 25; 
                 strip.setBrightness(brillo);
                #ifdef DEBUG
                   Serial.print("Brillo:");
                   Serial.println(brillo,DEC);
                #endif  
               }
               break; 
           case KFLASH :
               ajuste = true;
           break;
           case KFADE :
               fade = true;
           break;
           case KSMOOTH :
               fade = false;
           break;
      }
    }
    irrecv.resume(); // Receive the next value
  }
  if (ajuste) {
      ajustarHoraFecha();
  }
}

void ajustaDigito(byte valor) {
   #ifdef DEBUG
     Serial.print("ajustaDigito:");
     Serial.println(valor, DEC);
   #endif  
   numero[digito] = valor; 
   if ( digito < 5 ) {
     digito ++; 
     numero[digito] = 0; 
   }
   actualizaAjuste();
}

void capturaAjuste () {
  if (irrecv.decode(&results)) {
    if ( results.value != 0xFFFFFFFF) {
      switch (results.value) {
           case KBRIGHTNESS_UP : 
               if ( digito < 5 ) {
                 digito ++; 
                 numero[digito] = 0; 
               }
               break; 
           case KBRIGHTNESS_DOWN : 
               if ( digito > 0 ) {
                 digito --; 
                 numero[digito] = 0; 
               }
               break; 
           case KRED           : 
             ajustaDigito(2); 
             break;
           case KGREEN         : 
             ajustaDigito(5); 
             break;
           case KBLUE          : 
             ajustaDigito(6); 
             break;
           case KWHITE         : 
             ajustaDigito(9); 
             break;
           case KORANGE        : 
             ajustaDigito(3); 
             break;
           case KYELLOW_LIGHT  : 
             ajustaDigito(4); 
             break;
           case KGREEN_LIGHT   : 
             ajustaDigito(8); 
             break;
           case KPURPLE_DARK   : 
             ajustaDigito(7); 
             break;
           case KPINK          : 
             ajustaDigito(1); 
             break;
           case KOFF          : 
             ajustaDigito(0); 
             break;
           case KFLASH        :
               ponHora();
           break;
           case KSTROBE       :
               ponFecha();
           break;
           case KFADE :
           case KSMOOTH :
               ajuste = false;
           break;
      }
    }
    irrecv.resume(); // Receive the next value
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

// Crea un efecto arcoIris, los pixel tiene colores distintos
void arcoIris() {
    strip.setPixelColor(MOOD_LED, strip.Color(255,255,0));
    apagaLamparas();
    captura();
    idx_arcoIris = (idx_arcoIris+1) & 255; 
    for(uint16_t i=0; i < 6; i++) {
      strip.setPixelColor(digitosl[i], CicloColor((i*30+idx_arcoIris) & 255));
   }
   strip.show();
   delay(20);
}