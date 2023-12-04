#include <ArduinoBearSSL.h>
#include <ArduinoECCX08.h>
#include <ArduinoMqttClient.h>
#include <WiFiNINA.h>
#include "arduino_secrets.h"
#include <Servo.h>

Servo servo;

int cds_value = 0;
int condition = 0;

#include <ArduinoJson.h>

const char ssid[]        = SECRET_SSID;
const char pass[]        = SECRET_PASS;
const char broker[]      = SECRET_BROKER;
const char* certificate  = SECRET_CERTIFICATE;

WiFiClient    wifiClient;            // Used for the TCP socket connection
BearSSLClient sslClient(wifiClient); // Used for SSL/TLS connection, integrates with ECC508
MqttClient    mqttClient(sslClient);

unsigned long lastMillis = 0;

void setup() {
  servo.attach(7);
  servo.write(0);
  Serial.begin(115200);
  while (!Serial);

  if (!ECCX08.begin()) {
    Serial.println("No ECCX08 present!");
    while (1);
  }
  ArduinoBearSSL.onGetTime(getTime);
  sslClient.setEccSlot(0, certificate);
  mqttClient.onMessage(onMessageReceived);
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
  }
  int cds_value = analogRead(A1)/4;
  if (!mqttClient.connected()) {
    // MQTT client is disconnected, connect
    connectMQTT();
  }

  if (condition == 1) {
    servo.write(180);
  }
  else if (condition == 2) {
    servo.write(90);
  }
  else if (condition == 3) {
    servo.write (0);
  }
  else {
    if ( cds_value >= 101 ) {
      servo.write(170);
    }
    else if ( cds_value<= 100 && cds_value >= 31) {
      servo.write(90);
    }
    else if( cds_value <= 30){
      servo.write(0);
    }
  }
  // poll for new MQTT messages and send keep alives
  mqttClient.poll();

  // publish a message roughly every 5 seconds.
  if (millis() - lastMillis > 5000) {
    lastMillis = millis();
    char payload[512];
    getDeviceStatus(payload);
    sendMessage(payload);
  }
}

unsigned long getTime() {
  // get the current time from the WiFi module  
  return WiFi.getTime();
}

void BlindWork() {
  int cds_value = analogRead(A1);
  if ( cds_value/4>=101 ) {
    servo.write(170);
  }
  else if ( cds_value<=100 && cds_value>=31) {
    servo.write(90);
  }
  else if( cds_value<=30){
    servo.write(0);
  }
}

void connectWiFi() {
  Serial.print("Attempting to connect to SSID: ");
  Serial.print(ssid);
  Serial.print(" ");

  while (WiFi.begin(ssid, pass) != WL_CONNECTED) {
    // failed, retry
    Serial.print(".");
    delay(5000);
  }
  Serial.println();

  Serial.println("You're connected to the network");
  Serial.println();
}

void connectMQTT() {
  Serial.print("Attempting to MQTT broker: ");
  Serial.print(broker);
  Serial.println(" ");

  while (!mqttClient.connect(broker, 8883)) {
    // failed, retry
    Serial.print(".");
    delay(5000);
  }
  Serial.println();

  Serial.println("You're connected to the MQTT broker");
  Serial.println();

  // subscribe to a topic
  mqttClient.subscribe("$aws/things/MyMKRWiFi1010/shadow/update/delta");
}

void getDeviceStatus(char* payload) {
  // Read temperature as Celsius (the default)
  int cds_value = analogRead(A1)/4;

  condition = 0;

  // make payload for the device update topic ($aws/things/MyMKRWiFi1010/shadow/update)
  sprintf(payload,"{\"state\":{\"reported\":{\"SunShine\":\"%d\",\"Blind Condition\":\"%d\"}}}",cds_value,condition);
}

void sendMessage(char* payload) {
  char TOPIC_NAME[]= "$aws/things/MyMKRWiFi1010/shadow/update";
  
  Serial.print("Publishing send message:");
  Serial.println(payload);
  mqttClient.beginMessage(TOPIC_NAME);
  mqttClient.print(payload);
  mqttClient.endMessage();
}


void onMessageReceived(int messageSize) {
  // we received a message, print out the topic and contents
  Serial.print("Received a message with topic '");
  Serial.print(mqttClient.messageTopic());
  Serial.print("', length ");
  Serial.print(messageSize);
  Serial.println(" bytes:");

  // store the message received to the buffer
  char buffer[512] ;
  int count=0;
  while (mqttClient.available()) {
     buffer[count++] = (char)mqttClient.read();
  }
  buffer[count]='\0'; // 버퍼의 마지막에 null 캐릭터 삽입
  Serial.println(buffer);
  Serial.println();

  DynamicJsonDocument doc(1024);
  deserializeJson(doc, buffer);
  JsonObject root = doc.as<JsonObject>();
  JsonObject state = root["state"];
  const char* condition_c = state["Condition"];
  Serial.println(condition_c);
  
  char payload[512];
  
  if (strcmp(condition_c,"1")==0) {
    servo.write(0);
    sprintf(payload,"{\"state\":{\"reported\":{\"Condition\":\"%s\"}}}","1");
    condition = 1;
    sendMessage(payload);
  } 
  else if (strcmp(condition_c,"2")==0) {
    servo.write(90);
    sprintf(payload,"{\"state\":{\"reported\":{\"Condition\":\"%s\"}}}","2");
    sendMessage(payload);
  } 
  else if (strcmp(condition_c,"3")==0) {
    servo.write(170);
    sprintf(payload,"{\"state\":{\"reported\":{\"Condition\":\"%s\"}}}","3");
    sendMessage(payload);
  }
}