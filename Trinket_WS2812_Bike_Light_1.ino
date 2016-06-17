//based on the Adafruit Strandtest, verified to work on an Adafruit Trinket
// Uses TinyPinChange to catch a button press to switch between animations.
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif
#include <TinyPinChange.h>
const int buttonPin = 2;

volatile int buttonState = LOW;
volatile int Pattern = 0;
const int MaxPattern = 3;
volatile boolean jumpOut = false;
#define PIN 1 // connected to the pixels
#define USERLED 3  // blink me between different color combinations
#define NUM_LEDS 50
int i = 0;
#define UP 1
#define DOWN -1
int dir = UP;

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, PIN, NEO_RGB + NEO_KHZ800);

// IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across
// pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
// and minimize distance between Arduino and first pixel.  Avoid connecting
// on a live circuit...if you must, connect GND first.

void setup() {
  // This is for Trinket 5V 16MHz, you can remove these three lines if you are not using a Trinket
  #if defined (__AVR_ATtiny85__)
    if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
  #endif
  // End of trinket special code
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(USERLED, OUTPUT);
  digitalWrite(USERLED, buttonState);
  TinyPinChange_Init();
  TinyPinChange_RegisterIsr(buttonPin, pin_ISR);
  TinyPinChange_EnablePin(buttonPin);  
  strip.setBrightness(128); // 0-256, 64 peaks out at ~80mA, with no brightness, ~300mA.
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
}

void loop() {
  byte r, g, b;
  // Some example procedures showing how to display to the pixels:
  switch( Pattern ){
    case 0:
      rainbowCycle( random(0,10) );
      break;
    case 1:
      rainbow(20);
      break;  
    case 2:
      r = random( 0, 255 );
      g = random( 0, 64 );
      b = random( 0, 255 );
      Sparkle_bg(r, g, b, random( 20, 100) );
      break;
    case 3:
      rainbow_sparkle(20);
      break;  
    }
}

void pin_ISR() {
  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();
    // If interrupts come faster than 300ms, assume it's a bounce and ignore
    if (interrupt_time - last_interrupt_time > 300){
      jumpOut = true;
      buttonState = !buttonState;
      digitalWrite(USERLED, buttonState);
      Pattern = ++Pattern;
      if( Pattern > MaxPattern )
        Pattern = 0;
    }
    last_interrupt_time = interrupt_time;
}


// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    if( jumpOut ){
      jumpOut = false;
      break;
    }
    delay(wait);
  }
}

void rainbow(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i+j) & 255));
    }
    strip.show();
    if( jumpOut ){
      jumpOut = false;
      return;
    }
    delay(wait);
  }
}

void rainbow_sparkle(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i+j) & 255));
    }
    int pixel = random(NUM_LEDS);
    setPixel(pixel,255,255,255);
    strip.show();
    strip.setPixelColor(i, Wheel((i+j) & 255));
    if( jumpOut ){
      jumpOut = false;
      return;
    }
    delay(wait);
  }
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    if( jumpOut ){
      jumpOut = false;
      return;
    }
    delay(wait);
  }
}


// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}


void Sparkle_bg(byte red, byte green, byte blue, int SpeedDelay) {
  int Pixel = random(NUM_LEDS);

  i = i + dir;
  setAll(i, 0, i*2 );
  if( i == 20 ) dir = DOWN;
  if( i == 2 ) dir = UP;
  
  setPixel(Pixel,red,green,blue);
  strip.show();
  delay(SpeedDelay);
  setPixel(Pixel,0,0,0);
}
void setPixel(int Pixel, byte red, byte green, byte blue) {
   strip.setPixelColor(Pixel, strip.Color(red, green, blue));
}

void setAll(byte red, byte green, byte blue) {
  for(int i = 0; i < NUM_LEDS; i++ ) {
    setPixel(i, red, green, blue); 
  }
  strip.show();
}
