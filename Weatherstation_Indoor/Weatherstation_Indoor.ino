/*
MQTT: https://randomnerdtutorials.com/esp32-mqtt-publish-subscribe-arduino-ide/
BME280: https://lastminuteengineers.com/bme280-esp8266-weather-station/
MQ135:  
https://www.hackster.io/aktaktaw/simple-iot-human-counter-in-premises-c8e100
https://github.com/GeorgK/MQ135
*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <MQ135.h>

// BME 280
#define SEALEVELPRESSURE_HPA (1013.25)
Adafruit_BME280 bme;
float temperature, humidity, pressure, altitude;

// MQ 135
#define ANALOGPIN A0
#define RZERO 206.85
float rzero, ppm;
MQ135 gasSensor = MQ135(ANALOGPIN);

// Network
WiFiClient espClient;
PubSubClient client(espClient);

long lastMsg = 0;

// Settings
const char *SSID = "";
const char *PSK = "";
const char *MQTT_BROKER = "";
const char *MQTT_USER = "";
const char *MQTT_PWD = "";
const char *HOSTNAME = "WeatherstationESP";
String BASE_TOPIC = String("/weatherstation/indoor");

void setup()
{
    Serial.begin(9600);
    delay(100);
    setup_wifi();
    client.setServer(MQTT_BROKER, 1883);

    // BME 280
    bme.begin(0x76);
    // MQ135
    rzero = gasSensor.getRZero();
    delay(3000);
    Serial.print("MQ135 RZERO Calibration Value: ");
    Serial.println(rzero);
}

void setup_wifi()
{
    WiFi.hostname(HOSTNAME);
    WiFi.begin(SSID, PSK);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(100);
    }
    Serial.print("\nConntected with IP-Addr.: ");
    Serial.println(WiFi.localIP());
}

void loop()
{
    if (!client.connected())
    {
        while (!client.connected())
        {
            client.connect(HOSTNAME, MQTT_USER, MQTT_PWD, String(BASE_TOPIC + "/LWT").c_str(), 0, true, "offline");
            delay(100);
        }
    }
    client.loop();

    long now = millis();
    if (now - lastMsg > 300000)
    {
        client.publish(String(BASE_TOPIC + "/LWT").c_str(), "online");
        publish_bme_measures();
        publish_mq135_measure();
        lastMsg = now;
    }
}

void publish_mq135_measure()
{
    ppm = gasSensor.getPPM();
    if (isnan(ppm))
    {
        Serial.println("Failed to read MQ135 Sensor");
        client.publish("/weatherstation/indoor/airquality", "99");
    }
    static char ppmTemp[7];
    dtostrf(ppm, 6, 2, ppmTemp);
    client.publish("/weatherstation/indoor/airquality", ppmTemp);

    Serial.println("CO2 ppm value: ");
    Serial.println(ppm);
}

void publish_bme_measures()
{
    temperature = bme.readTemperature();
    humidity = bme.readHumidity();
    pressure = bme.readPressure() / 100.0F;
    altitude = bme.readAltitude(SEALEVELPRESSURE_HPA);

    client.publish(String(BASE_TOPIC + "/temperature").c_str(), String(temperature).c_str());
    client.publish(String(BASE_TOPIC + "/humidity").c_str(), String(humidity).c_str());
    client.publish(String(BASE_TOPIC + "/pressure").c_str(), String(pressure).c_str());
}
