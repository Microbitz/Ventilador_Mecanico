#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x3F, 16, 2);
#include <EEPROM.h>
#include <Servo.h>

Servo myservo_1;
Servo myservo_2;

//Tiempo..................................................INICIALIZACION
float Tiempo = 0.0;
float Tiempo_wave = 0.0;
float Tiempo_actual = 0.0;
float Tiempo_actual_rise = 0.0;
float Tiempo_actual_edge = 0.0;
float Tiempo_actual_inh = 6.0;
float Tiempo_actual_exh = 6.0;
float Tiempo_relax_i = 1.0;
float Tiempo_relax_e = 1.0;
float Tiempo_actual_inh_Prev;
float Tiempo_actual_exh_Prev;
float Tiempo_relax_i_Prev;
float Tiempo_relax_e_Prev;
float Tiempo_actual_inh_int;
float Tiempo_actual_exh_int;
float Tiempo_relax_i_int;
float Tiempo_relax_e_int;
float Desface = 0.0;
float Tiempo_maximo = 0.0;
float Tiempo_a = 6.0;

float freq = 0.0;
bool Primer_pico = false;

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
bool Act_4 = false;
bool Hold = false;
bool INH = true;
bool EXH = false;
bool RLX_i = false;
bool RLX_e = false;
bool modo = false;

//Servo....................................................INICIALIZACION
float pos_1 = 60.0;
float pos_2 = 30.0;
float Funcion;
int pos_1_int = 60;
int pos_2_int = 30;
int Serv_max = 100;
float Salida05_f;
int Salida05;

void setup() {
  Serial.begin(9600);

  pinMode(buttonPin0, INPUT);
  pinMode(11, OUTPUT);
  pinMode(12, OUTPUT);
  pinMode(13, OUTPUT);
  digitalWrite(11, HIGH);
  digitalWrite(12, HIGH);
  digitalWrite(13, LOW);

  lcd.init();
  lcd.home ();
  lcd.print("Control_PID");
  lcd.setBacklight(HIGH);

  myservo_1.attach(9);
  myservo_2.attach(10);
  myservo_1.write(0);
  myservo_2.write(180);

  Tiempo_actual_inh_int = EEPROM.read(0);
  Tiempo_actual_exh_int = EEPROM.read(1);
  Tiempo_relax_i_int = EEPROM.read(2);
  Tiempo_relax_e_int = EEPROM.read(3);

  Tiempo_actual_inh = float(Tiempo_actual_inh_int);
  Tiempo_actual_exh = float(Tiempo_actual_exh_int);
  Tiempo_relax_i = float(Tiempo_relax_i_int);
  Tiempo_relax_e = float(Tiempo_relax_e_int);

  Tiempo_actual_inh = Tiempo_actual_inh / 10.0;
  Tiempo_actual_exh = Tiempo_actual_exh / 10.0;
  Tiempo_relax_i = Tiempo_relax_i / 10.0;
  Tiempo_relax_e = Tiempo_relax_e / 10.0;

  Tiempo_actual_inh_Prev = Tiempo_actual_inh;
  Tiempo_actual_exh_Prev = Tiempo_actual_exh;
  Tiempo_relax_i_Prev = Tiempo_relax_i;
  Tiempo_relax_e_Prev = Tiempo_relax_e;

  // TIMER 2 for interrupt frequency 20 Hz:
  TCCR2A = 0;
  TCCR2B = 0;
  TCNT2  = 0;
  OCR2A = 12499;
  TCCR2B |= (1 << WGM21);
  TCCR2B |= (0 << CS22) | (1 << CS21) | (1 << CS20);
  TIMSK2 |= (1 << OCIE2A);

  Tiempo_actual_edge = Tiempo_actual_exh;


}

//Interrupcion 1 Segundo..................................OPERATIVO
ISR(TIMER2_COMPA_vect) {
  Tiempo = Tiempo + 0.05;
  if (INH == true) {
    Tiempo_actual_rise = Tiempo / 100;
  }
  if (RLX_i == true) {

  }
  if (EXH == true) {
    Tiempo_actual_edge = Tiempo_actual_exh - (Tiempo_actual - (Tiempo_actual_inh + Tiempo_relax_i));
  }
  if (RLX_e == true) {

  }
  if (standby == true) {
    standby_time = standby_time + 0.05;
  }
  Tiempo_actual = Tiempo / 100;
}

