#include <SoftwareSerial.h>
#include "SNIPE.h"

#define PING  1
#define PONG  2

#define CODE  PONG    /* Please define PING or PONG */

#define TXpin 11
#define RXpin 10
#define ATSerial Serial
int buzzer = 6;
int button = 7;
int led = 8;
int count1 = 0;
//16byte hex key
String lora_app_key = "99 88 33 44 55 66 77 88 99 aa bb cc dd ee ff 00";  

SoftwareSerial DebugSerial(RXpin,TXpin);
SNIPE SNIPE(ATSerial);

void setup() {
  ATSerial.begin(115200);


  Serial.begin (115200); 
  pinMode (led, OUTPUT) ; 
  pinMode (button, INPUT);
  pinMode (buzzer, OUTPUT);
  // put your setup code here, to run once:
  while(ATSerial.read()>= 0) {}
  while(!ATSerial);

  DebugSerial.begin(115200);

  /* SNIPE LoRa Initialization */
  if (!SNIPE.lora_init()) {
    DebugSerial.println("SNIPE LoRa Initialization Fail!");
    while (1);
    
  }

  /* SNIPE LoRa Set Appkey */
  if (!SNIPE.lora_setAppKey(lora_app_key)) {
    DebugSerial.println("SNIPE LoRa app key value has not been changed");
  }
  
  /* SNIPE LoRa Set Frequency */
  if (!SNIPE.lora_setFreq(LORA_CH_1)) {
    DebugSerial.println("SNIPE LoRa Frequency value has not been changed");
  }

  /* SNIPE LoRa Set Spreading Factor */
  if (!SNIPE.lora_setSf(LORA_SF_7)) {
    DebugSerial.println("SNIPE LoRa Sf value has not been changed");
  }

  /* SNIPE LoRa Set Rx Timeout 
   * If you select LORA_SF_12, 
   * RX Timout use a value greater than 5000  
  */
  if (!SNIPE.lora_setRxtout(5000)) {
    DebugSerial.println("SNIPE LoRa Rx Timout value has not been changed");
  }  
    
  DebugSerial.println("SNIPE LoRa PingPong Test");
}

void loop() {
  int readValue = digitalRead(button);
#if CODE == PING
      if(readValue==HIGH){
        if (SNIPE.lora_send("PING"))
        {
          DebugSerial.println("send success");
          
          
          String ver = SNIPE.lora_recv();
          DebugSerial.println(ver);

          if (ver == "PONG")
          {
           
            DebugSerial.println("recv success");
            DebugSerial.println(SNIPE.lora_getRssi());
            DebugSerial.println(SNIPE.lora_getSnr());            
          }
        }
          else
          {
            DebugSerial.println("recv fail");
            delay(500);
          }
        }
     
       delay(1000);
        DebugSerial.println(readValue);
       
#elif CODE == PONG
        String ver = SNIPE.lora_recv();
        delay(300);

        DebugSerial.println(ver);
        
        if (ver == "PING" )
        {
          digitalWrite (led, HIGH);
          if(count1 <= 5){
          digitalWrite(buzzer, HIGH);
          }
          delay(2000);
          DebugSerial.println("recv success");
          //DebugSerial.println(SNIPE.lora_getRssi());
          //DebugSerial.println(SNIPE.lora_getSnr());
          count1++;
          if(readValue==HIGH){
            if(SNIPE.lora_send("PONG"))
            {
              DebugSerial.println("send success");
            }
          }
        }
          else
          {
            digitalWrite(led, LOW);
            digitalWrite(buzzer,LOW);
            DebugSerial.println("send fail");
            delay(500);
            count1 = 0;
          }
        
       delay(1000);
#endif
}//button led buzzer 추가 코드