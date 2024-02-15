// https://forum.arduino.cc/t/16-bit-spi-adc-acts-like-a-15-bits/18964/13

// **********************************************
// Define the Arduino pins used for SPI interface
// **********************************************
#define SELPIN 10    // Selection pin
#define DATAOUT 11   // Controller out; peripheral in
#define DATAIN 12    // Controller in; peripheral out
#define SPICLOCK 13  // Clock

long readValue;
double readVoltage;

byte datainempty = 0b00000000;

// This has to be 4 if you are using the ADS8664 chip (read ch 1,2,3,4).
//For the ADS8668, you can set it to 8 (read ch 1,2,3,4,5,6,7,8). If you
// are only using 6 channels with an ADS8668 (for gantry stage control)
// set the value to 6. Channels are 1-indexed and the range in inclusive.
// This cannot skip channels, and always starts at 1.
int stopCh = 6;

int chN;

// *********************************************************
// Setup function - initializes the board and sets pin modes
// *********************************************************
void setup() {
  // Set pin modes
  pinMode(SELPIN, OUTPUT);
  pinMode(DATAOUT, OUTPUT);
  pinMode(DATAIN, INPUT);
  pinMode(SPICLOCK, OUTPUT);
  
  // Disable device at the outset
  digitalWrite(SELPIN, HIGH);
  digitalWrite(DATAOUT, LOW);
  digitalWrite(SPICLOCK, LOW);
  
  // Setup communications rate
  Serial.begin(9600);
  analogReference(DEFAULT);
}


void loop() {

  for(chN=1; chN <= stopCh; chN++) {

    // Loop to read all  channels and send it to serial
    readValue = read_adc(chN);

    // 5000 bc mVolts; 65535 for the max int value for 16 bits
    readVoltage = ((double(readValue)*20.48)/4096)-10.24;
    Serial.print(readVoltage);
    if (chN < stopCh) {
      Serial.print(", ");
    }
  }
  Serial.println();
  // Wait a bit for the analog-to-digital converter
  // to stabilize after the last reading
  delay(10);
}


//
int read_adc(int channel) {

  long adcvalue = 0;
  byte commandbits = 0b11000001;

  // Select channel
  switch(channel)  
  {
    case 1:  {
      commandbits = 0b11000001;
      }
      break;
    case 2:  {
      commandbits = 0b11000100;
      }
      break;
    case 3:  {
      commandbits = 0b11001000;
      }
      break;
    case 4:  {
      commandbits = 0b11001100;
      }
      break;
    case 5:  {
      commandbits = 0b11010000;
      }
      break;
    case 6:  {
      commandbits = 0b11010100;
      }
      break;
    case 7:  {
      commandbits = 0b11011000;
      }
      break;
    case 8:  {
      commandbits = 0b11011100;
      }
      break;
  }

  // Select adc
  digitalWrite(SELPIN,LOW);

  for (int i=7; i>=0; i--) {

    digitalWrite(DATAOUT,commandbits&1<<i);
    // Cycle the clock 8 times
    digitalWrite(SPICLOCK,HIGH);
    digitalWrite(SPICLOCK,LOW);
  }

  for (int i=7; i>=0; i--) {
    digitalWrite(DATAOUT, datainempty&1<<i);

    digitalWrite(SPICLOCK,HIGH);
    digitalWrite(SPICLOCK,LOW);
  }

  // Read bits from adc
  for (int i=11; i>=0; i--) {
    adcvalue+=digitalRead(DATAIN)<<i;

    digitalWrite(SPICLOCK,HIGH);
    digitalWrite(SPICLOCK,LOW);
  }
  
  // 32 cycles total
  digitalWrite(SELPIN, HIGH); //turn off device
  return adcvalue;
}