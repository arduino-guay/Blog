/*
*  Organo de luces psicodelicas 
*  Organo de luces de tres canales , utilizando la FFT   
*  Autor: Jose Daniel Herrera
*  Fecha: 05/09/2012
*  http://arduino-guay.blogspot.com.es
*/

#include "fix_fft.h"

#define MUESTRAS 128           // Numero de muestras para el cálculo de la FFT
#define LOGM 7                 // Logaritmo en base 2 del número de muestras

#define BAJOS_MEDIOS 7         // Nº de banda para el corte entre Bajos y Medios
#define MEDIOS_AGUDOS 35       // Nº de banda para el corte entre Medios y Agudos

#define BPIN  9                // Pin de salida Bajos
#define MPIN  10               // Pin de salida Medios 
#define APIN  11               // Pin de salida Agudos

#define MAX_PASADAS 10         // Nº de pasadas para el cálculo de los límites

char data[MUESTRAS];           // Array con los valores muestreados (parte real)
char im[MUESTRAS];             // Array con los valores muestreados (parte imaginaria)

unsigned char salida[MUESTRAS/2];  // Valores obtenidos de la FFT (espectro de 64 bandas)
unsigned char bajos,medios,agudos; // Valores calculados para cada canal 

byte  pasada,                            // nº de pasada para el cáculo de los límites
      acumBajos,acumMedios,acumAgudos,   // acumuladores de veces que se supera el límite
      limBajos,limMedios,limAgudos;      // límites calculados para cada canal


/*
* Funcion que aplica una ventana de Hann a los datos muestreados para reducir el 
* efecto de las discontinuidades en los extremos
*/
void aplicaVentana (char *vData) {
    double muestrasMenosUno = (double(MUESTRAS) - 1.0);
 // Como la ventana es simétrica , se calcula para la mitad y se aplica el factor por los dos extremos
    for (uint8_t i = 0; i < MUESTRAS/2 ; i++) {
        double indiceMenosUno = double(i);
        double ratio = (indiceMenosUno / muestrasMenosUno);
        double factorPeso = 0.5 * (1.0 - cos(6.28 * ratio));
 vData[i] *= factorPeso;
 vData[MUESTRAS - (i + 1)] *= factorPeso;
    }
}

void setup() {                   
    // Configuramos el prescaler a 32 -> 16Mhz/32 = 500 KHz
    // como cada conversion son 13 ciclos 500/13 ~ 38.4KHz
    // Es decir podemos medir en teoria hasta unos 19KHz, 
    // que para este proyecto sobra.
    bitWrite(ADCSRA,ADPS2,1);
    bitWrite(ADCSRA,ADPS1,0);
    bitWrite(ADCSRA,ADPS0,1); 

    // Como la señal es muy baja,utilizamos la referencia interna 
    // de 1.1 V en vez de la de defecto de 5 V.
    analogReference(INTERNAL);   
    
    // Salidas para los canales de Bajos,Medios y Agudos
    pinMode(BPIN,OUTPUT);
    pinMode(MPIN,OUTPUT);
    pinMode(APIN,OUTPUT);
    
    // Variables para el cálculo de los límites
    pasada = 0;
    acumBajos = acumMedios = acumAgudos = 0;
    limBajos = limMedios = limAgudos = 50;
}

void loop() {

    // Realizamos el muestreo 
    for( int i=0; i < MUESTRAS; i++) {
       data[i] = analogRead(0)/4 -128;  //Convertimos de 0..1024 a -128..127                                 
       im[i] = 0;                       // parte imaginaria = 0                          
    }
    
    // Aplicamos la ventana de Hann
    aplicaVentana (data);
 
    // Calculamos la FFT 
    fix_fft(data,im,LOGM,0);
    
    // Sólo nos interesan los valores absolutos, no las fases, asi que 
    // calculamos el módulos de los vectores re*re + im*im.
    // Dado que los valores son pequeños utilizamos el cuadrado 
    for (int i=0; i < MUESTRAS/2; i++){
       salida[i] = data[i] * data[i] + im[i] * im[i];
    }
   
    // Ahora repartimos las bandas entre las 3 salidas
    // En vez de sacar la media, utilizo sólo el valor máximo de
    // una banda 
    bajos = 0;
    for (int i=2; i < BAJOS_MEDIOS; i++){
      bajos += salida[i];
    }
    bajos = bajos/2;
    
    medios = 0;
    for (int i=BAJOS_MEDIOS ; i < MEDIOS_AGUDOS; i++){
      medios += salida[i];
    }
    medios = medios/2;
    
    agudos = 0;
    for (int i=MEDIOS_AGUDOS; i < MUESTRAS/2; i++){
      agudos += salida[i];
    }
    agudos = agudos/2;
    
   // Calculamos si el canal correspondiente 
   // supera le límite para encenderlo 
   int siBajos  =  bajos  > limBajos;
   int siMedios =  medios > limMedios;
   int siAgudos =  agudos > limAgudos;
   
   digitalWrite(BPIN,siBajos ? HIGH : LOW);
   digitalWrite(MPIN,siMedios? HIGH : LOW);
   digitalWrite(APIN,siAgudos? HIGH : LOW);
   
   // Utilizamos las veces que se supera para
   // recalcular el límite y evitar que con los
   // cambios de volumen los canales se saturen
   // o no funcionen.
   acumBajos  += siBajos;
   acumMedios += siMedios;
   acumAgudos += siAgudos;
   
   if ( ++pasada > MAX_PASADAS ) {
      pasada = 0;
      limBajos  = 20 + acumBajos*5;
      limMedios = 20 + acumMedios*5;
      limAgudos = 20 + acumAgudos*5;
      acumBajos  = 0;
      acumMedios = 0;
      acumAgudos = 0;
   }
 }