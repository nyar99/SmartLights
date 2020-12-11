#define TIMER_INTERRUPT_DEBUG      0

#define USE_TIMER_1     true
#define USE_TIMER_2     false
#define USE_TIMER_3     false
#define USE_TIMER_4     false
#define USE_TIMER_5     false

#define FASTLED_ALLOW_INTERRUPTS 0
#define FASTLED_INTERRUPT_RETRY_COUNT 1

#include "TimerInterrupt.h"
#include <FastLED.h>
#define LED_PIN 3
#define NUM_LEDS 45
CRGB leds[NUM_LEDS];

//Pattern Control
volatile int repetitions = 5;
volatile int selectedPattern = 0;
volatile int flashPattern_red = 100;
volatile int flashPattern_green = 200;
volatile int flashPattern_blue = 100;
#define NUMOFPATTERNS 5 

//Pattern Functions
//To add a new pattern:
//1: Add a new function
//2: Add a toggle and non toggle mode
//3: Increment num of patterns
//4: Append to array of function pointers

//flashes random colors at bpm
void flashRandomColor(bool toggle){
  if (!repetitions) {
    repetitions = 20;
  } else {repetitions--;}
  int r = random8(255);
  int g = random8(255);
  int b = random8(255);
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CHSV(g, r, b);  
  }
  FastLED.show();
}
//flashes sequential hues of the rainbow
void sequentialHues(bool toggle) {
  int startingHue = random8(255);
  if (!repetitions) {
    repetitions = (255 - startingHue);
  } else {
    repetitions-=7;
    if (repetitions <= 7) {
      repetitions = 0;
    }
  }
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i].setHue(repetitions);  
  }
  FastLED.show();
}
//flashes a random color for 5 beats
void flash(bool toggle){
  if (!repetitions) {
    repetitions = 5;
    flashPattern_red = random8(255);
    flashPattern_green = random8(255);
    flashPattern_blue = random8(255);
  } else {repetitions--;}
  if (toggle){
      fill_solid(leds, NUM_LEDS, CHSV(flashPattern_red, flashPattern_green, flashPattern_blue));
    } else {
      FastLED.clear();  
    }
  FastLED.show();
}

//flashes a rainbow pattern
void rainbow(bool toggle) {
  if(!repetitions) {
    repetitions = 10;  
  }  else {repetitions--;}
  if (toggle){
    fill_rainbow(leds, NUM_LEDS, random8(255));  
  } else {
    FastLED.clear();
  }
  FastLED.show();
}

void chase(bool toggle) {
  if(!repetitions) {
    repetitions = 10;
  }  else {repetitions--;}
  FastLED.clear();
  for (int i = (10-repetitions) * 3; i < (10-repetitions)*3 + 15; i++){
    leds[i] = CHSV(flashPattern_red, flashPattern_green, flashPattern_blue);
  }
  FastLED.show();
}

void (*pattern[5])(bool toggle) = {flash, flashRandomColor, sequentialHues, rainbow, chase};

//BPM Control
char tempo[9];
bool tempoAvailable = false;
int tempoIndex = 0;
float tempoNum;
unsigned long lastSerial = 0;


void TimerHandler1(){
  if(true)
  {
    static bool toggle1 = false;
    if (repetitions <= 0){
      selectedPattern = random8(0,NUMOFPATTERNS);
      Serial.println(selectedPattern);
    }
    (*pattern[selectedPattern])(toggle1);
    //flash(toggle1);
    //flashRandomColor(toggle1);
    //sequentialHues(toggle1);
    //rainbow(toggle1);
    //chase(toggle1);
    toggle1 = !toggle1;
  }
}



void setup() {
  // put your setup code here, to run once:
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
  Serial.begin(115200); 
  while(!Serial);
  ITimer1.init();
  if (ITimer1.attachInterruptInterval(400, TimerHandler1))
    Serial.println("Starting  ITimer1");
  else
    Serial.println("Can't set ITimer1. Select another freq. or timer");
}

int bpmToPeriodMs(float bpm){
  float hz = bpm/60.0;
  float s = 1.0/hz;
  int ms = round(1000 * s);
  return ms;
}

void loop() {
  if(Serial.available()){
    if(millis() - lastSerial > 1000){
      //probably a new tempo
      tempoIndex = 0;
      tempoAvailable = false;
    }
    char c = Serial.read();
    tempo[tempoIndex] = c;
    tempoIndex++;
    lastSerial = millis();
    Serial.print(c);
  }
  if (tempoIndex >=6){
    if (!tempoAvailable){
      tempo[tempoIndex+1] = '\0';
      if (round(atof(tempo)) != round(tempoNum)){
        tempoNum = atof(tempo);
        if (tempoNum < 100) {
          tempoNum *= 2;  
        }
        tempoAvailable = true;
        ITimer1.setInterval(bpmToPeriodMs(tempoNum), TimerHandler1);
        Serial.println("Changing Interval");
      }
    }
  }   
}
  
