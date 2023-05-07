// include the library
#include <RadioLib.h>
#include <Adafruit_NeoPixel.h>

// include the config
#include "config.h"

SX1280 radio = new Module(SX1280_NSS, SX1280_DI01, SX1280_NRST, SX1280_BUSY);
Adafruit_NeoPixel pixels(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

// flag to indicate that a packet was received
volatile bool receivedFlag = false;

// disable interrupt when it's not needed
volatile bool enableInterrupt = true;

// this function is called when a complete packet
// is received by the module
// IMPORTANT: this function MUST be 'void' type
//            and MUST NOT have any arguments!
#if defined(ESP8266) || defined(ESP32)
  ICACHE_RAM_ATTR
#endif

void setFlag(void) {
  // check if the interrupt is enabled
  if(!enableInterrupt) {
    return;
  }

  // we got a packet, set the flag
  receivedFlag = true;
}

void func_rxSetData(){
  if(receivedFlag) {
    enableInterrupt = false;

    // reset flag
    receivedFlag = false;

    byte byteArr[33];
    int state = radio.readData(byteArr, 33);

    if (state == RADIOLIB_ERR_NONE) {
//        for(int i=0; i < 5; i++)
//        {
//          Serial.print(byteArr[i]);
//          Serial.print("\t");
//        }
//        Serial.println();
        upDown = byteArr[1];
        leftRight= byteArr[2];
    } else if (state == RADIOLIB_ERR_CRC_MISMATCH) {
      // packet was received, but is malformed
//      Serial.println(F("CRC error!"));
    } else {
      // some other error occurred
      Serial.print(F("failed, code "));
      Serial.println(state);
    }

    radio.startReceive();
    enableInterrupt = true;
  }
}

void func_rxControl(){

  // Set the Forward/Backward
  if ( (upDown > defaultUpDown) or (upDown < defaultUpDown) ) {
    roverUpDownDirection = (upDown > defaultUpDown) ? 1 : 0;
    roverSpeed = (upDown > defaultUpDown) ? map(upDown, defaultUpDown, 255, defaultUpDown, 255) : map(upDown, 0, defaultUpDown, 255, defaultUpDown);
  } else {
    upDown = 127;
    roverSpeed = 0;
    roverUpDownDirection = 1;
  }

  // Set the left/right
  if ( (leftRight > defaultLeftRight) or (leftRight < defaultLeftRight) ) {
    roverLeftRightDirection = (leftRight > defaultLeftRight) ? 1 : 0;
    roverLeftRightDirectionSpeed = (leftRight > defaultLeftRight) ? map(leftRight, defaultLeftRight, 255, defaultLeftRight, 255) : map(leftRight, 0, defaultLeftRight, 255, defaultLeftRight);
  } else {
    leftRight = 127;
    roverLeftRightDirection = 0;
    roverLeftRightDirectionSpeed = 0;
  }

  // Move without changing direction
  if(roverSpeed > 0 && roverLeftRightDirectionSpeed == 0) {
    // Set the speed for each side
    setWheelSpeed(
      roverSpeed, roverSpeed
    );

    // Set the direction for each wheel
    setWheelDirection(
      roverUpDownDirection, // L: back
      roverUpDownDirection, // L: front
  
      roverUpDownDirection, // R: back
      roverUpDownDirection  // R: front
    );

  // Turn Left/Right without moving forward or backward
  } else if(roverSpeed == 0 && roverLeftRightDirectionSpeed > 0) {

    // Set the speed for each side
    setWheelSpeed(
      roverLeftRightDirectionSpeed, roverLeftRightDirectionSpeed
    );

    // If we'll only change the direction
    setWheelDirection(
      roverLeftRightDirection, // L: back
      roverLeftRightDirection, // L: front
  
      roverLeftRightDirection == 1 ? 0 : 1, // R: back
      roverLeftRightDirection == 1 ? 0 : 1 // R: front
    );


    Serial.print(roverLeftRightDirection);
    Serial.println();

  } else if(roverSpeed > 0 && roverLeftRightDirectionSpeed > 0) {
    // Set the speed for each side
    setWheelSpeed(
      roverSpeed,//left side
      roverSpeed//right side
    );


    // Set the direction for each wheel
    setWheelDirection(
      roverLeftRightDirection == 1 ? 2 : roverUpDownDirection, // L: back
      roverLeftRightDirection == 0 ? 2 : roverUpDownDirection, // L: front
  
      roverLeftRightDirection == 0 ? 2 : roverUpDownDirection, // R: back
      roverLeftRightDirection == 1 ? 2 : roverUpDownDirection  // R: front
    );
  } else {

    // Set the speed for each side
    setWheelSpeed(
      0, 0
    );
  }

  if(roverLeftRightDirectionSpeed > 0 or roverSpeed > 0) {
    pixels.setBrightness(30);
  
    ledChange(3,173,21,10);
    ledChange(4,173,21,10);
    
    ledChange(1,238,238,238);
  } else {
    ledChange(3,201,0,0);
    ledChange(4,201,0,0);
    ledChange(1,153,153,153);
  }

  if(roverLeftRightDirectionSpeed > 0) {
      if(roverLeftRightDirection == 0) {
        ledChange(2,255,230,19);
      } else {
        ledChange(5,255,230,19);
      }
  } else {
      ledChange(2,0,0,0);
      ledChange(5,0,0,0);
  }
}

void setWheelDirection(int wheel_1, int wheel_2, int wheel_3, int wheel_4 ) {

  // Left Side
  // - back
  digitalWrite(M2_IN1, wheel_1 == 2 ? LOW : wheel_1 == 1 ? LOW : HIGH);
  digitalWrite(M2_IN2, wheel_1 == 2 ? LOW : wheel_1 == 1 ? HIGH : LOW);

  // - front
  digitalWrite(M1_IN3, wheel_2 == 2 ? LOW : wheel_2 == 1 ? LOW : HIGH);
  digitalWrite(M1_IN4, wheel_2 == 2 ? LOW : wheel_2 == 1 ? HIGH : LOW);

  // Right Side
  // - back
  digitalWrite(M2_IN3, wheel_3 == 2 ? LOW : wheel_3 == 1 ? HIGH : LOW);
  digitalWrite(M2_IN4, wheel_3 == 2 ? LOW : wheel_3 == 1 ? LOW : HIGH);

  // - front
  digitalWrite(M1_IN1, wheel_4 == 2 ? LOW : wheel_4 == 1 ? LOW : HIGH);
  digitalWrite(M1_IN2, wheel_4 == 2 ? LOW : wheel_4 == 1 ? HIGH : LOW);
}

void setWheelSpeed(int leftSide, int rightSide) {
  analogWrite(ENA, leftSide);
  analogWrite(ENB, rightSide);
}

void setup() {
  Serial.begin(115200);
  pinMode(VOLTAGE_DIVIDER, INPUT);

  pixels.begin();
  pixels.setBrightness(10);
  
  pixels.clear();

  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);

  pinMode(M1_IN1, OUTPUT);
  pinMode(M1_IN2, OUTPUT);
  pinMode(M1_IN3, OUTPUT);
  pinMode(M1_IN4, OUTPUT);

  pinMode(M2_IN1, OUTPUT);
  pinMode(M2_IN2, OUTPUT);
  pinMode(M2_IN3, OUTPUT);
  pinMode(M2_IN4, OUTPUT);

  // initialize SX1280 with default settings
  Serial.print(F("[SX1280] Initializing ... "));

  int state = radio.beginFLRC();

  state = radio.setFrequency(frequency);
  state = radio.setBitRate(bitRate);
  state = radio.setCodingRate(codingRate);
  state = radio.setOutputPower(outputPower);
  state = radio.setDataShaping(dataShaping);
  state = radio.setSyncWord(syncWord, 4);
  state = radio.setCRC(crcValue);

  // set the function that will be called
  // when new packet is received
  radio.setDio1Action(setFlag);

  // start listening for LoRa packets
  Serial.print(F("[SX1280] Starting to listen ... "));
  state = radio.startReceive();
  if (state == RADIOLIB_ERR_NONE) {
    Serial.println(F("success!"));
  } else {
    Serial.print(F("failed, code "));
    Serial.println(state);
    while (true);
  }

}

