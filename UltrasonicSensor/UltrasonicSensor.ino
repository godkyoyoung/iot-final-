#include <SoftwareSerial.h>
#include "SNIPE.h"

#define PING  1
#define PONG  2

#define CODE  PING    /* Please define PING or PONG */

#define TXpin 11  
#define RXpin 10
#define ATSerial Serial

int trig = 3;      // NoRa TXD = 10  
int echo = 2;      // NoRa RXD = 11
int count = 0;

//16byte hex key
String lora_app_key = "99 88 33 44 55 66 77 88 99 aa bb cc dd ee ff 00";  

SoftwareSerial DebugSerial(RXpin,TXpin);
SNIPE SNIPE(ATSerial);

void setup() {
  ATSerial.begin(115200);
  pinMode(trig,OUTPUT);     //trig 핀 모드 설정
  pinMode(echo,INPUT);      //echo 핀 모드 설정
  Serial.begin(115200); 

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
  
#if CODE == PING
        float Length, distance;
        digitalWrite(trig, LOW);     //초기화
        delay(2);
        digitalWrite(trig, HIGH);    // trigger 신호 발생 (10us)
        delay(10);
        digitalWrite(trig, LOW);
        Length = pulseIn(echo, HIGH);
        distance = ((float)(340 * Length) / 10000) / 2; 

        if (distance <= 30){
          count ++;
          if (count >= 5){
            if (SNIPE.lora_send("PING"))
            {
              DebugSerial.println("send success");
              
              String ver = SNIPE.lora_recv();
              DebugSerial.println(ver);
    
              if (ver == "PONG")
              {
                DebugSerial.println("recv success");
                //DebugSerial.println(SNIPE.lora_getRssi());
                //DebugSerial.println(SNIPE.lora_getSnr());            
              }
              else
              {
                DebugSerial.println("recv fail");
                delay(500);
              }
              
            }
          }
        }
        else
        {
          count = 0;
        }
        delay(1000);
        DebugSerial.println(count);

       
#elif CODE == PONG
        String ver = SNIPE.lora_recv();
        delay(300);

        DebugSerial.println(ver);
        
        if (ver == "PING" )
        {
          DebugSerial.println("recv success");
          DebugSerial.println(SNIPE.lora_getRssi());
          DebugSerial.println(SNIPE.lora_getSnr());

          if(SNIPE.lora_send("PONG"))
          {
            DebugSerial.println("send success");
          }
          else
          {
            DebugSerial.println("send fail");
            delay(500);
          }
        }
       delay(1000);
#endif
}