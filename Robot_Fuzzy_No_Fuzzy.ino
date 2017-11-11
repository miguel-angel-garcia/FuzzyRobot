#include <Servo.h>

// Creamos los objetos para los servos
Servo motor;
Servo rueda;

// Estados del robot
#define DRIVE_FORWARD 0
#define TURN_SOFT_LEFT 1
#define TURN_LEFT 2
#define TURN_SOFT_RIGHT 3
#define TURN_RIGHT 4
#define NEGRO 0
#define GRIS 1
#define BLANCO 2
#define MAX_GIRO 40

// Constantes
const int serialPeriod = 250; // Sólo se imprime en la consola cada 1/4 segundo
const int loopPeriod = 20; // Período de 20 ms = frecuencia de 50 Hz
const int LDRRight = A0; // Pin del LDR derecho
const int LDRLeft = A1;  // Pin del LDR izquierdo
const int buzzer = 9;    // Pin del buzzer
const int pinVariador = 10;
const int pinServo = 12;

// Variables globales
int valLDRDer;
int valLDRIzq;
float medBlancoDer = 0;
float medBlancoIzq = 0;
unsigned long timeSerialDelay = 0;
unsigned long timeLoopDelay = 0;

int V;
int gradoGiro = 90;
long resMinRight = 0;
long resMaxRight = 0;
long resMinLeft = 0;
long resMaxLeft = 0;
long umbralRight;
long umbralLeft;

// Variables de estado
int state = DRIVE_FORWARD; // 0 = hacia adelante (por defecto), 1 = girar a la izquierda, 2 = girar a la derecha
int lastState = DRIVE_FORWARD;
int stateLDRDer = BLANCO;
int stateLDRIzq = BLANCO;

void blink_led_buzz(int times)
{
  int j;
  for(j = 0; j < times; j++)
  {
      digitalWrite(LED_BUILTIN, HIGH);
      tone(buzzer, 2000);  
      delay(500); // Espera medio segundo
      digitalWrite(LED_BUILTIN, LOW);
      noTone(buzzer);
      delay(500);
  }
}

void buzz_left()
{
  tone(buzzer, 3000);
}

void buzz_right()
{
  tone(buzzer, 2000);
}

void buzz_forward()
{
  noTone(buzzer);
}

void setup() 
{
  int i;
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);

  // Inicialización de los servos
  motor.attach(pinVariador);
  motor.write(0);
  rueda.attach(pinServo);
  rueda.write(90);
  
  // Calibración de los sensores LDR
  delay(1000); // Espera 1 segundo
  blink_led_buzz(2); // El led parpadea dos veces y el buzzer también
  delay(1000); // Espera 1 segundo
  // Primero el blanco
  for(i = 0; i < 1000; i++)
  {
    resMaxRight = resMaxRight + analogRead(LDRRight)*100;
    resMaxLeft = resMaxLeft + analogRead(LDRLeft)*100;
  }
  resMaxRight = resMaxRight/1000;
  resMaxLeft = resMaxLeft/1000;
  Serial.print("Blanco Derecho: ");
  Serial.println(resMaxRight);
  Serial.print("Blanco Izquierdo: ");
  Serial.println(resMaxLeft);

  blink_led_buzz(3); // El led parpadea tres veces y el buzzer también
  delay(1000); // Espera 1 segundo
  // Después para el negro
  for(i = 0; i < 1000; i++)
  {
    resMinRight = resMinRight + analogRead(LDRRight)*100;
    resMinLeft = resMinLeft + analogRead(LDRLeft)*100;
  }
  resMinRight = resMinRight/1000;
  resMinLeft = resMinLeft/1000;
  Serial.print("Negro Derecho: ");
  Serial.println(resMinRight);
  Serial.print("Negro Izquierdo: ");
  Serial.println(resMinLeft);

  // Asignamos el umbral
  umbralRight = (resMinRight + resMaxRight) / 2;
  umbralLeft = (resMinLeft + resMaxLeft) / 2;
  // Señal de finalización de la calibración
  delay(1000); // Espera 1 segundo
  blink_led_buzz(3); // El led parpadea tres veces y el buzzer también

}

void loop()
{
  // Salida de depuración
  debugOutput();
  // Control del sensor de ultrasonidos
  if(millis() - timeLoopDelay >= loopPeriod)
  {
    readLDRs();
    stateMachine();
    moveMachine();
    timeLoopDelay = millis();
  }

}

