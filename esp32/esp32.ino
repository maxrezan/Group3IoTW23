#include <string.h>
#include "BLEDevice.h"
#include <WiFi.h>
#include <PubSubClient.h>

#define N_DEVICES 3
#define DELAY_TIME 1000
#define MAX_DELAY_TIME 5000
#define SERIAL_DEBUG_BAUD    115200 // Baud rate for debug serial
#define MQTT_CLIENT_ID "esp32_v2"
#define MQTT_TOPIC "v1/devices/me/telemetry"
#define MQTT_USERNAME "u_esp32_v2"
#define MQTT_PASSWORD "p_esp32v2"
#define SSID  "max_a53"
#define password  "mehanikk"
#define MQTT_SERVER  "192.168.224.84"
#define MQTT_PORT  1883

#define MAC1 "d3:1f:b4:68:69:47"
#define MAC2 "eb:fa:98:57:eb:c7"
#define MAC3 "c8:b8:8e:67:81:53"

// The remote service we wish to connect to.
static BLEUUID serviceUUID("00001809-0000-1000-8000-00805f9b34fb");
// The characteristic of the remote service we are interested in
static BLEUUID charUUID("00002a1e-0000-1000-8000-00805f9b34fb");

// Whether to connect to BLE devices
static boolean do_connect = false;
// Count of found devices
static int count_devices = 0;

// Thingys mac addresses
static const char * mac_addresses[] = {MAC1, MAC2, MAC3};

// Device representation of the thingys
static BLEAdvertisedDevice * bt_devcs[] = {NULL, NULL, NULL};
// Data received
static float data_received[] = { 0., 0., 0. };

WiFiClient espClient;
static PubSubClient client(espClient);

boolean connect() {
    Serial.println("Connecting");
    return client.connect(MQTT_CLIENT_ID, MQTT_USERNAME, MQTT_PASSWORD);
    Serial.println("Connection done");

}
int publishMessage(const char *message) {
  if (!client.publish(MQTT_TOPIC, message)) {
    Serial.println(F("Error publishing"));
    return -1;
  }
  return 0;
}

/**
 * @brief Function called after receiving a notification from the bluetooth device 
 * 
 * @param pBLERemoteCharacteristic 
 * @param pData Received data
 * @param length Length of the data
 * @param isNotify Differs the notify from the indicate
 * @param deviceID Device identification
 */
static void notify_callback_func(uint8_t* pData, size_t length, bool isNotify, int deviceID) {
  double temperature, humidity;
  uint32_t timestamp;

  if (length < (sizeof(double) * 2 + sizeof(uint32_t))) return;

  // Copy the bytes from the array to the variables
  memcpy(&timestamp, &pData[0], sizeof(uint32_t));
  memcpy(&temperature, &pData[sizeof(uint32_t)], sizeof(double));
  memcpy(&humidity, &pData[sizeof(uint32_t) + sizeof(double)], sizeof(double));


  String result = "{timestamp" + String(deviceID) + ": " + String(timestamp) + ", temperature" + String(deviceID) + ": "  + String(temperature) + ", humidity" + String(deviceID) + ": " + String(humidity) + "}";
  if (publishMessage(result.c_str())) {
    return;
  }
  Serial.print("Sent: ");
  Serial.println(result.c_str());
};

static void notify_callback0(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
  notify_callback_func(pData, length, isNotify, 0);
}

static void notify_callback1(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
  notify_callback_func(pData, length, isNotify, 1);
}

static void notify_callback2(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
  notify_callback_func(pData, length, isNotify, 2);
}

/**
 * @brief type of the parameteres of the notify callback function
 * 
 */
typedef void (*notify_callback_type)(
  BLERemoteCharacteristic*,
  uint8_t*,
  size_t,
  bool
);

/**
 * @brief Array with the different callback functions for the different thingys
 * 
 */
// TODO: verify if we need different callbacks for the different thingys
static notify_callback_type notifyCallbackFunctions[] = {
  notify_callback0,
  notify_callback1,
  notify_callback2,
};

/**
 * @brief Callbacks for the BLE client
 * 
 */
