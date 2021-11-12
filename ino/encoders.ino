#define N_DECIMAL_POINTS_PRECISION (100) // n = 2. Dos decimales

const int PIN_CLOCK_1 = 7;
const int PIN_DATA_1  = 6;

const int PIN_CLOCK_2 = 5;
const int PIN_DATA_2  = 4;

unsigned int GrayToBinary(unsigned int num)
{
    unsigned int mask = num;
    while (mask) {           // Each Gray code bit is exclusive-ored with all more significant bits.
        mask >>= 1;
        num   ^= mask;
    }
    return num;
}

void setup() {
  Serial.begin(57600);

  pinMode (PIN_CLOCK_1, OUTPUT);
  pinMode (PIN_DATA_1,  INPUT);
  pinMode (PIN_CLOCK_2, OUTPUT);
  pinMode (PIN_DATA_2 , INPUT_PULLUP);
}

void loop() {

  int enco_dly  = 0.25;

  int pos_1     = 0;
  int pos_2     = 0;  
  
  int bits      = 13;
  
  float grado_1 = 0.0;
  float grado_2 = 0.0;
  unsigned int   grado_1_int;
  unsigned int   grado_1_dec;
  unsigned int   grado_2_int;
  unsigned int   grado_2_dec;
  
  char buffer[40];
  
  for (int i = 0; i < 6; i++) 
  {
    digitalWrite (PIN_CLOCK_1, HIGH);
    digitalWrite (PIN_CLOCK_2, HIGH);
    delay(enco_dly);
  }

  for (int i = 0; i < bits; i++) 
  {
    digitalWrite(PIN_CLOCK_1, LOW);
    digitalWrite(PIN_CLOCK_2, LOW);
    delay(enco_dly);

    digitalWrite(PIN_CLOCK_1, HIGH);
    digitalWrite(PIN_CLOCK_2, HIGH);
    delay(enco_dly/2);
  
    pos_1 = pos_1 | digitalRead(PIN_DATA_1);
    if (i < bits - 1) pos_1 = pos_1 << 1;

    pos_2 = pos_2 | digitalRead(PIN_DATA_2);
    if ( i < bits - 1) pos_2 = pos_2 << 1;
  }

  for (int i = 0; i < 6; i++) 
  {
    digitalWrite(PIN_CLOCK_1, HIGH);
    digitalWrite(PIN_CLOCK_2, HIGH);
    delay(enco_dly);
  }

  grado_1= (float(GrayToBinary(pos_1))*360.0)/(pow(2,13));
  grado_2= (float(GrayToBinary(pos_2))*360.0)/(pow(2,13));

  grado_1_int = (unsigned int) grado_1;
  grado_1_dec = ((unsigned int)(grado_1*N_DECIMAL_POINTS_PRECISION)%N_DECIMAL_POINTS_PRECISION);

  grado_2_int = (unsigned int) grado_2;
  grado_2_dec = ((unsigned int)(grado_2*N_DECIMAL_POINTS_PRECISION)%N_DECIMAL_POINTS_PRECISION);
  
  delay(20);

  sprintf(buffer, "AR_ANG,%04d,%03d.%02d,DEC_ANG,%04d,%03d.%02d\r\n", 
          pos_1, grado_1_int, grado_1_dec, pos_2, grado_2_int,grado_2_dec);

  Serial.write(buffer);
}