void debugOutput()
{
  if((millis() - timeSerialDelay) > serialPeriod)
  {
    Serial.print("Lectura Der: ");
    Serial.println(valLDRDer);    
    if(stateLDRDer < umbralRight)
      Serial.println("Derecha: NEGRO");
    else
      Serial.println("Derecha: BLANCO");

    Serial.print("Lectura Izq: ");
    Serial.println(valLDRIzq);
    if(valLDRIzq < umbralLeft)
      Serial.println("Izquierda: NEGRO");
    else
      Serial.println("Izquierda: BLANCO");
    timeSerialDelay = millis();
  }
}

long readLeftLDR()
{
  int lectLeft = analogRead(LDRLeft)*100;
  Serial.print("Lect Izq: ");
  Serial.println(lectLeft);
  // Normalizamos
  return(100*(lectLeft - resMinLeft)/(resMaxLeft - resMinLeft));
}

long readRightLDR()
{
  int lectRight = analogRead(LDRRight)*100;
  Serial.print("Lect Der: ");
  Serial.println(lectRight);
  // Normalizamos
  return(100*(lectRight - resMinRight)/(resMaxRight - resMinRight));
}


void readLDRs()
{
  // Lectura de los datos de los sensores LDR
  valLDRDer = readRightLDR();
  valLDRIzq = readLeftLDR();
  if(valLDRDer < 33)
    stateLDRDer = NEGRO;
  else
    if(valLDRDer < 66)
      stateLDRDer = GRIS;
    else
      stateLDRDer = BLANCO;
  if(valLDRIzq < 33)
    stateLDRIzq = NEGRO;
  else
    if(valLDRIzq < 66)
      stateLDRIzq = GRIS;
    else
      stateLDRIzq = BLANCO;
}

void stateMachine()
{
  lastState = state;
  if(stateLDRDer == BLANCO)
  {
    if(stateLDRIzq == BLANCO) // Estamos en una recta
      state = DRIVE_FORWARD; // Seguir adelante
    else // Si hay una curva
      if(stateLDRIzq == GRIS)
        state = TURN_SOFT_LEFT; // Si es gris, vamos a la izquierda
      else
        if(stateLDRIzq == NEGRO)
          state = TURN_LEFT; // Si es negro será más a la izquierda
  }
  if(stateLDRIzq == BLANCO)
  {
    if(stateLDRDer == BLANCO) // Estamos en una recta
      state = DRIVE_FORWARD; // Seguir adelante
    else // Si hay una curva
      if(stateLDRDer == GRIS)
        state = TURN_SOFT_RIGHT; // Si es gris, vamos a la derecha
      else
        if(stateLDRDer == NEGRO)
          state = TURN_RIGHT; // Si es negro será más a la derecha
  }
  Serial.print("Estado: ");
  Serial.println(state);
}

void moveMachine()
{
  if(state == DRIVE_FORWARD) // Sigue en una recta
  {
    // Seguir adelante
    buzz_forward();
    motor.write(55); // Adelante
    rueda.write(gradoGiro); // Adelante
    //gradoGiro = 90;
  }
  if(state == TURN_SOFT_LEFT) // Comenzamos una curva -> hay que girar a la izquierda
  {
    // Girar un poco a la izquierda
    buzz_left();
    motor.write(34);
    if(gradoGiro > MAX_GIRO) gradoGiro--;
    rueda.write(gradoGiro); // Hacemos que vaya girando poco a poco
  }
  if(state == TURN_LEFT) // Entramos en una curva -> hay que girar a la izquierda
  {
    // Girar a la izquierda
    buzz_left();
    motor.write(30);
    if(gradoGiro > MAX_GIRO) gradoGiro-=4;
    rueda.write(gradoGiro); // Hacemos que vaya girando poco a poco
  }
  if(state == TURN_SOFT_RIGHT) // Comenzamos en una curva -> hay que girar a la derecha
  {
    // Girar un poco a la derecha
    buzz_right();
    motor.write(34);
    if(gradoGiro < (MAX_GIRO + 90)) gradoGiro++;
    rueda.write(gradoGiro); // Hacemos que vaya girando poco a poco
  }
  if(state == TURN_RIGHT) // Estamos en una curva -> hay que girar a la derecha
  {
    // Girar a la derecha
    buzz_right();
    motor.write(30);
    if(gradoGiro < (MAX_GIRO + 90)) gradoGiro+=4;
    rueda.write(gradoGiro); // Hacemos que vaya girando poco a poco
  }
}

