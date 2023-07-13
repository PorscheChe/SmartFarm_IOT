#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

#define WIFI_SSID "Wishchai_2.4G"
#define WIFI_PASSWORD "0619916262"

#define API_KEY "AIzaSyCBg0vQvQ_KvpmFXh26MbqnpvM1j3AZVRE"
String DATABASE_URL = "https://smart-farm-iot-7a9f1-default-rtdb.asia-southeast1.firebasedatabase.app/";
const int potPin = 34;// Analog pin
#define ledPin25  12
#define ledPin50  27
#define ledPin75  26
#define ledPin100 33
#define treshold1  1241 //about 30%
#define treshold2  2482 //aboiut 60%
#define treshold3  3723 //about 90%
#define tresholdMax 4090 //about 99-100%

FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;
bool npk15Val;
bool npk46Val;
bool pot1Val;
unsigned long sendDataPrevMillis = 0;
int count = 0;
bool signupOK = false;

int potValue;// do not change
float voltage =0;// do not change
float moistPercent;
int seconds;
int minutes;
int hours;
int days ;
int daycal;
int getnpk46;
int getnpk15;
int dayXHour;
bool autoVal;
int dayXMin;
void setup() {
  // initialize serial communication at 115200 bits per second:
  Serial.begin(115200);
  pinMode(ledPin25, OUTPUT);
  pinMode(ledPin50, OUTPUT);
  pinMode(ledPin75, OUTPUT);
  pinMode(ledPin100, OUTPUT);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;
  if (Firebase.signUp(&config, &auth, "", "")){
  Serial.println("ok");
  signupOK = true;
  }
  else{
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

   /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
  
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}


void loop() {
  int time = millis();
  delay(100);
  seconds = time/1000;
  minutes = seconds/60;
  hours = minutes/60;
  days = hours/24;
  daycal = days+1;
  Serial.print("Days : ");
  Serial.print(days);
  Serial.print(" ;");
  Serial.print(hours);
  Serial.print(":");
  Serial.print(minutes);
  Serial.print(":");
  Serial.println(seconds);
        
  //For Writing Data
  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0)){
    sendDataPrevMillis = millis();
    potValue = analogRead(potPin);
    voltage = (3.3/4095.0) * potValue;
    Serial.print("potValue:");
    Serial.print(potValue);
    moistPercent= (potValue/4095.0) * 100;
    Serial.print(" moistPercent = ");
    Serial.print(moistPercent);
    Serial.print(" Voltage:");
    Serial.print(voltage);
    Serial.println("V");  
    
    // delay in between reads for stability
    if (Firebase.RTDB.setInt(&fbdo, "Monitor/Humidity", moistPercent)){
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
    //get Manual for Npk15
    if (Firebase.RTDB.getBool(&fbdo, "/manual/npk15")) {
      Serial.println("CHECK READ DATA TYPE : " +  fbdo.dataType());
      if (fbdo.dataType() == "boolean") {
        npk15Val = fbdo.boolData();
        Serial.println("NPK15 VALUE : " + npk15Val);
      }
    }
    else {
      Serial.println(fbdo.errorReason());
    }
    //Get Manual for npk46
    if (Firebase.RTDB.getBool(&fbdo, "/manual/npk46")) {
      if (fbdo.dataType() == "boolean") {
        npk46Val = fbdo.boolData();
        Serial.println("NPK46 VALUE : " + npk46Val);
      }
    }
    else {
      Serial.println(fbdo.errorReason());
    }
    
    if (Firebase.RTDB.getBool(&fbdo, "/manual/pot1")) {
      if (fbdo.dataType() == "boolean") {
        pot1Val = fbdo.boolData();
        Serial.println("POT1 VALUE : " + pot1Val);
      }
    }
    else {
      Serial.println(fbdo.errorReason());
    }
    String line = "/Day";
    String slash1 = "/Hour";
    String comb = line+daycal+slash1;
    String slash = "/Min";
    String comb2 = line+daycal+slash;
    String slash3 = "/NPK15";
    String slash4 = "/NPK46";
    String comb3 = line+daycal+slash3;
    String comb4 = line+daycal+slash4;
    Serial.println(comb);
    if(Firebase.RTDB.getInt(&fbdo, comb)){
          Serial.println(fbdo.dataType());
          Serial.println(fbdo.intData());
          dayXHour = fbdo.intData();
    }
    if(Firebase.RTDB.getInt(&fbdo, comb2)){
        Serial.println(fbdo.intData());  
        dayXMin = fbdo.intData();
    }
    if(Firebase.RTDB.getInt(&fbdo, comb3)){
        Serial.println(fbdo.intData());  
        getnpk15 = fbdo.intData()*60;
    }
    if(Firebase.RTDB.getInt(&fbdo, comb4)){
        Serial.println(fbdo.intData());  
        getnpk46 = fbdo.intData()*60;
    }
    if(Firebase.RTDB.getBool(&fbdo, "/auto")){
        Serial.println(fbdo.boolData());
        autoVal = fbdo.boolData();
      }
    
    int combineTime = (getnpk15+getnpk46);
    Serial.println(combineTime);
    if(autoVal){
     if (hours == dayXHour && minutes == dayXMin){
        //if(pot1Val){digitalWrite(ledPin50,HIGH);}else{digitalWrite(ledPin50,LOW);}
        if(getnpk15!=0){
           digitalWrite(ledPin25,HIGH);  
        }else{
          digitalWrite(ledPin25,LOW);
       }
        if(getnpk46!=0){
           digitalWrite(ledPin50,HIGH);  
        }else{
           digitalWrite(ledPin50,LOW);
        }
        delay(combineTime*100);
        digitalWrite(ledPin25,LOW);
        digitalWrite(ledPin50,LOW);
      }
     }else if(pot1Val){
            digitalWrite(ledPin75,HIGH);
              if(npk15Val){
                digitalWrite(ledPin25,HIGH);  
              }else{
                digitalWrite(ledPin25,LOW);
              }
              if(npk46Val){
                  digitalWrite(ledPin50,HIGH);  
              }else{
                 digitalWrite(ledPin50,LOW);
              }
        }else{
           digitalWrite(ledPin75,LOW);
           digitalWrite(ledPin50,LOW);
           digitalWrite(ledPin25,LOW);
       }   
     }
  }


 
