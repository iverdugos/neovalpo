#include <HCSR04.h>
#include <FastLED.h>

FASTLED_USING_NAMESPACE

#define DATA_PIN    7
#define LED_TYPE    WS2812 // o WS2811 o NEOPIXEL
#define COLOR_ORDER GRB  //  GREEN RED BLUE, comun en neopixeles
#define NUM_LEDS    12  //  12 leds tiene nuestro anillo de luz
CRGB leds[NUM_LEDS];
#define BRIGHTNESS          96
#define FRAMES_PER_SECOND  120

HCSR04 sensor1(5,6);
HCSR04 sensor2(9,10);


void setup() {
  delay(3000); 
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);
  Serial.begin(9600);
 
}

uint8_t gHue = 0; // rotating "base color" used by many of the patterns

void loop() {

  if(sensor1.dist() < 20.0){
      for(int whiteLed = 0; whiteLed < NUM_LEDS; whiteLed = whiteLed + 1) {
       leds[whiteLed] = CRGB::White;
        FastLED.show();
       delay(1);
        leds[whiteLed] = CRGB::Green;
       }
   }
   if(sensor2.dist() < 20.0){
      for(int whiteLed = 0; whiteLed < NUM_LEDS; whiteLed = whiteLed + 1) {
        leds[whiteLed] = CRGB::White;
        FastLED.show();
        delay(1);
       leds[whiteLed] = CRGB::Red;
    } 
  }
}
