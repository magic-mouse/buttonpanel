#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

#define AS5600_ADDR 0x36
#define OLED_ADDR   0x3C
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

Adafruit_SH1106G display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// EMA smoothing
float angleFiltered = 0;
float alpha = 0.15; // smaller = smoother, slower

float threshold = 1.0;
float lastAngleDeg = 0;

volatile bool updateButtonsInterrupt = false;




void onButtonIRQ(){
  updateButtonsInterrupt = true;
}


void setup() {
  Wire.begin();
  Serial.begin(115200);

  pinMode(7, INPUT_PULLUP);   // or INPUT depending on your wiring
  attachInterrupt(digitalPinToInterrupt(7), onButtonIRQ, RISING);


  // Initialize OLED
  if(!display.begin(OLED_ADDR, true)) {
    while(true); // init failed
  }

  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(0,0);
  display.println("Init...");
  display.display();
  delay(500);
}

// Read 12-bit angle from AS5600
uint16_t readAngle() {
  Wire.beginTransmission(AS5600_ADDR);
  Wire.write(0x0C); // high byte
  Wire.endTransmission(false);

  Wire.requestFrom(AS5600_ADDR, 2);
  if(Wire.available() == 2) {
    uint8_t high = Wire.read();
    uint8_t low  = Wire.read();
    return ((uint16_t)high << 8 | low) & 0x0FFF;
  }
  return 0;
}

void updateButtons(){
  if(updateButtons){

    display.println("Button!!!");

  }
}

void loop() {
  uint16_t raw = readAngle();


  // Apply EMA
  if(angleFiltered == 0) angleFiltered = raw; // initialize first value
  angleFiltered = alpha * raw + (1 - alpha) * angleFiltered;

  float angleDeg = (angleFiltered * 360.0) / 4096.0;

  if(abs(angleDeg - lastAngleDeg) > threshold) {
    lastAngleDeg = angleDeg;
    // update display
      display.clearDisplay();
    display.setCursor(0,0);
    display.print("Deg: "); display.println(angleDeg, 1);
    updateButtons();    
    display.display();
  }



  delay(50); // update ~20 Hz
}
