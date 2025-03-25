#include <WiFi.h>
#include <HTTPClient.h>
#include <time.h>
#include <WiFiClientSecure.h> 
#include <PubSubClient.h>
#include <ArduinoJson.h>

#define TRIG_PIN 5  // Trigger pin
#define ECHO_PIN 18 // Echo pin
#define AWS_IOT_ENDPOINT "a38ninjga0qc4o-ats.iot.eu-north-1.amazonaws.com"  // Replace with your AWS IoT endpoint
#define CLIENT_ID "WaterLevelSensor"
#define TOPIC "water/level"

// WiFi Credentials
const char* ssid = "OPPO F23 5G";
const char* password = "arjunhotspot";
const char* server_url = "https://us-east-1-1.aws.cloud2.influxdata.com/api/v2/write?org=iot-project&bucket=cloud_project&precision=s";
const char* sensor_id = "sensor_1";
const char* token = "E43_Y6JwUS-1bhvLv6Mk08bng4y8rJdRKP-FDXfY1vPQ8bzvwD0x7bQzjpGwE8FGSR8CM-ARSh3XPoqaN2FF5g==";

// NTP Server for UTC Time
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 0; // UTC Time
const int daylightOffset_sec = 0;

const char* AWS_CERT_CA = "-----BEGIN CERTIFICATE-----\n"
"MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF\n"
"ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6\n"
"b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL\n"
"MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv\n"
"b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj\n"
"ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM\n"
"9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw\n"
"IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6\n"
"VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L\n"
"93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm\n"
"jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC\n"
"AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA\n"
"A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI\n"
"U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs\n"
"N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv\n"
"o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU\n"
"5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy\n"
"rqXRfboQnoZsG4q5WTP468SQvvG5\n"
"-----END CERTIFICATE-----";

const char* AWS_CERT_CRT = "-----BEGIN CERTIFICATE-----\n"
"MIIDWTCCAkGgAwIBAgIUGYu5imLSMxpOkuHmA5OrQuSA9PgwDQYJKoZIhvcNAQEL\n"
"BQAwTTFLMEkGA1UECwxCQW1hem9uIFdlYiBTZXJ2aWNlcyBPPUFtYXpvbi5jb20g\n"
"SW5jLiBMPVNlYXR0bGUgU1Q9V2FzaGluZ3RvbiBDPVVTMB4XDTI1MDMyNDA5MDUx\n"
"OVoXDTQ5MTIzMTIzNTk1OVowHjEcMBoGA1UEAwwTQVdTIElvVCBDZXJ0aWZpY2F0\n"
"ZTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAMiLMPm1p7zTi5eohA6X\n"
"DO/y2NS2RlzLmILFE/nlZPRc/gJ6T7hrc0UT6ObP6YrlXCgFZZdgaKdRaxiLJTjy\n"
"NFqMGoL9j0DvOs2BxElxdpDSsm6chZgdmvTKs7KaEoEQTOnYIcBqIqrk/87BfSEo\n"
"+BcnqhUeKGMigLyjqFt+3iTBU5l2fL3FYZzbQF4QDQgbXsEFa+24cPNoOfBKHgDy\n"
"A/X/o4+MW3yIZ/aGZSpOnj8TYUOwOroz/sF5qCnEQSt23zeGpRHWxypmwsE5hhCt\n"
"VG9wp/FKZM8eVk/QvkdC6aa8Nj/gwErLHp5JmfAp0QiTCw4whT7NUASeZTPr+kdR\n"
"alkCAwEAAaNgMF4wHwYDVR0jBBgwFoAUA+GnIjYKxJB2744SmO1WPg74HkwwHQYD\n"
"VR0OBBYEFGMo3HGq4Ac+igBc0SzEYPDg/8xsMAwGA1UdEwEB/wQCMAAwDgYDVR0P\n"
"AQH/BAQDAgeAMA0GCSqGSIb3DQEBCwUAA4IBAQBYPzdEBDC9G0dva5bSe+YVQQKh\n"
"n8lnR9mi3nUvBZwg99eSlFIaq/dRoBE/2wXA8zDlaqT9/7LIN+JoPUX+wK11kt+T\n"
"EthivmUHv83hNfavn/dJLXtmeP89z2HydT3cdNO6OKjAfyAVPWwB5QTQtrspaKlm\n"
"ul9ty8s617O/9jhKi6JgHr6tydtFbY/1gqSGr4KDyCMChAkQySrFEyEGqL9IEmJp\n"
"b88mcBfofUWK5RMLQMDcDRHAHQFZvynnWkZw3KrItbR3s21occ3p1Wg9+bKIDrcl\n"
"FwW8lNEb64Q5U2j7BHdDR8SQ+8mD3+Anl/wrUNOFcFPd+hLm7MLlz8jMMSCW\n"
"-----END CERTIFICATE-----";


