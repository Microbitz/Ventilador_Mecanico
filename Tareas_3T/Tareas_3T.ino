#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x3F, 16, 2);
#include <EEPROM.h>

//Botones y menu...........................................INICIALIZACION
float Tiempo_actual_a = 0.0;
float Tiempo_actual_b = 0.0;
float Tiempo_actual_c = 0.0;
float Tiempo_a = 0.0;
float Tiempo_b = 0.0;
float Tiempo_c = 0.0;
float T_a = 0.1;   // Tiempo rutina 1 debe superar 0.05 Seg
float T_b = 0.25;   // Tiempo rutina 2 debe superar 0.05 Seg
float T_c = 1.0;   // Tiempo rutina 3 debe superar 0.05 Seg
float standby_time = 0.0;
bool standby = false;
bool Seg_c = false;
bool Seg_b = false;
bool Seg_a = false;

void setup() {

  pinMode(6, OUTPUT); //Configuramos el pin 6
  pinMode(7, OUTPUT); //Configuramos el pin 7
  pinMode(8, OUTPUT); //Configuramos el pin 8
  digitalWrite(6, LOW);
  digitalWrite(7, LOW);
  digitalWrite(8, LOW);

  Serial.begin(9600);

  lcd.init();
  lcd.home ();
  lcd.print("Menu");
  lcd.setBacklight(HIGH);

  // put your setup code here, to run once:
  // TIMER 2 for interrupt frequency 20 Hz:
  TCCR2A = 0;
  TCCR2B = 0;
  TCNT2  = 0;
  OCR2A = 12499;
  TCCR2B |= (1 << WGM21);
  TCCR2B |= (0 << CS22) | (1 << CS21) | (1 << CS20);
  TIMSK2 |= (1 << OCIE2A);
}

ISR(TIMER2_COMPA_vect) {
  Tiempo_a = Tiempo_a + 0.05;
  Tiempo_b = Tiempo_b + 0.05;
  Tiempo_c = Tiempo_c + 0.05;
  Tiempo_actual_a = Tiempo_a / 100.0;
  Tiempo_actual_b = Tiempo_b / 100.0;
  Tiempo_actual_c = Tiempo_c / 100.0;
  if (standby == true) {
    standby_time = standby_time + 0.05;
  }
  if (Tiempo_actual_a > T_a) {
    Seg_a = true;
  }
  if (Tiempo_actual_a > (T_a*2.0)) {
    Seg_a = false;
    Tiempo_a = 0.0;
  }
  if (Tiempo_actual_b > T_b) {
    Seg_b = true;
  }
  if (Tiempo_actual_b > (T_b*2.0)) {
    Seg_b = false;
    Tiempo_b = 0.0; 
  }
  if (Tiempo_actual_c > T_c) {
    Seg_c = true;
  }
  if (Tiempo_actual_c > (T_c*2.0)) {
    Seg_c = false;
    Tiempo_c = 0.0;
  }
}

void loop() {
  lcd.home ();
  lcd.print("Stand_BY");
  lcd.setCursor ( 0, 1 );
  lcd.print(Tiempo_actual_c, 2);
  //Serial.println(Tiempo_actual, 2);

  //Rutina A.............................................OPERATIVO
  if (Seg_a == true) {//No borrar
    digitalWrite(6, HIGH);
  } else {
    digitalWrite(6, LOW);
  }//No borrar
  //Rutina b.............................................OPERATIVO
  if (Seg_b == true) {//No borrar
    digitalWrite(7, HIGH);
  } else {
    digitalWrite(7, LOW);
  }
  //Rutina a c.............................................OPERATIVO
  if (Seg_c == true) {//No borrar
    digitalWrite(8, HIGH);
  } else {
    digitalWrite(8, LOW);
  }//No borrar
  Serial.print(Seg_a);
  Serial.print(",");
  Serial.print(Seg_b+2);
  Serial.print(",");
  Serial.println(Seg_c+4);
}
