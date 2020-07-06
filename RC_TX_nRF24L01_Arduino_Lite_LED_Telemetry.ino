
#include <SPI.h>      //https://github.com/arduino/ArduinoCore-avr/tree/master/libraries/SPI
#include <nRF24L01.h> //https://github.com/nRF24/RF24 
#include <RF24.h>     //https://github.com/nRF24/RF24

//pins for driver
#define driv1   A0
#define driv2   A1
#define driv3   A2
#define driv4   A3
#define driv5   2
#define driv6   3
#define driv7   A4
#define driv8   A5

//led telemetry
#define ledVCC  4

//pins for nRF24L01
#define CE      9
#define CSN     10
//***** MOSI    11
//***** MISO    12
//***** SCK     13

RF24 radio(CE, CSN); //set CE and CSN pins

const byte addresses[][6] = {"tx001", "rx002"};

//**************************************************************************************************************************
//structure size max 32 bytes **********************************************************************************************
//**************************************************************************************************************************
struct tx_data
{
  byte ch1;
  byte ch2;
  byte ch3;
  byte ch4;
  byte ch5;
  byte ch6;
  byte ch7;
  byte ch8;
};
tx_data rc_data; //Create a variable with the above structure

//**************************************************************************************************************************
//this struct defines data, which are embedded inside the ACK payload ******************************************************
//**************************************************************************************************************************
struct ackPayload
{
  float vcc; //led telemetry
};
ackPayload payload;

//**************************************************************************************************************************
//reset values ​​(min = 0, mid = 127, max = 255) *****************************************************************************
//**************************************************************************************************************************
void resetData()
{
  rc_data.ch1 = 127;     
  rc_data.ch2 = 127;
  rc_data.ch3 = 127;
  rc_data.ch4 = 127;
  rc_data.ch5 = 0;
  rc_data.ch6 = 0;
  rc_data.ch7 = 127;
  rc_data.ch8 = 127;
}

//**************************************************************************************************************************
//**************************************************************************************************************************
//**************************************************************************************************************************
void inputDriver()
{
/*
 * Read all analog inputs and map them to one byte value
 * Normal:    rc_data.ch1 = map( analogRead(A0), 0, 1023, 0, 255);
 * Reversed:  rc_data.ch1 = map( analogRead(A0), 0, 1023, 255, 0);
 * Convert the analog read value from 0 to 1023 into a byte value from 0 to 255
 */ 
  rc_data.ch1 = map(analogRead(driv1), 0, 1023, 0, 255);
  rc_data.ch2 = map(analogRead(driv2), 0, 1023, 0, 255);
  rc_data.ch3 = map(analogRead(driv3), 0, 1023, 0, 255);
  rc_data.ch4 = map(analogRead(driv4), 0, 1023, 0, 255);
  rc_data.ch5 = digitalRead(driv5);
  rc_data.ch6 = digitalRead(driv6);
  rc_data.ch7 = map(analogRead(driv7), 333, 691, 7, 255); //Hitec Ranger throttle 333, 691, 7, 255
  rc_data.ch8 = map(analogRead(driv8), 330, 694, 4, 255); //Hitec Ranger steering 330, 694, 4, 255
}

//**************************************************************************************************************************
//initial main settings ****************************************************************************************************
//**************************************************************************************************************************
void setup()
{
  Serial.begin(9600);

  pinMode(ledVCC, OUTPUT); //led telemetry

  resetData(); //reset each channel value
  
  //define the radio communication
  radio.begin();
  
  radio.setAutoAck(1);             //ensure autoACK is enabled
  radio.enableAckPayload();        //allow optional ack payloads
  radio.enableDynamicPayloads();
  radio.setRetries(5, 5);          //minimum time between retries(5x 250 us delay (blocking !), max. 5 retries)
  
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_LOW);   //set power amplifier(PA): RF24_PA_MIN, RF24_PA_LOW, RF24_PA_HIGH and RF24_PA_MAX  

  radio.openWritingPipe(addresses[1]);    //rx002, both radios listen on the same pipes by default, and switch when writing
  radio.openReadingPipe(1, addresses[0]); //tx001
}

//**************************************************************************************************************************
//program loop *************************************************************************************************************
//**************************************************************************************************************************
void loop()
{               
  if (radio.write(&rc_data, sizeof(tx_data)))   //send all data from the structure and check if the transfer was successful
  {
    if (radio.isAckPayloadAvailable())
    {
      radio.read(&payload, sizeof(ackPayload)); //read the payload, if available
    }
  }

  inputDriver();
  led_indication();

//  Serial.println(rc_data.ch8); //print value ​​on a serial monitor
} //end program loop

//**************************************************************************************************************************
//telemetry with undervoltage detection by LED indication ******************************************************************
//**************************************************************************************************************************
void led_indication()
{
  if (payload.vcc == LOW)
  {
    digitalWrite(ledVCC, LOW);  //0.00
  }
  else
  {
    digitalWrite(ledVCC, HIGH); //1.00
  }
}
  
