#include <Servo.h>
#include <FuzzyRule.h>
#include <FuzzyComposition.h>
#include <Fuzzy.h>
#include <FuzzyRuleConsequent.h>
#include <FuzzyOutput.h>
#include <FuzzyInput.h>
#include <FuzzyIO.h>
#include <FuzzySet.h>
#include <FuzzyRuleAntecedent.h>

// Creamos los objetos para los servos
Servo motor;
Servo rueda;

// Inicializamos los objetos de la lógica difusa
Fuzzy* fuzzy = new Fuzzy();
FuzzySet* blancoD = new FuzzySet(0, 0, 30, 50);
FuzzySet* grisD = new FuzzySet(30, 50, 50, 70);
FuzzySet* negroD = new FuzzySet(50, 70, 100, 100);
FuzzySet* blancoI = new FuzzySet(0, 0, 30, 50);
FuzzySet* grisI = new FuzzySet(30, 50, 50, 70);
FuzzySet* negroI = new FuzzySet(50, 70, 100, 100);

const int LDRRight = A0; // Pin del LDR derecho
const int LDRLeft = A1;  // Pin del LDR izquierdo
const int buzzer = 9;    // Pin del buzzer
const int pinVariador = 10;
const int pinServo = 12;
 
long resMinRight = 0;
long resMaxRight = 0;
long resMinLeft = 0;
long resMaxLeft = 0;

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

  // Señal de finalización de la calibración
  delay(1000); // Espera 1 segundo
  blink_led_buzz(3); // El led parpadea tres veces y el buzzer también

  // Inicialización de los conjuntos difusos de entrada
  FuzzyInput* sensorDer = new FuzzyInput(1);
  FuzzyInput* sensorIzq = new FuzzyInput(2);

  sensorDer->addFuzzySet(blancoD);
  sensorDer->addFuzzySet(grisD);
  sensorDer->addFuzzySet(negroD);
  fuzzy->addFuzzyInput(sensorDer);
  sensorIzq->addFuzzySet(blancoI);
  sensorIzq->addFuzzySet(grisI);
  sensorIzq->addFuzzySet(negroI);
  fuzzy->addFuzzyInput(sensorIzq);

  // Conjuntos de salida
  FuzzyOutput* velocidad = new FuzzyOutput(1);
 
  FuzzySet* parado = new FuzzySet(0, 0, 0, 0);
  velocidad->addFuzzySet(parado);
  FuzzySet* lento = new FuzzySet(1, 35, 35, 45);
  velocidad->addFuzzySet(lento);
  FuzzySet* normal = new FuzzySet(35, 40, 40, 45);
  velocidad->addFuzzySet(normal);
  FuzzySet* rapido = new FuzzySet(45, 50, 60, 60);
  velocidad->addFuzzySet(rapido);
  fuzzy->addFuzzyOutput(velocidad);

  FuzzyOutput* direccion = new FuzzyOutput(2);
  FuzzySet* izq = new FuzzySet(0, 0, 80, 90);
  direccion->addFuzzySet(izq);
  FuzzySet* centro = new FuzzySet(80, 90, 90, 100);
  direccion->addFuzzySet(centro);
  FuzzySet* der = new FuzzySet(90, 100, 180, 180);
  direccion->addFuzzySet(der);
  fuzzy->addFuzzyOutput(direccion);

  // Regla difusa 1
  FuzzyRuleAntecedent* sensorIzqBlancoYsensorDerBlanco = new FuzzyRuleAntecedent();
  sensorIzqBlancoYsensorDerBlanco->joinWithAND(blancoI, blancoD);

  FuzzyRuleConsequent* velRapidoYdireccionCentro = new FuzzyRuleConsequent();
  velRapidoYdireccionCentro->addOutput(rapido);
  velRapidoYdireccionCentro->addOutput(centro);

  FuzzyRule* fuzzyRule1 = new FuzzyRule(1, sensorIzqBlancoYsensorDerBlanco, velRapidoYdireccionCentro);
  fuzzy->addFuzzyRule(fuzzyRule1);

  // Regla difusa 2
  FuzzyRuleAntecedent* sensorIzqGrisYsensorDerBlanco = new FuzzyRuleAntecedent();
  sensorIzqGrisYsensorDerBlanco->joinWithAND(grisI, blancoD);

  FuzzyRuleConsequent* velNormalYdireccionIzq = new FuzzyRuleConsequent();
  velNormalYdireccionIzq->addOutput(normal);
  velNormalYdireccionIzq->addOutput(izq);
  
  FuzzyRule* fuzzyRule2 = new FuzzyRule(2, sensorIzqGrisYsensorDerBlanco, velNormalYdireccionIzq);
  fuzzy->addFuzzyRule(fuzzyRule2);

  // Regla difusa 3
  FuzzyRuleAntecedent* sensorIzqBlancoYsensorDerGris = new FuzzyRuleAntecedent();
  sensorIzqBlancoYsensorDerGris->joinWithAND(blancoI, grisD);

  FuzzyRuleConsequent* velNormalYdireccionDer = new FuzzyRuleConsequent();
  velNormalYdireccionDer->addOutput(normal);
  velNormalYdireccionDer->addOutput(der);

  FuzzyRule* fuzzyRule3 = new FuzzyRule(3, sensorIzqBlancoYsensorDerGris, velNormalYdireccionDer);
  fuzzy->addFuzzyRule(fuzzyRule3);

  // Regla difusa 4
  FuzzyRuleAntecedent* sensorDerBlancoOgris = new FuzzyRuleAntecedent();
  sensorDerBlancoOgris->joinWithOR(blancoD, grisD);
  FuzzyRuleAntecedent* sensorIzqNegroYsensorDerBlancoOgris = new FuzzyRuleAntecedent();
  sensorIzqNegroYsensorDerBlancoOgris->joinWithAND(negroI, sensorDerBlancoOgris);

  FuzzyRuleConsequent* velLentoYdireccionIzq = new FuzzyRuleConsequent();
  velLentoYdireccionIzq->addOutput(lento);
  velLentoYdireccionIzq->addOutput(izq);
  
  FuzzyRule* fuzzyRule4 = new FuzzyRule(4, sensorIzqNegroYsensorDerBlancoOgris, velLentoYdireccionIzq);
  fuzzy->addFuzzyRule(fuzzyRule4);

  // Regla difusa 5
  FuzzyRuleAntecedent* sensorIzqBlancoOgris = new FuzzyRuleAntecedent();
  sensorIzqBlancoOgris->joinWithOR(blancoI, grisI);
  FuzzyRuleAntecedent* sensorIzqBlancoOgrisYsensorDerNegro = new FuzzyRuleAntecedent();
  sensorIzqBlancoOgrisYsensorDerNegro->joinWithAND(sensorIzqBlancoOgris, negroD);

  FuzzyRuleConsequent* velLentoYdireccionDer = new FuzzyRuleConsequent();
  velLentoYdireccionDer->addOutput(lento);
  velLentoYdireccionDer->addOutput(der);

  FuzzyRule* fuzzyRule5 = new FuzzyRule(5, sensorIzqBlancoOgrisYsensorDerNegro, velLentoYdireccionDer);
  fuzzy->addFuzzyRule(fuzzyRule5);

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

void loop()
{
  // Lectura de los LDRs
  int valLDRDer = readRightLDR();
  int valLDRIzq = readLeftLDR();

  Serial.print("Sensores: (Izq, Der): ");
  Serial.print(valLDRIzq);
  Serial.print(" ,");
  Serial.println(valLDRDer);

  // Fuzzificación
  fuzzy->setInput(1, valLDRDer);
  fuzzy->setInput(2, valLDRIzq);

  fuzzy->fuzzify();

  Serial.print("Blanco Izq: ");
  Serial.println(blancoI->getPertinence());
  Serial.print("Gris Izq: ");
  Serial.println(grisI->getPertinence());
  Serial.print("Negro Izq: ");
  Serial.println(negroI->getPertinence());
  Serial.print("Blanco Der: ");
  Serial.println(blancoD->getPertinence());
  Serial.print("Gris Der: ");
  Serial.println(grisD->getPertinence());
  Serial.print("Negro Der: ");
  Serial.println(negroD->getPertinence());

  // Desfuzzificación
  int setVel = fuzzy->defuzzify(1);
  int setDir = fuzzy->defuzzify(2);

  delay(3000);
  // Aplicamos la salida
  motor.write(setVel);
  rueda.write(setDir);
}
