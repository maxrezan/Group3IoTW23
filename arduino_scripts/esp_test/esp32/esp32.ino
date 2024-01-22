#include <WiFi.h>
#include <PubSubClient.h>

char* ssid = "max_a53";
char* password = "mehanikk";
char* mqttServer = "192.168.224.84"; // IP address or hostname of your MQTT broker
int mqttPort = 1883;
char* topic = "testTopic";
// const char* ssid = "max_a53";
// const char* password = "mehanikk";
// const char* mqttServer = "192.168.204.84"; // IP address or hostname of your MQTT broker
// const int mqttPort = 1883;
char* mqttUser = "mqtt_broker";
char* mqttPassword = "mqttbroker";
// const char* topic = "testTopic";

WiFiClient espClient;
PubSubClient client(espClient);

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Topic: ");
  Serial.println(topic);

  Serial.print("Message: ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void setup() {
  Serial.begin(921600);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");

  // Connect to MQTT broker with username and password
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);

  while (!client.connected()) {
    if (client.connect("ESP32Client", mqttUser, mqttPassword)) {
      Serial.println("Connected to MQTT broker");
      client.subscribe(topic);
    } else {
      Serial.print("Failed, rc=");
      Serial.print(client.state());
      Serial.println(" Retrying in 5 seconds...");
      delay(5000);
    }
  }

  // Subscribe to a topic or perform other tasks as needed
}

void loop() {
  client.loop();
  // Perform other tasks in the loop as needed
}
