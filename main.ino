#include<DHT.h>
#include<WiFi.h>
#include<PubSubClient.h>
#include <Wire.h>
#include <Adafruit_BMP280.h>


const char* ssid = "mrsh77";
const char* pass = "1m77n2299215r77#";

const char* mqtt_server = "192.168.0.102";

#define DHTPIN 14

#define DHTTYPE DHT11

DHT dht(DHTPIN,DHTTYPE);

WiFiClient espClient;
PubSubClient client(espClient);

Adafruit_BMP280 bmp;
Adafruit_Sensor *bmp_temp = bmp.getTemperatureSensor();
Adafruit_Sensor *bmp_pressure = bmp.getPressureSensor();

void setup_wifi(){
  delay(10);
  Serial.println();
  Serial.print("connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid,pass);
  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("WiFi connected - ESP IP address: ");
  Serial.println(WiFi.localIP());
  
}

void callback(String topic, byte* message , unsigned int length){
  Serial.print("message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
}

void reconnect(){
  while(!client.connected()){
    Serial.print("attempting MQTT connection...");
    if(!client.connect("ESP8266Client")){
      Serial.println("connected");
    }
    else{
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println("try again in 5 seconds");
      delay(5000);
      
    }
  }
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server,1883);
  client.setCallback(callback);

  Serial.println(F("BMP280 Sensor event test"));
  
  Serial.println(F("DHT11 test!"));
  dht.begin();
  
  if (!bmp.begin()) {
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring or "
                      "try a different address!"));
    while (1) delay(10);
  }
    /* Default settings from datasheet. */
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */

  bmp_temp->printSensorDetails();

}

void loop() {
  if(!client.connected()){
    reconnect();
  }
  if(!client.loop())
    client.connect("ESP8266Client");

  float Humidity = dht.readHumidity();
  float temp = dht.readTemperature();

  float altitude_ = bmp.readAltitude(1013.25);
  sensors_event_t temp_event, pressure_event;
  //bmp_temp->getEvent(&temp_event);
  bmp_pressure->getEvent(&pressure_event);
  
  if (isnan(Humidity) || isnan(temp)){
    Serial.println(F("Failed to read from DHT sensor"));
    return;
  }
  char tempString[8];
  dtostrf(temp, 1, 2, tempString);
  
  char humString[8];
  dtostrf(Humidity, 1, 2, humString);

  char pressureString[8];
  dtostrf(pressure_event.pressure, 1, 2, pressureString);
/*
  Serial.print(F("Humidity: "));
  Serial.print(Humidity);
  Serial.print(F("%  Temperature: "));
  Serial.print(temp);
*/


  client.publish("ESP32/temp",tempString);
  client.publish("ESP32/hum",humString);
  client.publish("ESP32/pressure", pressureString);
  Serial.println(pressureString);
  delay(5000);

}
