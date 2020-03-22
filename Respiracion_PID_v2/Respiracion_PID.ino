#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x3F, 16, 2);
#include <EEPROM.h>
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
int Tiempo_Inhalacion_int;
int Tiempo_Exhalacion_int;
int Tiempo_Relax_int;
float Tiempo_Inhalacion;
float Tiempo_Exhalacion;
float Tiempo_Relax;
float Tiempo_Inhalacion_Prev;
float Tiempo_Exhalacion_Prev;
float Tiempo_Relax_Prev;
float Tiempo_Total;
bool INH = true;
bool EXA = false;
bool RLX = false;
int INH_RL = 11;
int EXA_RL = 12;
int RLX_RL = 13;
float Salida05_f;
int Salida05;
bool ONS = true;

//Botones y menu...........................................INICIALIZACION
const int buttonPin0 = 6;
const int buttonPin1 = 7;
const int buttonPin2 = 8;
int button0State = 6;
int button1State = 7;
int button2State = 8;
int menu = 0;
int acc = 0;
int segundos_menu = 0;
bool standby = false;
float standby_time;
bool Req_act = false;
bool Act_1 = false;
bool Act_2 = false;
bool Act_3 = false;
bool Hold = false;
void setup()
{
  Serial.begin(9600);

  pinMode(buttonPin0, INPUT);
  pinMode(buttonPin1, INPUT);
  pinMode(buttonPin2, INPUT);
  pinMode(INH_RL, OUTPUT);
  pinMode(EXA_RL, OUTPUT);
  pinMode(RLX_RL, OUTPUT);
  digitalWrite(INH_RL, HIGH);
  digitalWrite(EXA_RL, LOW);
  digitalWrite(RLX_RL, HIGH);

  lcd.init();
  lcd.home ();
  lcd.print("Control_PID");
  lcd.setBacklight(HIGH);

  myservo.attach(9);
  myservo.write(0);

  Tiempo_Inhalacion_int = EEPROM.read(0);
  Tiempo_Exhalacion_int = EEPROM.read(1);
  Tiempo_Relax_int = EEPROM.read(2);

  Tiempo_Inhalacion = float(Tiempo_Inhalacion_int);
  Tiempo_Exhalacion = float(Tiempo_Exhalacion_int);
  Tiempo_Relax = float(Tiempo_Relax_int);

  Tiempo_Inhalacion = Tiempo_Inhalacion / 10.0;
  Tiempo_Exhalacion = Tiempo_Exhalacion / 10.0;
  Tiempo_Relax = Tiempo_Relax_int / 10.0;

  Tiempo_Inhalacion_Prev = Tiempo_Inhalacion;
  Tiempo_Exhalacion_Prev = Tiempo_Exhalacion;
  Tiempo_Relax_Prev = Tiempo_Relax;

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
  if (standby == true) {
    standby_time = standby_time + 0.05;
  }
}


