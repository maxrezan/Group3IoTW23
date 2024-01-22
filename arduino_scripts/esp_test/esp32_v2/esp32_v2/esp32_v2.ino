#include <WiFi.h>
#include <PubSubClient.h>

char *ssid = "max_a53";
char *password = "mehanikk";
char *mqtt_server = "192.168.224.84";
int mqtt_port = 1883;

char *mqtt_client_id = "esp32_v2";
char *mqtt_topic = "v1/devices/me/telemetry"; // Adjust this topic based on your ThingsBoard configuration
char *mqtt_username = "u_esp32_v2";
char *mqtt_password = "p_esp32v2";
// char *mqtt_password = "da";


WiFiClient espClient;
PubSubClient client(espClient);

unsigned long lastMsg = 0;

void setup() {
  Serial.begin(921600);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Connect to MQTT broker
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);  // Set callback function for incoming MQTT messages

  while (!client.connected()) {
    Serial.println("Connecting to MQTT broker...");
    if (client.connect(mqtt_client_id, mqtt_username, mqtt_password)) {
      Serial.println("Connected to MQTT broker");
      client.subscribe(mqtt_topic);
    } else {
      Serial.print("Failed, rc=");
      Serial.print(client.state());
      Serial.println(" Retrying in 5 seconds...");
      delay(5000);
    }
  }
}

void loop() {
  if (!client.connected()) {
    // reconnect();
    client.connect(mqtt_client_id, mqtt_username, mqtt_password);
    // Serial.println("Connected to MQTT broker");
  }
  if (millis() - lastMsg > 1000) {
    lastMsg = millis();
    publishMessage("{hi:Tiago}");
  }
  client.loop();
}

void callback(char *topic, byte *payload, unsigned int length) {
  Serial.print("Received message on topic: ");
  Serial.println(topic);

  Serial.print("Message: ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void reconnect() {
  while (!client.connected()) {
    Serial.println("Reconnecting to MQTT broker...");
    if (client.connect(mqtt_client_id)) {
      Serial.println("Connected to MQTT broker");
      client.subscribe(mqtt_topic);
    } else {
      Serial.print("Failed, rc=");
      Serial.print(client.state());
      Serial.println(" Retrying in 5 seconds...");
      delay(5000);
    }
  }
}
void publishMessage(const char *message) {
  Serial.println("Publishing message...");
  if (client.publish(mqtt_topic, message)) {
    Serial.println("Message published successfully");
  } else {
    Serial.println("Failed to publish message");
  }
}
