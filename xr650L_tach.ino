/*
* XR650L Tachometer and FastLed control 
* for the custom made led matrix (12x3)
*/


volatile byte rpmcount;
unsigned int rpm;
unsigned long timeold;


/*

*/

int dataPin = 9;  // 74hc595 pin 15
int latchPin = 10; // 74hc595 pin 12
int clockPin = 11; // 74hc595 pin 11

byte leds = 0;

int mode = MSBFIRST;


void rpm_fun()
{
  //Each rotation, this interrupt function is run twice, so take that into consideration for 
  //calculating RPM
  //Update count
  rpmcount++;
}
void updateShiftRegister()
{
   digitalWrite(latchPin, LOW);
   shiftOut(dataPin, clockPin, mode, leds);
   digitalWrite(latchPin, HIGH);
}

void setup()
{

  rpmcount = 0;
  rpm = 0;
  timeold = 0;

  pinMode(latchPin, OUTPUT);
  pinMode(dataPin, OUTPUT);  
  pinMode(clockPin, OUTPUT); 
  pinMode(13, OUTPUT);
  // testing leds on bootup...
  for (int j = 0; j < 3; j++) 
  {
    for (int x = 6; x >= 0; x--) 
    {
      bitSet(leds, x); 
      updateShiftRegister();
      delay(150);
      leds=0;
    }
  }
  leds=126; // 6 lowest bits ON, 6 leds on
  updateShiftRegister();
  delay(100);
  leds=0;
  updateShiftRegister();
  delay(100);
  leds=126; // 6 lowest bits ON, 6 leds on
  updateShiftRegister();
  delay(100);
  leds=0;
  updateShiftRegister();
  delay(100);
  leds=126; // 6 lowest bits ON, 6 leds on
  updateShiftRegister();
  delay(100);
  leds=0;
  updateShiftRegister();
  delay(1000);
  
  Serial.begin(115200);
  Serial.println("XRL650L tachometer");
  
  //Interrupt 0 is digital pin 2, so that is where the IR detector is connected
  //Triggers on FALLING (change from HIGH to LOW)
  attachInterrupt(digitalPinToInterrupt(3), rpm_fun, FALLING);
}


void loop()
{
  // update the tach only once per second. Must be done this way because
  // at very low RPMs the arduino runs this code several hunders of thousands
  // of times per second which means we would have division by zero here.
  if ((millis() - timeold) > 200) // actually 10x/s :-P
  {
    //Don't process interrupts during calculations
    detachInterrupt(digitalPinToInterrupt(3) );
  
    rpm = 60000/(millis() - timeold)*rpmcount;
    
    //Restart the interrupt processing
    attachInterrupt(digitalPinToInterrupt(3) , rpm_fun, FALLING);
    
    Serial.print("Current rpm:");
    Serial.println(rpm);
    Serial.print(" - ");
    Serial.println(rpmcount);
    
    timeold = millis();
    rpmcount = 0;
  }

  
  leds = 0;
  if ( rpm < 1000 ) 
    leds = 60;//
  
  if ( rpm > 1000 )    bitSet(leds, 6);
  
  if ( rpm > 1500 )    bitSet(leds, 5);
  
  if ( rpm > 2000 )    bitClear(leds,6);
  
  if ( rpm > 2500 )    bitSet(leds, 4);
  
  if ( rpm > 3000 )    bitClear(leds,5);
  
  if ( rpm > 3500 )    bitSet(leds, 3);
  
  if ( rpm > 4000 )    bitClear(leds,4);
  
  if ( rpm > 4500 )    bitSet(leds, 2);
  
  if ( rpm > 5000 )    bitClear(leds,3);
  
  if ( rpm > 5500 )    bitSet(leds, 1);
  
  if ( rpm > 6000 )    bitClear(leds,2);
  

  updateShiftRegister();
}
