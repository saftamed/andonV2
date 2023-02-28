#define DEBUG
#define NEW
#include "DebugUtils.h"

#include <EEPROM.h>
#define EEPROM_SIZE 5
#include <ArduinoOTA.h>

#ifdef ESP8266
 #include <ESP8266WiFi.h>
#else
 #include <WiFi.h>
#endif
#include <ModbusIP_ESP8266.h>

#define ADD 3

/*  ******   Old andon   ******** */
#ifdef NEW


#define C1 22
#define C2 21
#define C3 18
#define C4 4
/* ******   New new new new new new new andon   ******** */

#else

#define C1 18
#define C2 17
#define C3 16
#define C4 4

#endif


#define Red 32
#define Blue  33
#define Yellow  25
#define White 26

#define Green 13

#define DELAY 100

const int caps[] = { C1, C2, C3, C4 };
const int colors[] = { Red, Blue, Yellow, White };

#ifndef STASSID
#define STASSID "WIFI_Andon"
#define STAPSK "1122334455"
#endif

const char* ssid = STASSID;
const char* password = STAPSK;

const int REG = 1;               
IPAddress remote(192,168,0,222);  
const int LOOP_COUNT = 10;

ModbusIP mb; 
int caps_state[] = { 0, 0, 0, 0 };
uint16_t tot = 0;

int attm = 0;
unsigned long lastSend = 0;
unsigned long sendDelay = 60000;

void testSysteme(){
  colorsOff();
  delay(500);
  #ifdef NEW
  digitalWrite(Green,1);
  #else
  digitalWrite(Green,0);
  #endif
  for(int i =0;i<4;i++){
    if(i > 0)
      digitalWrite(colors[i-1],0);
    digitalWrite(colors[i],1);
    delay(500);   
  }
  digitalWrite(White,0);
}

