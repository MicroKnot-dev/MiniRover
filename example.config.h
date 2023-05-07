/**
 * SX1280
 */
#define SX1280_NSS 2
#define SX1280_DI01 26
#define SX1280_NRST 14
#define SX1280_BUSY 21

float frequency     = 2400.0;
int bitRate         = 520;
int codingRate      = 2;
int outputPower     = 8;
float dataShaping   = 1.0;
uint8_t syncWord[]  = {0x01, 0x23, 0x45, 0x67};
int crcValue        = 0;

int upDown = 127,
    defaultUpDown = 127,
    leftRight = 127,
    defaultLeftRight = 127,
    roverUpDownDirection = 1,
    roverLeftRightDirection = 0,
    roverSpeed = 0,
    roverLeftRightDirectionSpeed = 0;

// Voltage Divider
#define VOLTAGE_DIVIDER 13

// LED
#define LED_PIN     32
#define NUM_LEDS    16

// Motor Drivers
#define ENA 15
#define ENB 23

#define M1_IN1 4
#define M1_IN2 16
#define M1_IN3 17
#define M1_IN4 22

#define M2_IN1 33
#define M2_IN2 25
#define M2_IN3 27
#define M2_IN4 12