const char* AWS_CERT_PRIVATE = "-----BEGIN RSA PRIVATE KEY-----\n"
"MIIEpAIBAAKCAQEAyIsw+bWnvNOLl6iEDpcM7/LY1LZGXMuYgsUT+eVk9Fz+AnpP\n"
"uGtzRRPo5s/piuVcKAVll2Bop1FrGIslOPI0Wowagv2PQO86zYHESXF2kNKybpyF\n"
"mB2a9MqzspoSgRBM6dghwGoiquT/zsF9ISj4FyeqFR4oYyKAvKOoW37eJMFTmXZ8\n"
"vcVhnNtAXhANCBtewQVr7bhw82g58EoeAPID9f+jj4xbfIhn9oZlKk6ePxNhQ7A6\n"
"ujP+wXmoKcRBK3bfN4alEdbHKmbCwTmGEK1Ub3Cn8Upkzx5WT9C+R0Lpprw2P+DA\n"
"SssenkmZ8CnRCJMLDjCFPs1QBJ5lM+v6R1FqWQIDAQABAoIBAGjD9nDtSlSl+w3e\n"
"diadGKAdsNLm6jk+D015oWbyRNw33Gb0+MKyN4TF0u0JfssV6R/rOdhsACAws48d\n"
"2fM27MzE1nDJ/9V/ZKvJd/6aLMOZnqeUU2JVTQdHAbtzK+mK13NueqtnhvbaLR0Z\n"
"DcKOAZUvHnZry9OGSdTCdohd7bUq8JogEiTUsjfZGLt4yPmB88ARq7ngWm9bEm1O\n"
"5vIHrrN481ECGtHJn2lwEDylIi+DSUYSayDaNckjS4/+ImBFSWqvUPbXS9errKUu\n"
"5bMlw7f+z+Ya6tM/s67Xv/+t5b/MHWzCQoPYmpLHK/74t2l23gqcU4Msb8sNf6l7\n"
"ugBXPoECgYEA9AudNy8Uu1LCLQ06I1QSs9Vxl/n3UOH5aVzYk6ReRdF0uA3z8den\n"
"2PFNX9dQyxTZTvDqsHD9s00hO3LUCpnlBra515m2yjGFCX7S8bKtAStrkw+DYaq9\n"
"cUsWKpvYPB1k4mrlcUXSYXxiSNYMPTu/LNO69GFnn8yDWql8plXCM5ECgYEA0l4O\n"
"axRD+1MZJ0LL/qMyD1zNi/MBjgQv0firKoLAYG+pgO7GjmsNTwtUAFTffaKGg3Wj\n"
"+wi13/7oDuUXJvIPJwmN3m8c5RrctiZfK6HoE9CxkYlWXg6nLJZHTPYMXv9PtNnK\n"
"u/C45QGHYJvdFx9WYWJJu6soXgpCmwwJLVC+VkkCgYApaYMfJyUceHmszW7NKeoW\n"
"jCS/hlJabk41e+TggObicB2fUfbQVikMXj4kAlSacYpCpA0fLb1PXMpD3UNwHRdJ\n"
"IV694D6tZuPNbbD8BLbo2OZjSSGhRCjrfbNgPZve75n79enDY/bERigjdfcTuPzC\n"
"7Zbu+0xrSHgNvW8ChtDykQKBgQCdnnuPH5AQLyMZl6LhF8XaXN5ljZ4MmnZ6DKez\n"
"ImSh3QhT4pRDAVLndsVpui36JFxFA0/qN4yOWJ/nCR9fvt47kZ0KnJY50vasQ/Pq\n"
"7VTGBAgH068yqif38GFb+L8sLn+6MbUFPEhMzehxKldti71dGhfE1WlZjLZ7FXmK\n"
"yoTSgQKBgQC+mVj6LCCO/MO5bSYwvaUpgTIE/990pvAFsdVemzEmkkcmthONQU6d\n"
"Jj4EEvImOEPYRaW60iINxUnfyN5IVwDM6g40Gd7hh5APNG6wnD1h4ULi/nD88Edt\n"
"dzGrk9UotKPUVQuq/xVeLVZYl6tF99W2QxvUnxFqDlKgoZOWZNBwZw==\n"
"-----END RSA PRIVATE KEY-----";