void last_status(){
  EEPROM.begin(EEPROM_SIZE);
  delay(10);
   for(int i =0;i<4;i++){
     DEBUG_PRINTLN(" A A A");
    caps_state[i] = EEPROM.read(i);
    DEBUG_PRINTLN(i);
    DEBUG_PRINTLN(" : ");
    DEBUG_PRINTLN(caps_state[i]);
    if(caps_state[i] == 1){
       setAction(i);
       break;
    }
  }
}
void ota_setup() {
  pinMode(Red, OUTPUT);
  digitalWrite(Red, 1);
  DEBUG_PRINTLN("Booting");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  int aa = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    DEBUG_PRINT(":w ");
    aa++;
    if (aa >= 1500) {
      ESP.restart();
    }
  }

  String h = String("ANDON3") + String(ADD);
  ArduinoOTA.setHostname(h.c_str());
  ArduinoOTA.setPassword("cii");

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { 
      type = "filesystem";
    }

    DEBUG_PRINTLN("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    DEBUG_PRINTLN("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    DEBUG_PRINT((progress / (total / 100)));
    attm++;
    if(attm > 20){
      digitalWrite(Red, !digitalRead(Red));
      attm=0;
    }
  });
  ArduinoOTA.onError([](ota_error_t error) {
    if (error == OTA_AUTH_ERROR) {
      DEBUG_PRINTLN("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      DEBUG_PRINTLN("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      DEBUG_PRINTLN("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      DEBUG_PRINTLN("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      DEBUG_PRINTLN("End Failed");
    }
  });
  ArduinoOTA.begin();
  DEBUG_PRINTLN("Ready");
  DEBUG_PRINT("IP address: ");
  DEBUG_PRINTLN(WiFi.localIP());
  int a = 0;

  while (1) {
    ArduinoOTA.handle();
    delay(500);
    digitalWrite(Red, !digitalRead(Red));
    a++;
    if (a >= 600) {
      break;
    }
  }
  digitalWrite(Red, 0);
}
void setup_Prog(){
  pinMode(Green,OUTPUT);
  for(int i =0;i<4;i++){ 
   pinMode(colors[i],OUTPUT);
   pinMode(caps[i],INPUT_PULLUP);
  }


  if(digitalRead(C4) == 0){

  testSysteme();
  }

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  int a = 0;

  checkConnections(2000,600);

  // while (WiFi.status() != WL_CONNECTED) {
  //   delay(100);
  //   a++;
  //   if(a > 150){
  //     ESP.restart();
  //   }
  //   DEBUG_PRINT(".");
  // }
  DEBUG_PRINTLN("");
  DEBUG_PRINTLN("WiFi connected");  
  DEBUG_PRINTLN("IP address: ");
  DEBUG_PRINTLN(WiFi.localIP());

  mb.client();
  colorsOff();
  delay(1000);
  last_status();

  DEBUG_PRINTLN("updated");
}
void setup() {
  #ifdef DEBUG
  pinMode(C1 ,INPUT_PULLUP);
  Serial.begin(115200);
  while (!Serial)
    ;
  #endif
  if (digitalRead(C1) == 1) {
    DEBUG_PRINTLN("ota begin");
    ota_setup();
  }
  DEBUG_PRINTLN("program begin");
  setup_Prog();
  lastSend = millis();

}

int readInput(int pin){
  if(pin == 0){
    return digitalRead(caps[pin]);
  }else{
    return !digitalRead(caps[pin]);
  }
}
int readInput2(int pin){
  if(pin == 0){
    return !digitalRead(caps[pin]);
  }else{
    return digitalRead(caps[pin]);
  }
}

bool getButton(int index){
  Serial.println(1);
  int x = 0;
  for(int i =0;i<50;i++){
    int a = readInput(index);
    delay(1);
    if(!a){
      Serial.println(0);
      return false;
    }
  }
  return true;
}

bool getButton2(int index){
  int x = 0;
  for(int i =0;i<50;i++){
    int a = readInput2(index);
    delay(1);
    if(!a){
      return false;
    }
  }
  return true;
}


void loop() {
  for(int i =0;i<4;i++){
    caps_state[i] = readInput(i);
    if(caps_state[i]){
       if(getButton(i)){
         setAction(i);
         break;
       }
    }
   
  }
  if((millis() - lastSend) > sendDelay) {
      sendData2();
      lastSend = millis();
    }
  delay(DELAY);
}


void sendData2(){
   tot =  caps_state[0] + caps_state[2] * 2 + caps_state[1] * 3 + caps_state[3] * 4;
  DEBUG_PRINTLN(tot);
   uint16_t trans = mb.writeHreg(remote, (ADD-1), tot);
   checkConnections(1,1);
    while (mb.isTransaction(trans)) {
      mb.task();
      delay(10);
      DEBUG_PRINT(".");
    }
    DEBUG_PRINTLN("may be sended");
}
void sendData(){
 tot =  caps_state[0] + caps_state[2] * 2 + caps_state[1] * 3 + caps_state[3] * 4;
  DEBUG_PRINTLN(tot);
  checkConnections(200,10);
   uint16_t trans = mb.writeHreg(remote, (ADD-1), tot);
    while (mb.isTransaction(trans)) {
      mb.task();
      delay(10);
    }

    while(true){
      uint16_t res2;
      uint16_t ttrans = mb.readHreg(remote, (ADD-1),&res2, 1);
      while (mb.isTransaction(ttrans)) {
        mb.task();
        delay(5);
      }
      // DEBUG_PRINTLN(res2);

      if(res2 != tot){
        DEBUG_PRINTLN("not sended Try again");
        checkConnections(1200,250);
        trans = mb.writeHreg(remote, (ADD-1), tot);
        while (mb.isTransaction(trans)) {
          mb.task();
          delay(10);
        }
      }else{
        DEBUG_PRINTLN("sended");
        break;
      }
    }
}

void setAction(int index){
    DEBUG_PRINT("Click on Button number : ");
    DEBUG_PRINT(caps[index]);
  while(!readInput2(index)){
    DEBUG_PRINT(".");
    delay(DELAY);
  }
  delay(DELAY);
  while(!readInput2(index)){
    DEBUG_PRINT(".");
    delay(DELAY);
  }
  colorsOff();
  digitalWrite(colors[index],1);
    #ifdef NEW
  digitalWrite(Green,1);
  #else
  digitalWrite(Green,0);
  #endif
  caps_state[index] = 1;
  EEPROM.write(index, 1);
  EEPROM.commit();
  sendData();
  delay(100);
  DEBUG_PRINTLN("wait for cancling alarm");
  /*
  while(readInput2(index)){
    DEBUG_PRINT("2");
    delay(DELAY);
  }
  delay(DELAY);
  while(readInput2(index)){
    DEBUG_PRINT("2");
    delay(DELAY);
  }

*/
  lastSend = millis();
  while(true){
    if(!getButton2(index)){
      break;
    }else if((millis() - lastSend) > sendDelay) {
      sendData2();
      lastSend = millis();
    }
  }
  DEBUG_PRINT("Click off Button number : ");
  DEBUG_PRINT(caps[index]);
 while(!readInput2(index)){
    DEBUG_PRINT(".");
    delay(DELAY);
  }
  delay(DELAY);
  while(!readInput2(index)){
    DEBUG_PRINT(".");
    delay(DELAY);
  }
  colorsOff();
  sendData();
  EEPROM.write(index, 0);
  EEPROM.commit();
  delay(100);
  lastSend = millis();

}

void colorsOff(){
  for(int i =0;i<4;i++){
    digitalWrite(colors[i],0);
     caps_state[i] = 0;
  }
    #ifdef NEW
  digitalWrite(Green,0);
  #else
  digitalWrite(Green,1);
  #endif

}

// check the wifi and modbus connections
void checkConnections(int delayWifi,int delayMb) {
  if (WiFi.status() != WL_CONNECTED) {
    DEBUG_PRINTLN("Wifi not connect");
    WiFi.begin(ssid, password);
    int a = 0;
    while (WiFi.status() != WL_CONNECTED) {
      delay(100);
      DEBUG_PRINT(" :w ");
      a++;
      if (a >= delayWifi) {
         if(delayWifi < 5){
          return;
        }
        ESP.restart();
      }
    }
  }

  if (!mb.isConnected(remote)) {
    DEBUG_PRINTLN("Modbus not connect");
    int b = 0;
    while (!mb.isConnected(remote)) {
      mb.connect(remote);
      DEBUG_PRINT(" :m ");
      delay(100);
      b++;
      if (b >= delayMb) {
        if(delayMb < 5){
          return;
        }
        ESP.restart();
      }
    }

  }
}