class MyClientCallback : public BLEClientCallbacks {
  /**
   * @brief Function called after establishing a connection with a client
   * 
   * @param pclient Client that the connection has been established
   */
  void onConnect(BLEClient* pclient) {
    Serial.print(F("BLE Connected: "));
    Serial.println(pclient->toString().c_str());
  }

  /**
   * @brief Function called after disconnecting the connection with a client 
   * 
   * @param pclient Disconnected client
   */
  void onDisconnect(BLEClient* pclient) {
    Serial.print(F("BLE Disconnected: "));
    Serial.println(pclient->getPeerAddress().toString().c_str());
  }
};

/**
* Forms connection to BLE from thingys
*/
bool connect_ble_device(BLEAdvertisedDevice * myDevice, int clientId) {
  Serial.print(F("Forming a connection to "));
  Serial.println(myDevice->getAddress().toString().c_str());

  BLEClient * pClient = BLEDevice::createClient();
  pClient->setClientCallbacks(new MyClientCallback());

  pClient->connect(myDevice);
  pClient->setMTU(517); //set client to request maximum MTU

  // Obtain a reference to the service we are after
  BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
  if (pRemoteService == nullptr) {
    Serial.print(F("Failed to find our service UUID: "));
    Serial.println(serviceUUID.toString().c_str());
    pClient->disconnect();
    return false;
  }

  // Obtain a reference to the characteristic we are after
  BLERemoteCharacteristic * pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
  if (pRemoteCharacteristic == nullptr) {
    Serial.print(F("Failed to find our characteristic UUID: "));
    Serial.println(charUUID.toString().c_str());
    pClient->disconnect();
    return false;
  }

  if(pRemoteCharacteristic->canNotify()) {
    pRemoteCharacteristic->registerForNotify(notifyCallbackFunctions[clientId]);
  }

  return true;
}

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
  /**
   * @brief Called for each advertising BLE device
   * 
   * @param advertisedDevice advertised device found
   */
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    Serial.print(F("BLE Advertised Device found: "));
    Serial.println(advertisedDevice.getAddress().toString().c_str());

    for (int i = 0; i < N_DEVICES; i++) {
      if (strcmp(mac_addresses[i], advertisedDevice.getAddress().toString().c_str()) == 0
          && advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceUUID)) {
        bt_devcs[i] = new BLEAdvertisedDevice(advertisedDevice);
        count_devices++;
        break;
      }
    }

    if (count_devices == N_DEVICES) {
      Serial.println(F("Got all devices; Stopping scan and starting connect."));
      BLEDevice::getScan()->stop();
      do_connect = true;
    }
  }
};

void wifi_mqtt_setup() {
  // Configurations to connect to the ThingsBoard

  // Connect to Wi-Fi
  WiFi.begin(SSID, password);
  Serial.print(F("Connecting to WiFi"));
  while (WiFi.status() != WL_CONNECTED) {
    delay(DELAY_TIME);
    Serial.print(".");
  }
  Serial.println(F("\nConnected to WiFi"));

  // Connect to MQTT broker with username and password
  client.setServer(MQTT_SERVER, MQTT_PORT);

  while (!client.connected()) {
    if (connect()) {
      Serial.println(F("Connected to MQTT broker"));
      client.subscribe(MQTT_TOPIC);
    } else {
      delay(MAX_DELAY_TIME);
    }
  }

}

void bluetooth_setup() {
  BLEDevice::init("");

  // Retrieve a Scanner and set the callback we want to use to be informed when we
  // have detected a new device. Specify that we want active scanning and start the
  // scan to run for 5 seconds.
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(5, false);
}

void setup() {
  Serial.begin(SERIAL_DEBUG_BAUD);

  wifi_mqtt_setup();
  bluetooth_setup();
}

void loop() {
  if (do_connect == true) {
    for (int i = 0; i < N_DEVICES; i++) {
      if (!(bt_devcs[i] != NULL && connect_ble_device(bt_devcs[i], i))) {
        Serial.print(F("Failed to connect to the bt device "));
        Serial.println(i);
      }
    }
    do_connect = false;
  }

  if (!client.connected()) {
    connect();
  }
  client.loop();
  delay(DELAY_TIME);
}

// TODO: comment code
