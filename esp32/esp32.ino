#include <string.h>
#include "BLEDevice.h"

#define N_DEVICES 1

// Baud rate for debug serial
#define SERIAL_DEBUG_BAUD    115200

// The remote service we wish to connect to.
static BLEUUID serviceUUID("00001809-0000-1000-8000-00805f9b34fb");
// The characteristic of the remote service we are interested in.
static BLEUUID    charUUID("00002a1e-0000-1000-8000-00805f9b34fb");

// Whether to connect to BLE devices
static boolean do_connect = false;
// Count of found devices
static int count_devices = 0;

// MAC addresses of our thingys
static const char * mac_addresses[] = {
  "d3:1f:b4:68:69:47",
  "c8:b8:8e:67:81:53"
};
// "C8:B8:8E:67:81:53"

// The device representation of the thingys
static BLEAdvertisedDevice * bt_devcs[] = {
  NULL,
};

// Last data received
static float data_received[] = { 0., };

static void notify_callback0(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
  Serial.printf("Received the following data:\n");
  // Create variables to store the doubles
  double temperature, humidity;

  // Copy the bytes from the array to the first double variable
  memcpy(&temperature, &pData[0], sizeof(double));

  // Copy the bytes from the array to the second double variable
  memcpy(&humidity, &pData[sizeof(double)], sizeof(double));
  Serial.printf("Temperature: %.1fC | Humidity: %.1f %%\n", temperature, humidity);
};

typedef void (*notify_callback_type)(
  BLERemoteCharacteristic*,
  uint8_t*,
  size_t,
  bool
);
static notify_callback_type notifyCallbackFunctions[] = {
  notify_callback0,
};

/**
 * Callback for BLE client
 */
class MyClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient* pclient) {
    Serial.printf("Connected BLE client %s\n", pclient->toString().c_str());
  }

  void onDisconnect(BLEClient* pclient) {
    Serial.printf("Disconnected BLE client %s\n", pclient->toString().c_str());
  }
};


/**
* Forms connection to BLE servers from thingys
*/
bool connect_ble_server(BLEAdvertisedDevice * myDevice, int clientId) {
  Serial.printf("Forming a connection to %s\n", myDevice->getAddress().toString().c_str());

  BLEClient * pClient = BLEDevice::createClient();
  pClient->setClientCallbacks(new MyClientCallback());

  // Connect to the remove BLE Server.
  pClient->connect(myDevice);  // if you pass BLEAdvertisedDevice instead of address, it will be recognized type of peer device address (public or private)
  Serial.println(" - Connected to BLE server");
  pClient->setMTU(517); //set client to request maximum MTU from server (default is 23 otherwise)
  
  // Obtain a reference to the service we are after.
  BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
  if (pRemoteService == nullptr) {
    Serial.print("Failed to find our service UUID: ");
    Serial.println(serviceUUID.toString().c_str());
    pClient->disconnect();
    return false;
  }

  // Obtain a reference to the characteristic we are after.
  BLERemoteCharacteristic * pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
  if (pRemoteCharacteristic == nullptr) {
    Serial.print("Failed to find our characteristic UUID: ");
    Serial.println(charUUID.toString().c_str());
    pClient->disconnect();
    return false;
  }

  if(pRemoteCharacteristic->canNotify())
    pRemoteCharacteristic->registerForNotify(notifyCallbackFunctions[clientId]);

  return true;
}


/**
 * Scan for BLE servers and find out thingys.
 */
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {

 /**
  * Called for each advertising BLE server.
  */
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    Serial.print("BLE Advertised Device found: ");
    Serial.println(advertisedDevice.toString().c_str());

    Serial.printf("Address: %s\n", advertisedDevice.getAddress().toString().c_str());

    for (int i = 0; i < N_DEVICES; i++) {
      if (strcmp(mac_addresses[i], advertisedDevice.getAddress().toString().c_str()) == 0
          && advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceUUID)) {
        Serial.print("WE FOUND THE CORRECT ONE!\n");
        bt_devcs[i] = new BLEAdvertisedDevice(advertisedDevice);
        count_devices++;
        break;
      }
    }

    if (count_devices == N_DEVICES) {
      Serial.println("Got all devices; stopping scan and starting connect.");      
      BLEDevice::getScan()->stop();
      do_connect = true;
    }
  }
};

void setup() {
  // Initialize serial for debugging
  Serial.begin(SERIAL_DEBUG_BAUD);
  
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

void loop() {
  // If the flag "doConnect" is true then we have scanned for and found the desired
  // BLE Server with which we wish to connect. Now we connect to it. Once we are 
  // connected we set the connected flag to be false.
  if (do_connect == true) {
    for (int i = 0; i < N_DEVICES; i++) {
      if (bt_devcs[i] != NULL && connect_ble_server(bt_devcs[i], i)) {
        Serial.printf("We are now connected to BLE Server %d.\n", i);
      } else {
        Serial.println("We have failed to connect to the server; there is nothin more we will do.");
      }
    }
    do_connect = false;
  }
  
  delay(1000); // Delay a second between loops.
}
