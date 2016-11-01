/*
*  RGB Rainbow
*  Ejemplo de como hacer un led 'Rainbow' con un led RGB de 
*  ánodo común Se utilizan tres salidas con la instrucción 
*  'analogWrite' que produce una salida PWM para controlar 
*  el brillo de cada led.
*  Autor: Jose Daniel Herrera
*  Fecha: 22/08/2012
*  http://arduino-guay.blogspot.com.es
*/

// Retardo que determina la velocidad de la transicion entre 
// colores
#define RETARDO 10 
#define MAXBRILLO 50 

// Salidas para cada color (tiene que permitir PWM).
int rojo   = 9;
int verde = 11;
int azul  = 10;

/* 
* Funcion para variar el valor de PWM
* Para dir =1 aumenta para dir = -1 disminuye
*/
void fade(int pin, int dir)
{
  if ( dir != 1 ) 
  {
    for (int i = MAXBRILLO; i >= 0; i--)
    {
      analogWrite(pin, i);
      delay(RETARDO);
    }
  }
  else 
  {
    for (int i = 0; i <= MAXBRILLO; i++)
    {
      analogWrite(pin, i);
      delay(RETARDO);
    }
  }  
}

/*
* Ponemos los pines como salidas , y  
*/
void setup()
{
  pinMode(rojo, OUTPUT);
  pinMode(verde, OUTPUT);
  pinMode(azul, OUTPUT);

  //Partimos del amarillo
  analogWrite(rojo, MAXBRILLO);
  analogWrite(verde, MAXBRILLO);
  analogWrite(azul, 0);

}

/*
* Vamos cambiando los tres colores de modo que se produce 
* Un paso por todos los colores RGB y sus mezclas 2 a 2
*/
void loop()
{

  fade(verde,-1); // amarillo -> rojo
  fade(azul,1);   // rojo -> violeta
  fade(rojo,-1);  // violeta -> azul
  fade(verde,1);  // azul -> cian
  fade(azul,-1);  // cian -> verde
  fade(rojo,1);   // verde -> amarillo 
}
