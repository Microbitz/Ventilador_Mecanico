#include "Filter.h"
int InputValue;
float SmoothInp;
float SmoothInp_val;


//Auto-reset...........................................INICIALIZACION
int resetcont = 0;
long T_on;

//Computo............................................INICIALIZACION
ExponentialFilter<float> FilteredInput(1, 0);
float sample1;
float sample2;
float sample3;
float sample4;
float sample5;
bool Rise;
bool Fall;
bool Hold;

//Inputs............................................INICIALIZACION
int INHState = 0;
int EXHState = 0;
int RLXState = 0;
bool INH;
bool EXH;
bool RLX;
int INHInp = 10;
int EXHInp = 11;
int RLXInp = 12;

//Stepper..............................................INICIALIZACION
int Pot = 0;
float Tiempo_f = 0.0;
int Tiempo_int = 0;
int Encendido = 0;
#define dirPin 8
#define stepPin 9
#define stepsPerRevolution 800

void setup() {
  Serial.begin(9600);
  pinMode(dirPin, OUTPUT);
  pinMode(stepPin, OUTPUT);
  pinMode(INHInp, INPUT);
  pinMode(EXHInp, INPUT);
  pinMode(RLXInp, INPUT);

  // TIMER 1 for interrupt frequency 1 Hz:
  cli(); // stop interrupts
  TCCR1A = 0; // set entire TCCR1A register to 0
  TCCR1B = 0; // same for TCCR1B
  TCNT1  = 0; // initialize counter value to 0
  // set compare match register for 50 Hz increments
  OCR1A = 52082; // = 16000000 / (8 * 50) - 1 (must be <65536)
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS12, CS11 and CS10 bits for 8 prescaler
  TCCR1B |= (0 << CS12) | (1 << CS11) | (0 << CS10);
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);
  sei(); // allow interrupts
}

void(* resetFunc) (void) = 0;
//Interrupcion 1 Segundo..................................OPERATIVO
ISR(TIMER1_COMPA_vect) {
  T_on = T_on + 1;
  if (T_on == 1) {
    sample1 = SmoothInp_val;
  }
  if (T_on == 2) {
    sample2 = SmoothInp_val;
  }
  if (T_on == 3) {
    sample3 = SmoothInp_val;
  }
  if (T_on == 4) {
    sample4 = SmoothInp_val;
  }
  if (T_on == 5) {
    sample5 = SmoothInp_val;
    T_on = 0;
  }
  if (sample5 > sample4 && sample4 > sample3 && sample3 > sample2 && sample2 > sample1) {
    Rise = true;
    Fall = false;
    INH = true;
    EXH = false;
  }
  if (sample5 < sample4 && sample4 < sample3 && sample3 < sample2 && sample2 < sample1) {
    Rise = false;
    Fall = true;
    INH = false;
    EXH = true;
  }
}

void pulseCounter()
{

}

void loop() {
  //Computo de señales......................................OPERATIVO

  InputValue = analogRead(A0);
  InputValue = map(InputValue, 0, 1023, 0, 100);
  SmoothInp = float(InputValue);
  FilteredInput.Filter(SmoothInp);
  SmoothInp_val = FilteredInput.Current();
  Tiempo_f = (SmoothInp_val * 2000.0) / 100;
  Tiempo_f = map(Tiempo_f, 0.0, 2000.0, 0, 2000.0);
  Tiempo_int = int(Tiempo_f);

  //Input...................................................OPERATIVO

  INHState = digitalRead(INHInp);
  EXHState = digitalRead(EXHInp);
  RLXState = digitalRead(RLXInp);

  //Motor...................................................OPERATIVO
  if (INHState == false) {
    digitalWrite(dirPin, HIGH);
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(Tiempo_int);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(Tiempo_int);
  }

  if (EXHState == false) {
    digitalWrite(dirPin, LOW);
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(Tiempo_int);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(Tiempo_int);
  }

  //Transmision...............................................OPERATIVO

  Serial.println(SmoothInp_val);
  delay(1);

}