void ledChange(int lightPosition, int c1, int c2, int c3) {
  if(lightPosition == 1 ) {
    pixels.setPixelColor(0, pixels.Color(c1, c2, c3));
    pixels.setPixelColor(1, pixels.Color(c1, c2, c3));
    pixels.setPixelColor(2, pixels.Color(c1, c2, c3));
    pixels.setPixelColor(3, pixels.Color(c1, c2, c3));
    pixels.setPixelColor(4, pixels.Color(c1, c2, c3));
    pixels.setPixelColor(5, pixels.Color(c1, c2, c3));
  } else if(lightPosition == 2) {
    pixels.setPixelColor(6, pixels.Color(c1, c2, c3));
    pixels.setPixelColor(7, pixels.Color(c1, c2, c3));
  } else if(lightPosition == 3) {
    pixels.setPixelColor(8, pixels.Color(c1, c2, c3));
    pixels.setPixelColor(9, pixels.Color(c1, c2, c3));
    pixels.setPixelColor(10, pixels.Color(c1, c2, c3));
  } else if(lightPosition == 4) {
    pixels.setPixelColor(11, pixels.Color(c1, c2, c3));
    pixels.setPixelColor(12, pixels.Color(c1, c2, c3));
    pixels.setPixelColor(13, pixels.Color(c1, c2, c3));
  } else if(lightPosition == 5) {
    pixels.setPixelColor(14, pixels.Color(c1, c2, c3));
    pixels.setPixelColor(15, pixels.Color(c1, c2, c3));
  }
  pixels.show();
}

void loop() {
  func_rxSetData();
  func_rxControl();


  int batteryAnalogValue = analogRead(VOLTAGE_DIVIDER); // read the analog value from the voltagePin
  int voltage = batteryAnalogValue * 3.12;
  if(voltage < 7) {
    for(int i=0; i<NUM_LEDS; i++) {
      pixels.setPixelColor(i, pixels.Color(201, 0, 0));
    }
    pixels.show();
  }

}
