#define THROTTLE_PIN 15
#define THROTTLE_LOW 150
#define THROTTLE_HIGH 710

#define HALL_1_PIN 6
#define HALL_2_PIN 7
#define HALL_3_PIN 8

#define AH_PIN 23
#define AL_PIN 20
#define BH_PIN 22
#define BL_PIN 9
#define CH_PIN 21
#define CL_PIN 10

#define LED_PIN 13

#define HALL_OVERSAMPLE 4

//uint8_t hallToMotor[8] = {255, 255, 255, 255, 255, 255, 255, 255};
uint8_t hallToMotor[8] = {255, 3, 5, 4, 1, 2, 0, 255}; 

void setup() {
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  digitalWriteFast(LED_PIN, HIGH);

  pinMode(AH_PIN, OUTPUT); //set all PWM pins as output
  pinMode(AL_PIN, OUTPUT);
  pinMode(BH_PIN, OUTPUT);
  pinMode(BL_PIN, OUTPUT);
  pinMode(CH_PIN, OUTPUT);
  pinMode(CL_PIN, OUTPUT);

  analogWriteFrequency(AH_PIN, 8000);
  analogWriteFrequency(BH_PIN, 8000);
  analogWriteFrequency(CH_PIN, 8000);
  analogWriteResolution(8);

  pinMode(HALL_1_PIN, INPUT); //set the hall pins as input
  pinMode(HALL_2_PIN, INPUT);
  pinMode(HALL_3_PIN, INPUT);

  pinMode(THROTTLE_PIN, INPUT);

  delay(1000);
  
  identifyHalls();
}

void loop() {
  uint8_t throttle = readThrottle();//only do this occasionally because its slow
  for(uint8_t i = 0; i < 200; i++)
  {  
    uint8_t hall = getHalls();
    uint8_t motorState = hallToMotor[hall];
    writePWM(motorState, throttle);
    delayMicroseconds(10);
  }
  /*static uint8_t i = 0;
  Serial.println(i);
  writePWM(i++, 20);
  delay(100);
  i = i % 6;*/
  /*identifyHalls();
  delay(3000);*/
  /*writePWM(0, 20);
  delay(100);
  //writePWM(4, 20);
  //delay(100);
  writePWM(5, 20);
  delay(100);*/
}

void identifyHalls()
{
  for(uint8_t i = 0; i < 6; i++)
  {
    uint8_t prevState = (i + 1) % 6;
    Serial.print("Going to ");
    Serial.println(i);
    for(uint8_t j = 0; j < 50; j++)
    {
      delay(10);
      writePWM(i, 20);
      delay(10);
      writePWM(prevState, 20);
    }
    hallToMotor[getHalls()] = (i + 2) % 6;
  }
  
  writePWM(0, 0);//turn phases off
  
  for(uint8_t i = 0; i < 8; i++)
  {
    Serial.print(hallToMotor[i]);
    Serial.print(", ");
  }
  Serial.println();

  //while(1);
}

void writePWM(uint8_t motorState, uint8_t dutyCycle)
{
  if(dutyCycle == 0)
    motorState = 255;//if zero throttle, turn all switches off

  switch(motorState){
    case 0://LOW A, HIGH B
      analogWrite(AH_PIN, 0);
      analogWrite(CH_PIN, 0);
      analogWrite(BH_PIN, dutyCycle);
      digitalWrite(BL_PIN, 0);
      digitalWrite(CL_PIN, 0);
      digitalWrite(AL_PIN, 1);
      break;
    case 1://LOW A, HIGH C
      analogWrite(AH_PIN, 0);
      analogWrite(BH_PIN, 0);
      analogWrite(CH_PIN, dutyCycle);
      digitalWrite(BL_PIN, 0);
      digitalWrite(CL_PIN, 0);
      digitalWrite(AL_PIN, 1);
      break;
    case 2://LOW B, HIGH C
      analogWrite(AH_PIN, 0);
      analogWrite(BH_PIN, 0);
      analogWrite(CH_PIN, dutyCycle);
      digitalWriteFast(AL_PIN, 0);
      digitalWriteFast(CL_PIN, 0);
      digitalWriteFast(BL_PIN, 1);
      break;
    case 3://LOW B, HIGH A
      analogWrite(BH_PIN, 0);
      analogWrite(CH_PIN, 0);
      analogWrite(AH_PIN, dutyCycle);
      digitalWriteFast(AL_PIN, 0);
      digitalWriteFast(CL_PIN, 0);
      digitalWriteFast(BL_PIN, 1);
      break;
    case 4://LOW C, HIGH A
      analogWrite(BH_PIN, 0);
      analogWrite(CH_PIN, 0);
      analogWrite(AH_PIN, dutyCycle);
      digitalWriteFast(AL_PIN, 0);
      digitalWriteFast(BL_PIN, 0);
      digitalWriteFast(CL_PIN, 1);
      break;
    case 5://LOW C, HIGH B
      analogWrite(AH_PIN, 0);
      analogWrite(CH_PIN, 0);
      analogWrite(BH_PIN, dutyCycle);
      digitalWriteFast(AL_PIN, 0);
      digitalWriteFast(BL_PIN, 0);
      digitalWriteFast(CL_PIN, 1);
      break;
    default://all off
      writePhases(0, 0, 0, 0, 0, 0);
  }
}

void writePhases(uint8_t ah, uint8_t bh, uint8_t ch, uint8_t al, uint8_t bl, uint8_t cl)
{
  analogWrite(AH_PIN, ah);
  analogWrite(BH_PIN, bh);
  analogWrite(CH_PIN, ch);
  digitalWriteFast(AL_PIN, al);
  digitalWriteFast(BL_PIN, bl);
  digitalWriteFast(CL_PIN, cl);
}

uint8_t getHalls()
{
  uint8_t hallCounts[] = {0, 0, 0};
  for(uint8_t i = 0; i < HALL_OVERSAMPLE; i++) //read all the hall pins repeatedly, tally results 
  {
    hallCounts[0] += digitalReadFast(HALL_1_PIN);
    hallCounts[1] += digitalReadFast(HALL_2_PIN);
    hallCounts[2] += digitalReadFast(HALL_3_PIN);
  }

  uint8_t hall = 0;
  
  if (hallCounts[0] >= HALL_OVERSAMPLE / 2) //if votes >= threshold, call that a 1
    hall |= (1<<0);
  if (hallCounts[1] >= HALL_OVERSAMPLE / 2)
    hall |= (1<<1);
  if (hallCounts[2] >= HALL_OVERSAMPLE / 2)
    hall |= (1<<2);

  return hall & 0x7;
}

uint8_t readThrottle()
{
  int32_t adc = analogRead(THROTTLE_PIN); //analogRead takes about 100us
  adc = (adc - THROTTLE_LOW) << 8;
  adc = adc / (THROTTLE_HIGH - THROTTLE_LOW);

  if (adc > 255)
    return 255;

  if (adc < 0)
    return 0;
  
  return adc;
}