void loop() {

  Tiempo_maximo = Tiempo_actual_inh + Tiempo_relax_i + Tiempo_actual_exh + Tiempo_relax_e;
  Serial.print(Tiempo_actual);
  Serial.print(",");
  /*Serial.print((Tiempo_wave / 100.0)*Desface);
    Serial.print(",");
    Serial.print(Desface);
    Serial.print(",");
    Serial.print(Tiempo_actual_rise);
    Serial.print(",");*/
  Serial.print(pos_1);
  Serial.print(",");
  Serial.println(Funcion);

  //Boton.....................................................OPERATIVO
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
        Tiempo_actual_inh_Prev = Tiempo_actual_inh_Prev + 0.5;
      }
      if (menu == 2) {
        Tiempo_actual_exh_Prev = Tiempo_actual_exh_Prev + 0.5;
      }
      if (menu == 3) {
        Tiempo_relax_i_Prev = Tiempo_relax_i_Prev + 0.5;
      }
      if (menu == 4) {
        Tiempo_relax_e_Prev = Tiempo_relax_e_Prev + 0.5;
      }
      if (Tiempo_actual_inh_Prev > 20.0) Tiempo_actual_inh_Prev = 20.0;
      if (Tiempo_actual_exh_Prev > 20.0) Tiempo_actual_exh_Prev = 20.0;
      if (Tiempo_relax_i_Prev > 10.0) Tiempo_relax_i_Prev = 10.0;
      if (Tiempo_relax_e_Prev > 10.0) Tiempo_relax_e_Prev = 10.0;
      standby = true;
    }
    if (button2State == HIGH) {
      if (menu == 1) {
        Tiempo_actual_inh_Prev = Tiempo_actual_inh_Prev - 0.5;
      }
      if (menu == 2) {
        Tiempo_actual_exh_Prev = Tiempo_actual_exh_Prev - 0.5;
      }
      if (menu == 3) {
        Tiempo_relax_i_Prev = Tiempo_relax_i_Prev - 0.5;
      }
      if (menu == 4) {
        Tiempo_relax_e_Prev = Tiempo_relax_e_Prev - 0.5;
      }
      if (Tiempo_actual_inh_Prev > 20.0) Tiempo_actual_inh_Prev = 20.0;
      if (Tiempo_actual_exh_Prev > 20.0) Tiempo_actual_exh_Prev = 20.0;
      if (Tiempo_relax_i_Prev > 10.0) Tiempo_relax_i_Prev = 10.0;
      if (Tiempo_relax_e_Prev > 10.0) Tiempo_relax_e_Prev = 10.0;
      standby = true;
    }

    if (menu == 5) {
      menu = 0;
      Req_act = true;
      Primer_pico = false;
      if (Tiempo_actual_inh_Prev != Tiempo_actual_inh) {
        Tiempo_actual_inh_int = int(Tiempo_actual_inh_Prev * 10);
        EEPROM.write(0, Tiempo_actual_inh_int);
      }
      if (Tiempo_actual_exh_Prev != Tiempo_actual_exh) {
        Tiempo_actual_inh_int = int(Tiempo_actual_exh_Prev * 10);
        EEPROM.write(1, Tiempo_actual_inh_int);
      }
      if (Tiempo_relax_i_Prev != Tiempo_relax_i) {
        Tiempo_relax_i_int = int(Tiempo_relax_i_Prev * 10);
        EEPROM.write(2, Tiempo_relax_i_int);
      }
      if (Tiempo_relax_e_Prev != Tiempo_relax_e) {
        Tiempo_relax_e_int = int(Tiempo_relax_e_Prev * 10);
        EEPROM.write(3, Tiempo_relax_e_int);
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
    lcd.print(pos_1);
    lcd.print("   ");
    lcd.setCursor ( 0, 1 );
    if (INH == true)lcd.print("INH");
    if (EXH == true)lcd.print("EXH");
    if (RLX_i == true || RLX_e == true)lcd.print("RLX");
  }
  //Menu_1
  if (menu == 1) {
    lcd.home ();
    lcd.print("T_INH:");
    lcd.print(Tiempo_actual_inh_Prev, 2);
    lcd.print(" ");
    lcd.setCursor ( 0, 1 );
    lcd.print("Act:");
    lcd.print(Tiempo_actual_inh, 2);
    lcd.print(" ");
    lcd.setCursor ( 13, 1 );
    if (INH == true)lcd.print("INH");
    if (EXH == true)lcd.print("EXH");
    if (RLX_i == true || RLX_e == true)lcd.print("RLX");
  }
  //Menu_2
  if (menu == 2) {
    lcd.home ();
    lcd.print("T_EXH:");
    lcd.print(Tiempo_actual_exh_Prev, 2);
    lcd.print(" ");
    lcd.setCursor ( 0, 1 );
    lcd.print("Act:");
    lcd.print(Tiempo_actual_exh, 2);
    lcd.print(" ");
    lcd.setCursor ( 13, 1 );
    if (INH == true)lcd.print("INH");
    if (EXH == true)lcd.print("EXH");
    if (RLX_i == true || RLX_e == true)lcd.print("RLX");
  }
  //Menu_3
  if (menu == 3) {
    lcd.home ();
    lcd.print("T_RLX_i:");
    lcd.print(Tiempo_relax_i_Prev, 2);
    lcd.print(" ");
    lcd.setCursor ( 0, 1 );
    lcd.print("Act:");
    lcd.print(Tiempo_relax_i, 2);
    lcd.print(" ");
    lcd.setCursor ( 13, 1 );
    if (INH == true)lcd.print("INH");
    if (EXH == true)lcd.print("EXH");
    if (RLX_i == true || RLX_e == true)lcd.print("RLX");
  }
  //Menu_4
  if (menu == 4) {
    lcd.home ();
    lcd.print("T_RLX_e:");
    lcd.print(Tiempo_relax_e_Prev, 2);
    lcd.print(" ");
    lcd.setCursor ( 0, 1 );
    lcd.print("Act:");
    lcd.print(Tiempo_relax_i, 2);
    lcd.print(" ");
    lcd.setCursor ( 13, 1 );
    if (INH == true)lcd.print("INH");
    if (EXH == true)lcd.print("EXH");
    if (RLX_i == true || RLX_e == true)lcd.print("RLX");
  }
  if (menu == 5) {
    lcd.home ();
    lcd.clear ();
    lcd.print("Guardando ");
    lcd.print(menu);
  }
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
  if (Act_1 == true && Act_2 == true && Act_3 == true && Act_4 == true && Req_act == true) {
    Act_1 = false;
    Act_2 = false;
    Act_3 = false;
    Req_act = false;
  }

  //Tiempo.....................................................OPERATIVO

  if (INH == true) {
    digitalWrite(11, LOW);
    digitalWrite(12, HIGH);
    digitalWrite(13, HIGH);
    freq = (Tiempo_actual_rise * 45.0) / (Tiempo_actual_inh);
    Tiempo_wave = Tiempo_actual_rise;
    Tiempo_wave = (Tiempo_wave * 100) / Tiempo_actual_inh;
    if (Tiempo_actual > Tiempo_actual_inh) {
      INH = false;
      EXH = false;
      RLX_i = true;
      RLX_e = false;
      Tiempo_actual_rise = Tiempo_actual_inh;
    }
  }

  if (RLX_i == true) {
    digitalWrite(11, HIGH);
    digitalWrite(12, HIGH);
    digitalWrite(13, LOW);
    freq = 45.0;
    Tiempo_wave = 100.0;
    Funcion = 100.0;
    if (Tiempo_actual > (Tiempo_actual_inh + Tiempo_relax_i)) {
      INH = false;
      EXH = true;
      RLX_i = false;
      RLX_e = false;
      Tiempo_wave = Tiempo_actual_inh;
    }
  }

  if (EXH == true) {
    digitalWrite(11, HIGH);
    digitalWrite(12, LOW);
    digitalWrite(13, HIGH);
    freq = (Tiempo_actual_edge * 45.0) / (Tiempo_actual_exh);
    Tiempo_wave = Tiempo_actual_edge;
    Tiempo_wave = (Tiempo_wave * 100) / Tiempo_actual_exh;
    if (Tiempo_actual > (Tiempo_actual_inh + Tiempo_relax_i + Tiempo_actual_exh)) {
      INH = false;
      EXH = false;
      RLX_i = false;
      RLX_e = true;
      Tiempo_actual_edge = Tiempo_actual_exh;
    }
  }

  if (RLX_e == true) {
    digitalWrite(11, HIGH);
    digitalWrite(12, HIGH);
    if (Tiempo_relax_e > 0.0) {
      digitalWrite(13, LOW);
    } else {
      digitalWrite(13, HIGH);
    }
    freq = 0.0;
    Tiempo_wave = 0.0;
    Funcion = 0.0;
    if (Tiempo_actual > (Tiempo_actual_inh + Tiempo_relax_i + Tiempo_actual_exh + Tiempo_relax_e)) {
      INH = true;
      EXH = false;
      RLX_i = false;
      RLX_e = false;
      Tiempo = 0.0;
    }
    if (Req_act == true || menu == 0) {
      if (Tiempo_relax_i_Prev != Tiempo_relax_i) Tiempo_relax_i = Tiempo_relax_i_Prev;
      if (Tiempo_relax_e_Prev != Tiempo_relax_e) Tiempo_relax_e = Tiempo_relax_e_Prev;
      Act_1 = true;
    }
    if (Req_act == true || menu == 0) {
      if (Tiempo_actual_inh_Prev != Tiempo_actual_inh) Tiempo_actual_inh = Tiempo_actual_inh_Prev;
      if (Tiempo_actual_exh_Prev != Tiempo_actual_exh) Tiempo_actual_exh = Tiempo_actual_exh_Prev;
      Act_2 = true;
    }
    if (Req_act == true || menu == 0) {
      if (Tiempo_relax_i_Prev != Tiempo_relax_i) Tiempo_relax_i = Tiempo_relax_i_Prev;
      if (Tiempo_relax_e_Prev != Tiempo_relax_e) Tiempo_relax_e = Tiempo_relax_e_Prev;
      Act_3 = true;
    }
    if (Req_act == true || menu == 0) {
      if (Tiempo_actual_inh_Prev != Tiempo_actual_inh) Tiempo_actual_inh = Tiempo_actual_inh_Prev;
      if (Tiempo_actual_exh_Prev != Tiempo_actual_exh) Tiempo_actual_exh = Tiempo_actual_exh_Prev;
      Act_4 = true;
    }
  }

  freq = (freq * 45.0) / 100;

  if (RLX_i == false || RLX_e == false) {
    if (Tiempo_actual_inh > 2.9 || Tiempo_actual_exh > 2.9) {
      Funcion = 100 * sin((((Tiempo_wave * 100) / 180.0) / Tiempo_maximo) / 4);
      Funcion = Funcion + ((Tiempo_wave / 100.0) * Desface);
      if (Desface > 0.0 || Primer_pico == true) Funcion = (Funcion * Tiempo_wave) / (100.0 - Desface);
      if (Funcion > 100.0) Funcion = 100.0;
    }
  }


  if (Tiempo_actual_inh < 2.9 || Tiempo_actual_exh < 2.9) {
    Funcion = Tiempo_wave;
  }

  if (RLX_i == true) {
    if (Primer_pico == false) {
      if (Funcion < 100.0) {
        Desface = 100.0 - Funcion;
        Primer_pico = true;
      }
    }
  }
  //Salida.....................................................OPERATIVO
  pos_1 = map(Funcion, 0, 100, 0, Serv_max);
  pos_1_int = int(pos_1);
  myservo_1.write(pos_1_int);

  Salida05_f = pos_1;
  Salida05_f = map(Salida05_f, 0.0, 100.0, 0.0, 255.0);
  Salida05 = int(Salida05_f);
  analogWrite(5, Salida05);
}
