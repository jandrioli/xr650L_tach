/*
* XR650L Tachometer 
* 
* This sketch uses digital pins 9, 10, 11 and 3 (interrupt); and 13 (led)
* There is a 74hc595 shifter to convert serial to parallel outputs, which controls 6 LEDs that show the RPM range.
* The RPM is calculated by using interrupt 1 (digital pin 3), which is conencted to a op-amp (LM358) which in turn
* is connected to the motorcycle's magnetic pick up (pulse-generator).
* The op-amp compares the pulse to a reference voltage which I defined with a trim-pot. Anything above 1.5V seen by
* the op-amp should trigger my interrupt here on arduino. 
* LM358
* http://www.ti.com/lit/ds/symlink/lm158-n.pdf
*/
#include <TimerOne.h>

volatile byte rpmcount = 1;
volatile bool doflash = false;
float rpm = 1;
unsigned long timeold, time2;
byte leds = 0;
#define dataPin    9  // 74hc595 pin 15
#define latchPin  10  // 74hc595 pin 12
#define clockPin  11  // 74hc595 pin 11
#define ledPin    13  // just a red led
#define mode      MSBFIRST


void rpm_fun()
{
  //Update count
  rpmcount++;
  if ( rpm < 900 && rpm > 60 ) 
  {
    doflash = true;
  }
}
void flashOn()
{
  digitalWrite(ledPin, HIGH);  
  leds = 126;
  updateShiftRegister();
  Timer1.start();
}
void flashOff()
{
  digitalWrite(ledPin, LOW);  
  leds = 0;
  updateShiftRegister();
  Timer1.stop();
  doflash= false;
}

void updateShiftRegister()
{
   digitalWrite(latchPin, LOW);
   shiftOut(dataPin, clockPin, mode, leds);
   digitalWrite(latchPin, HIGH);
}

int ledState = LOW;
void doBlink()
{
  ledState = !ledState;
  digitalWrite(ledPin, ledState);
}

void setup()
{
  rpmcount = 0;
  rpm = 0;
  timeold = 0;

  pinMode(latchPin, OUTPUT);
  pinMode(dataPin, OUTPUT);  
  pinMode(clockPin, OUTPUT); 
  pinMode(ledPin, OUTPUT);

  // first off let's check that my Timer can flash the red led on the board
  Timer1.initialize(50000); //every 0.05 seconds
  Timer1.attachInterrupt(doBlink); // blinkLED to run 
  

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
  for (int j = 0; j < 3; j++)
  {
    leds=126; // 6 lowest bits ON, 6 leds on
    updateShiftRegister();
    delay(100);
    leds=0;
    updateShiftRegister();
    delay(100);
  }
  
  Serial.begin(115200);
  Serial.println("XRL650L tachometer");
  
  //Interrupt 0 is digital pin 2, so that is where the IR detector is connected
  //Triggers on FALLING (change from HIGH to LOW)
  attachInterrupt(digitalPinToInterrupt(3), rpm_fun, FALLING);
  // first off let's check that my Timer can flash the red led on the board
  Timer1.initialize(50000); //every 0.05 seconds
  Timer1.detachInterrupt(); // blinkLED to run 
  Timer1.attachInterrupt(flashOff); // turn off all leds
  Timer1.stop();
  flashOff();
}


void loop()
{
  // update the tach only once per second. Must be done this way because
  // at very low RPMs the arduino runs this code several hunders of thousands
  // of times per second which means we would have division by zero here.
  int interval = millis() - timeold;
  if (interval >= 200) // actually 2x/s :-P
  {
    //Don't process interrupts during calculations
    detachInterrupt(digitalPinToInterrupt(3) );
    Serial.print("interval:");
    Serial.print(interval);
    Serial.print(" rpm:");
    Serial.print(rpm);
    Serial.print(" cnt:");
    Serial.println(rpmcount);
    // found out that my crankshaft generates 2 pulses per revolution (or is it 2 pulses per 720 degrees)?
    // 4 stroke engines have 720 degrees not just 360...
    rpm = ((60000/interval)*rpmcount ) / 2 ;
    rpmcount = 0;
    timeold = millis();
    //Restart the interrupt processing
    attachInterrupt(digitalPinToInterrupt(3) , rpm_fun, FALLING);
  }

  if (doflash) flashOn();
  
  leds = 0;
  
  if ( rpm > 1500 )    bitSet(leds, 6);
  if ( rpm > 2000 )    bitSet(leds, 5);
  if ( rpm > 2500 )    bitClear(leds,6);
  if ( rpm > 3000 )    bitSet(leds, 4);
  if ( rpm > 3500 )    bitClear(leds,5);
  if ( rpm > 4000 )    bitSet(leds, 3);
  if ( rpm > 4500 )    bitClear(leds,4);
  if ( rpm > 5000 )    bitSet(leds, 2);
  if ( rpm > 5500 )    bitClear(leds,3);
  if ( rpm > 6000 )    bitSet(leds, 1);
  if ( rpm > 7000 )    bitClear(leds,2);
  

  updateShiftRegister();
}