void loop()
{
  //Tiempo..................................................OPERATIVO
  if (ONS == true) {
    Tiempo_actual = Tiempo_Inhalacion;
    ONS = false;
  }

  Tiempo_Total = Tiempo_Inhalacion + Tiempo_Relax + Tiempo_Exhalacion + Tiempo_Relax;

  if (Tiempo_actual >= Tiempo_Total) {
    Tiempo = 0.0;
  }

  //Control_de_Motor........................................OPERATIVO

  if (Tiempo_actual < Tiempo_Inhalacion) {
    Ang = 0;
    Setpoint = 0;
    Output_IN = Tiempo_actual * (120 / Tiempo_Inhalacion);
    if (Output_IN < 5.0)delay(10);
    if (Output_IN > 115.0)delay(10);
    INH = true;
    EXA = false;
    RLX = false;
  }

  if (Tiempo_actual > Tiempo_Inhalacion && Tiempo_actual < Tiempo_Inhalacion + Tiempo_Relax) {
    INH = false;
    EXA = false;
    RLX = true;
    Output_int = 0;
  }

  if (Tiempo_actual > (Tiempo_Inhalacion + Tiempo_Relax) && Tiempo_actual < (Tiempo_Total - Tiempo_Relax)) {
    Ang = 120;
    Setpoint = 0;
    Output_OUT = (Tiempo_actual - (Tiempo_Inhalacion + Tiempo_Relax)) * (120 / Tiempo_Exhalacion);
    if (Output_IN < 5.0)delay(10);
    if (Output_OUT > 115.0)delay(10);
    INH = false;
    EXA = true;
    RLX = false;
  }

  if (Tiempo_actual > (Tiempo_Total - Tiempo_Relax)) {
    INH = false;
    EXA = false;
    RLX = true;
    Output_int = 120;
  }

  if (RLX == true) {
    if (menu == 0)lcd.setCursor ( 0, 1 );
    if (menu > 0)lcd.setCursor ( 13, 1 );
    lcd.print("RLX");
    if (menu == 0) {
      if (Req_act == true) {
        if (Tiempo_Exhalacion_Prev != Tiempo_Exhalacion)Tiempo_Exhalacion = Tiempo_Exhalacion_Prev;
        if (Tiempo_Inhalacion_Prev != Tiempo_Inhalacion)Tiempo_Inhalacion = Tiempo_Inhalacion_Prev;
        Act_1 = true;
      }
    }
  }

  if (INH == true) {
    Output = Output_IN;
    if (menu == 0)lcd.setCursor ( 0, 1 );
    if (menu > 0)lcd.setCursor ( 13, 1 );
    lcd.print("INH");
    if (menu == 0) {
      if (Req_act == true) {
        if (Tiempo_Relax_Prev != Tiempo_Relax)Tiempo_Relax = Tiempo_Relax_Prev;
        Act_2 = true;
      }
    }
  }
  if (EXA == true) {
    Output = 120 - Output_OUT;
    if (menu == 0)lcd.setCursor ( 0, 1 );
    if (menu > 0)lcd.setCursor ( 13, 1 );
    lcd.print("EXH");
    if (menu == 0) {
      if (Req_act == true) {
        if (Tiempo_Relax_Prev != Tiempo_Relax)Tiempo_Relax = Tiempo_Relax_Prev;
        Act_3 = true;
      }
    }
  }

  Output_int = int(Output);
  myservo.write(Output_int);

  Salida05 = map(Output_int, 0, 120, 82, 255);
  analogWrite(5, Salida05);
  Salida05_f = float(Salida05);
  Salida05_f = (Salida05_f * 2) / 100;

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

  //Botones.....................................................OPERATIVO
  button0State = digitalRead(buttonPin0);
  button1State = digitalRead(buttonPin1);
  button2State = digitalRead(buttonPin2);
  if (standby == false) {
    if (button0State == HIGH) {
      menu = menu + 1;
      lcd.clear ();
      standby = true;
    }

    if (button1State == HIGH) {
      if (menu == 1) {
        Tiempo_Inhalacion_Prev = Tiempo_Inhalacion_Prev + 0.5;
      }
      if (menu == 2) {
        Tiempo_Exhalacion_Prev = Tiempo_Exhalacion_Prev + 0.5;
      }
      if (menu == 3) {
        Tiempo_Relax_Prev = Tiempo_Relax_Prev + 0.5;
      }
      if (Tiempo_Inhalacion_Prev > 20.0) Tiempo_Inhalacion_Prev = 20.0;
      if (Tiempo_Exhalacion_Prev > 20.0) Tiempo_Exhalacion_Prev = 20.0;
      if (Tiempo_Relax_Prev > 10.0) Tiempo_Relax_Prev = 10.0;
      standby = true;
    }

    if (button2State == HIGH) {
      if (menu == 1) {
        Tiempo_Inhalacion_Prev = Tiempo_Inhalacion_Prev - 0.5;
      }
      if (menu == 2) {
        Tiempo_Exhalacion_Prev = Tiempo_Exhalacion_Prev - 0.5;
      }
      if (menu == 3) {
        Tiempo_Relax_Prev = Tiempo_Relax_Prev - 0.5;
      }
      if (Tiempo_Inhalacion_Prev < 0.0)Tiempo_Inhalacion_Prev = 2.0;
      if (Tiempo_Exhalacion_Prev < 0.0)Tiempo_Exhalacion_Prev = 2.0;
      if (Tiempo_Relax_Prev < 0.0)Tiempo_Relax_Prev = 2.0;
      standby = true;
    }

  }

  if (menu >= 4) {
    if (RLX == true) {
      menu = 0;
      Req_act = true;
      if (Tiempo_Inhalacion_Prev != Tiempo_Inhalacion) {
        Tiempo_Inhalacion_int = int(Tiempo_Inhalacion_Prev * 10);
        EEPROM.write(0, Tiempo_Inhalacion_int);
      }
      if (Tiempo_Exhalacion_Prev != Tiempo_Exhalacion) {
        Tiempo_Exhalacion_int = int(Tiempo_Exhalacion_Prev * 10);
        EEPROM.write(1, Tiempo_Exhalacion_int);
      }
      if (Tiempo_Relax_Prev != Tiempo_Relax) {
        Tiempo_Relax_int = int(Tiempo_Relax_Prev * 10);
        EEPROM.write(2, Tiempo_Relax_int);
      }
    }
  }

  //LCD.....................................................OPERATIVO
  //Menu_0
  if (menu == 0) {
    lcd.home ();
    lcd.print("T:");
    lcd.print(Tiempo_actual, 2);
    lcd.print(" ");
    lcd.print(" Ang:");
    lcd.print(Output_int);
    lcd.print("   ");
  }
  //Menu_1
  if (menu == 1) {
    lcd.home ();
    lcd.print("T_INH:");
    lcd.print(Tiempo_Inhalacion_Prev, 2);
    lcd.print(" ");
    lcd.setCursor ( 0, 1 );
    lcd.print("Act:");
    lcd.print(Tiempo_Inhalacion, 2);
    lcd.print(" ");
    //lcd.print(standby_time);
  }
  //Menu_2
  if (menu == 2) {
    lcd.home ();
    lcd.print("T_EXH:");
    lcd.print(Tiempo_Exhalacion_Prev, 2);
    lcd.print(" ");
    lcd.setCursor ( 0, 1 );
    lcd.print("Act:");
    lcd.print(Tiempo_Exhalacion, 2);
    lcd.print(" ");
    //lcd.print(standby_time);
  }
  //Menu_3
  if (menu == 3) {
    lcd.home ();
    lcd.print("T_RLX:");
    lcd.print(Tiempo_Relax_Prev, 2);
    lcd.print(" ");
    lcd.setCursor ( 0, 1 );
    lcd.print("Act:");
    lcd.print(Tiempo_Relax, 2);
    lcd.print(" ");
    //lcd.print(standby_time);
  }
  if (menu == 4) {
    lcd.home ();
    lcd.clear ();
    lcd.print("Guardando ");
    lcd.print(menu);
  }
  //Transmision_de_datos..................................OPERATIVO

  Serial.print(Tiempo_actual);
  Serial.print(",");
  Serial.print(Output_int);
  Serial.print(",");
  Serial.println(Salida05_f);
  /*Serial.print(",");
    Serial.print(Tiempo_Inhalacion);
    Serial.print(",");
    Serial.print(Tiempo_Exhalacion);
    Serial.print(",");
    Serial.println(Tiempo_Relax);*/
  delay(10);

  //Tiempo_menu..................................OPERATIVO
  acc = acc + 10;
  if (acc == 200) {
    segundos_menu = segundos_menu + 1;
    acc = 0;
    if (menu == 0) {
      lcd.setCursor ( 13, 1 );
      lcd.print(segundos_menu);
      lcd.print("   ");
    }
  }
  if (segundos_menu == 180) {
    lcd.clear ();
    segundos_menu = 0;
    menu = 0;
    standby_time = 0.0;
    standby = false;
  }
  if (standby_time >= 80.0) {
    standby_time = 0.0;
    standby = false;
  }
  if (Act_1 == true && Act_2 == true && Act_3 == true && Req_act == true) {
    Act_1 = false;
    Act_2 = false;
    Act_3 = false;
    Req_act = false;
  }
}
