#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const char* ssid = "";
const char* password = "";
const char* mqtt_server = "192.168.1.3";

WiFiClient espClient;
PubSubClient client(espClient);

unsigned long lastMsg = 0;

// Function to generate test data
void generateTestData(float &temp, float &pressure, float &humidity, float &gas) {
  temp = random(200, 350) / 10.0;       // 20.0°C - 35.0°C
  pressure = random(9900, 10200) / 10.0; // 990.0 hPa - 1020.0 hPa
  humidity = random(300, 700) / 10.0;   // 30.0% - 70.0%
  gas = random(100, 500) / 10.0;        // 10.0 - 50.0 kΩ equivalent
}

void setup() {
  Serial.begin(115200);

  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  client.setServer(mqtt_server, 1883);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  StaticJsonDocument<80> doc;
  char output[80];

  long now = millis();
  if (now - lastMsg > 5000) {
    lastMsg = now;

    float temp, pressure, humidity, gas;
    generateTestData(temp, pressure, humidity, gas);

    doc["t"] = temp;
    doc["p"] = pressure;
    doc["h"] = humidity;
    doc["g"] = gas;

    serializeJson(doc, output);
    Serial.println(output);
    client.publish("/home/sensors", output);
  }
}

void reconnect() {
  int attempts = 0;
  while (!client.connected() && attempts < 3) {
    attempts++;
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);

    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" retrying in 5 seconds");
      delay(5000);
    }
  }
  
  if (!client.connected()) {
    Serial.println("Failed to connect to MQTT after multiple attempts. Will try again in next loop.");
    delay(10000); // Wait longer before next attempt
  }
}
