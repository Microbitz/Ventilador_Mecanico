#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x3F, 16, 2);

#include <Servo.h>
Servo myservo;

//Serial_IN............................................INICIALIZACION
const int numberOfInData = 3;
String InData[numberOfInData];
String input = "";
int counter = 0;
int lastIndex = 0;

//SERVO..................................................INICIALIZACION
int Ang;
float Setpoint;
float Output_IN;
float Output_OUT;
float Output;
int Output_int;

//Tiempo..................................................INICIALIZACION
float Tiempo = 0;
float Tiempo_actual = 0;
float Tiempo_maximo = 0;
float Tiempo_INHlacion = 6.0;
float Tiempo_Exalacion = 6.0;
float Tiempo_Relax = 3.0;
float Tiempo_Total = 4.0;
bool INH = true;
bool EXA = false;
bool RLX = false;
int INH_RL = 11;
int EXA_RL = 12;
int RLX_RL = 13;
float Salida05_f;
int Salida05;


void setup()
{
  Serial.begin(9600);
  
  pinMode(INH_RL, OUTPUT);
  pinMode(EXA_RL, OUTPUT);
  pinMode(RLX_RL, OUTPUT);
  digitalWrite(INH_RL, HIGH);
  digitalWrite(EXA_RL, HIGH);
  digitalWrite(RLX_RL, HIGH);

  lcd.init();
  lcd.home ();
  lcd.print("Control_PID");
  lcd.setBacklight(HIGH);

  myservo.attach(9);

  // TIMER 2 for interrupt frequency 20 Hz:
  TCCR2A = 0;
  TCCR2B = 0;
  TCNT2  = 0;
  OCR2A = 12499;
  TCCR2B |= (1 << WGM21);
  TCCR2B |= (0 << CS22) | (1 << CS21) | (1 << CS20);
  TIMSK2 |= (1 << OCIE2A);
}

//Interrupcion 1 Segundo..................................OPERATIVO
ISR(TIMER2_COMPA_vect) {
  Tiempo = Tiempo + 0.05;
  Tiempo_actual = Tiempo / 100;
}


void loop()
{
  //Control_de_Motor........................................OPERATIVO
  Tiempo_Total = Tiempo_INHlacion + Tiempo_Relax + Tiempo_Exalacion + Tiempo_Relax;

  if (Tiempo_actual >= Tiempo_Total) {
    Tiempo = 0.0;
  }

  //Control_de_Motor........................................OPERATIVO

  if (Tiempo_actual < Tiempo_INHlacion) {
    Ang = 0;
    Setpoint = 0;
    Output_IN = Tiempo_actual * (120 / Tiempo_INHlacion);
    INH = true;
    EXA = false;
    RLX = false;
  }

  if (Tiempo_actual > Tiempo_INHlacion && Tiempo_actual < Tiempo_INHlacion + Tiempo_Relax) {
    INH = false;
    EXA = false;
    RLX = true;
  }

  if (Tiempo_actual > (Tiempo_INHlacion + Tiempo_Relax) && Tiempo_actual < (Tiempo_Total - Tiempo_Relax)) {
    Ang = 120;
    Setpoint = 0;
    Output_OUT = (Tiempo_actual - (Tiempo_INHlacion + Tiempo_Relax)) * (120 / Tiempo_INHlacion);
    INH = false;
    EXA = true;
    RLX = false;
  }

  if (Tiempo_actual > (Tiempo_Total - Tiempo_Relax)) {
    INH = false;
    EXA = false;
    RLX = true;
  }

  if (RLX == true) {
    lcd.setCursor ( 0, 1 );
    lcd.print("RLX");
  }

  if (INH == true) {
    Output = Output_IN;
    lcd.setCursor ( 0, 1 );
    lcd.print("INH");
  }
  if (EXA == true) {
    Output = 120 - Output_OUT;
    lcd.setCursor ( 0, 1 );
    lcd.print("EXA");
  }

  Output_int = int(Output);
  myservo.write(Output_int);

  Salida05 = map(Output_int,0,120,0,255);
  analogWrite(5, Salida05);
  Salida05_f = float(Salida05);
  Salida05_f = (Salida05_f*2)/100;
  
  //Relay.....................................................OPERATIVO
  if (INH == true) {
    digitalWrite(INH_RL, LOW);
    digitalWrite(EXA_RL, HIGH);
    digitalWrite(RLX_RL, HIGH);
  }
  if (EXA == true) {
    digitalWrite(INH_RL, HIGH);
    digitalWrite(EXA_RL, LOW);
    digitalWrite(RLX_RL, HIGH);
  }
  if (RLX == true) {
    digitalWrite(INH_RL, HIGH);
    digitalWrite(EXA_RL, HIGH);
    digitalWrite(RLX_RL, LOW);
  }
  //LCD.....................................................OPERATIVO

  lcd.home ();
  lcd.print("T:");
  lcd.print(Tiempo_actual, 2);
  lcd.print(" ");
  lcd.print(" Ang:");
  lcd.print(Ang);
  lcd.print("   ");

  //Transmision_de_datos..................................OPERATIVO

  Serial.print(Tiempo_actual);
  Serial.print(",");
  Serial.print(Output_int);
  Serial.print(",");
  Serial.println(Salida05_f);
  delay(10);

}
