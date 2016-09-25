#include "FastLED.h"

#define LED_PIN_LEFT    6        //Data pins for bitbashing the instructions to the WS2811 LEDS I'm using.
#define LED_PIN_RIGHT   5        // two strips of LEDs to make the wiring simpler and lighter on the plane
#define COLOR_ORDER     GRB      // GREEN RED BLUE byte order
#define CHIPSET         WS2811   //WS2811 LED chipset.  takes a bitbashed signal on the middle wire to set RGB for each LED on the strip
#define NUM_LEDS        16       //16 LEDS on each wing.
#define BRIGHTNESS      255      // Maximum brightness is 255/255, set to max so we can control it elsewhere.

// Define the arrays of LEDs
CRGB LEDS_LEFT[NUM_LEDS];
CRGB LEDS_RIGHT[NUM_LEDS];

//DEFINITIONS FOR READING A PWM SIGNAL FROM AN RC RECEIVER
#define INTERRUPT 0
#define INTERRUPT_PIN 2           //interrupt index is 0, which refers to physical pin 2 on the arduino
#define PULSE_WIDTH_NEUTRAL 1500  //pulse width, in microseconds, which describes neutral, or 90/180 degrees rotation to the servo

volatile int dutyCycle = PULSE_WIDTH_NEUTRAL;
volatile unsigned long pulseStartTime = 0;


///////SETUP FUNCTION
void setup()
{
   FastLED.addLeds<CHIPSET, LED_PIN_LEFT, COLOR_ORDER>(LEDS_LEFT, NUM_LEDS).setCorrection( TypicalLEDStrip );
   FastLED.addLeds<CHIPSET, LED_PIN_RIGHT, COLOR_ORDER>(LEDS_RIGHT, NUM_LEDS).setCorrection( TypicalLEDStrip );

   FastLED.setBrightness( BRIGHTNESS );

   attachInterrupt(INTERRUPT, readPWM, CHANGE);
   Serial.begin(9600); 
  }
////END SETUP FUNCTION

void loop() 
{ 

  Serial.print("Current Duty Cycle");
  Serial.println(dutyCycle);
  

  /*
    WS2811 LEDS 0-11 are strobes mounted under the leading edges of the wings
    WS2811 LEDS 12-15 are the nav lights on the tips of the undersides of the wings
  */
/*
 int lightMode = map(dutyCycle, 1000,2000, 0, 4); //Lazily remap the pwm signal to a 0-4 state so I don't have to deal with PWM measurment our output bounce and I don't have to mess with if else if else if for half the code.

  if(dutyCycle > 2050) //means we're out of normal PWM signal range, means a fault or the receiver is disconnected.
    lightMode = 0;

*/
  if(dutyCycle < 1010)
    basicNavLights();
    
  else if(dutyCycle < 1260)
    neeNaw();
    
  else if(dutyCycle < 1510)
    lightsOff();
  
  else if(dutyCycle < 1760)
      allGreen();
  
  else if(dutyCycle < 2010)
      allRed();
  
  else
     basicNavLights();
     
  FastLED.show(); 
  delay(10);
  

}

void basicNavLights()
{
  long splitSecond = millis() % 1000;
  
  LEDS_LEFT [12] = LEDS_LEFT [15] = splitSecond < 500 ? CRGB::Red : CRGB::Black;
  LEDS_LEFT [13] = LEDS_LEFT [14] = splitSecond < 500 ? CRGB::Black : CRGB::Red;
  //Swap which Leds are illuminated based on the time.  
  //this creates a flashing/growing & shrinking effect that makes them more visible.
  
  LEDS_RIGHT[12] = LEDS_RIGHT[15] = splitSecond < 500 ? CRGB::Green : CRGB::Black;
  LEDS_RIGHT[13] = LEDS_RIGHT[14] = splitSecond < 500 ? CRGB::Black : CRGB::Green;
  
  if(splitSecond < 550 && splitSecond > 400 || splitSecond < 300 && splitSecond > 150)
  {
    for(int i = 0; i < 12; i++)
    {
      LEDS_LEFT[i] = LEDS_RIGHT[i] = CRGB::White;
    }
  }
  else
  {
    for(int i = 0; i < 12; i++)
    {
      LEDS_LEFT[i] = LEDS_RIGHT[i] = CRGB::Black;
    }
    
    for(int i = 4; i < 8; i++)
    {
      LEDS_LEFT[i] = LEDS_RIGHT[i] = CRGB::White;
    }
  }
}

void neeNaw()
{
  long splitSecond = millis() % 1000;

  for(int i = 0; i < NUM_LEDS; i++)
  {
    LEDS_LEFT [i]   = splitSecond < 500 ? CRGB::Red : CRGB::Blue;
    LEDS_RIGHT [i]  = splitSecond < 500 ? CRGB::Blue : CRGB::Red;
  }
  
}

void lightsOff()
{
  for(int i = 0; i < NUM_LEDS; i++)
  {
    LEDS_LEFT [i]   = CRGB::Black;
    LEDS_RIGHT [i]  = CRGB::Black;
  }
}

void allGreen()
{
  for(int i = 0; i < NUM_LEDS; i++)
  {
    LEDS_LEFT [i]   = CRGB::Green;
    LEDS_RIGHT [i]  = CRGB::Green;
  }
}


void allRed()
{
  for(int i = 0; i < NUM_LEDS; i++)
  {
    LEDS_LEFT [i]   = CRGB::Red;
    LEDS_RIGHT [i]  = CRGB::Red;
  }
}


void readPWM()
{
  if(digitalRead(INTERRUPT_PIN) == HIGH)
  { 
    //set start time of the leading edge of the PWM pulse using the slightly innacurate microseconds function provided by the arduino
    //under load this can become innaccurate.
    pulseStartTime = micros();
  }
  else
  {
    //falling edge of the pulse, so subtract current time from start time to get the length of the duty cycle pulse.
          dutyCycle = (int)(micros() - pulseStartTime);
  }
}