WiFiClientSecure net;
PubSubClient client(net);

void connectAWS() {
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);

  client.setServer(AWS_IOT_ENDPOINT, 8883);

  while (!client.connected()) {
    Serial.println("Connecting to AWS IoT...");
    if (client.connect(CLIENT_ID)) {
      Serial.println("Connected to AWS!");
    } else {
      Serial.print("Failed, retrying...");
      delay(5000);
    }
  }
}

float getWaterLevel() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  float duration = pulseIn(ECHO_PIN, HIGH, 30000);  
  if (duration == 0) {
    Serial.println("Timeout - No echo detected");
    return -1.0;  
  }

  float distance = (duration * 0.034) / 2;  
  return distance;
}

void sendToAWS(float waterLevel) {
  if (!client.connected()) {
    connectAWS();
  }

  StaticJsonDocument<256> doc;
  doc["device_id"] = "ESP32_01";
  doc["water_level"] = waterLevel;
  doc["timestamp"] = millis();

  char jsonBuffer[256];
  serializeJson(doc, jsonBuffer);

  client.publish(TOPIC, jsonBuffer);
  Serial.println("Data Sent: " + String(jsonBuffer));
}

float getDistance() {
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    long duration = pulseIn(ECHO_PIN, HIGH, 30000); // 30ms timeout
    if (duration == 0) return -1; // No echo received
    return (duration * 0.0343) / 2; // Convert to cm
}

float calculateWaterLevel(float distance) {
    float D1 = 2.32, L1 = 3.5; // Full water level
    float D2 = 22.23, L2 = 0.0; // Empty tank

    float m = (L2 - L1) / (D2 - D1);
    float b = L1 - (m * D1);

    float water_level = (m * distance) + b;

    return water_level;
}

long long getTimestamp() {
    return time(nullptr); // Convert seconds to nanoseconds
}


void sendData(float distance, float litres, String sensor_id, long long timestamp) {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        http.begin(server_url);
        http.addHeader("Content-Type", "text/plain");
        http.addHeader("Authorization", String("Token ") + token);

        // Format data for InfluxDB Line Protocol
        String data = "water_level,sensor=" + sensor_id + 
                      " distance=" + String(distance) + 
                      ",litres=" + String(litres) + 
                      " " + String(timestamp);

        Serial.println("Sending data: " + data);
        int httpResponseCode = http.POST(data);
        if (httpResponseCode > 0) {
            Serial.print("HTTP Response Code: ");
            Serial.println(httpResponseCode);
            Serial.println(http.getString()); // Print response from server
        } else {
            Serial.print("Error sending data: ");
            Serial.println(httpResponseCode);
        }
        http.end();
    } else {
        Serial.println("WiFi not connected, unable to send data.");
    }
}


void setup() {
    Serial.begin(115200);
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);
    
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");

    connectAWS();
    // Configure time using NTP
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}

void loop() {
    float total_distance = 0;
    int readings = 0;

    for (int i = 0; i < 10; i++) { 
        float distance_cm = getDistance();
        if (distance_cm != -1) { // Ignore invalid readings
            total_distance += distance_cm;
            readings++;
        }
        delay(1000); 
    }

    if (readings > 0) {
        float avg_distance = total_distance / readings;
        float water_level = calculateWaterLevel(avg_distance);
        long long timestamp = getTimestamp();

        Serial.print("Average Distance: ");
        Serial.print(avg_distance);
        Serial.print(" cm, Water Level: ");
        Serial.print(water_level);
        Serial.println(" L");

        sendData(avg_distance, water_level, sensor_id, timestamp);
        sendToAWS(avg_distance);
    } else {
        Serial.println("No valid distance readings.");
    }

    client.loop();
}


