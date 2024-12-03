#include <ESP8266WiFi.h>
// #include <WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

//Wifi name
#define WLAN_SSID       "nhatvu"
//Wifi password
#define WLAN_PASS       "25122003"

//setup Adafruit
#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883
//fill your username                   
#define AIO_USERNAME    "abcdphamvau124"
//fill your key
#define AIO_KEY         "aio_cpiB54I0PzEEPt9acT0xIw8qQjjv"

//UART Command processing
#define TEMP_COMMAND_LEN 5
#define START_COMMAND_SYMBOL '!'
#define END_COMMAND_SYMBOL '#'

#define INITIALIZING    0
#define IDLE            1
#define COMMAND_STARTED 2
#define PAYLOAD_STARTED 3

char temp_command[] = "TEMP:";
int command_cnt = 0;
bool command_started = false;
bool payload_started = false;
char temp_value[20];
int payload_cnt = 0;

//setup MQTT
WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

//setup publish
Adafruit_MQTT_Publish temp_pub = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/temperature");

int led_counter = 0;
int led_status = HIGH;

int system_status = INITIALIZING;

void setup() {
  // put your setup code here, to run once:
  //set pin 2,5 as OUTPUT
  pinMode(2, OUTPUT);
  pinMode(5, OUTPUT);
  //set busy pin HIGH
  digitalWrite(5, HIGH);

  Serial.begin(115200);

  //connect Wifi
  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  //connect MQTT
  while (mqtt.connect() != 0) { 
    mqtt.disconnect();
    delay(500);
  }

  //finish setup, set busy pin LOW
  digitalWrite(5, LOW);

  system_status = IDLE;
}

void loop() {
  
  //read serial
  if(Serial.available()) {
    int msg = Serial.read();
    switch (system_status) {
      case IDLE:
        if (START_COMMAND_SYMBOL == msg) {
          system_status = COMMAND_STARTED;
          command_cnt = 0;
        }
        break;
      case COMMAND_STARTED:
        if (msg != temp_command[command_cnt]) {
          system_status = IDLE;
        } else {
          command_cnt++;
          if (TEMP_COMMAND_LEN == command_cnt) {
            system_status = PAYLOAD_STARTED;
            payload_cnt = 0;
          }
        }
        break;
      case PAYLOAD_STARTED:
        if (END_COMMAND_SYMBOL == msg) {
          if (payload_cnt > 0) {
            temp_value[payload_cnt] = '\0';
            temp_pub.publish(temp_value);
          }
          system_status = IDLE;
        } else {
          temp_value[payload_cnt] = msg;
          payload_cnt++;
        }
        break;
    }
  }

  led_counter++;
  if(led_counter == 100){
    // every 1s
    led_counter = 0;
    //toggle LED
    if(led_status == HIGH) led_status = LOW;
    else led_status = HIGH;

    digitalWrite(2, led_status);
  }

  delay(10);
}
