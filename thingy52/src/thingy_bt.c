#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/sys/byteorder.h>
#include <stdlib.h>

#define DEVICE_NAME CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN (sizeof(DEVICE_NAME) - 1) // null terminator
#define BT_UUID_HTS_INTERMEDIATE BT_UUID_DECLARE_16(0x2a1e)

static uint8_t data[5];

/*
 * Define and register service
 */
BT_GATT_SERVICE_DEFINE(health_thermometer_svc,
    BT_GATT_PRIMARY_SERVICE(BT_UUID_HTS),
    BT_GATT_CHARACTERISTIC(
	BT_UUID_HTS_INTERMEDIATE,
	BT_GATT_CHRC_NOTIFY,
        BT_GATT_PERM_READ,
	NULL, NULL, NULL
    ),
    BT_GATT_CCC(NULL, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
);

/**
 * Data to advertise in the advertisement packets
 */
static const struct bt_data advertisement_pk[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, BT_LE_AD_GENERAL),
    BT_DATA_BYTES(
        BT_DATA_UUID16_ALL,
        BT_UUID_16_ENCODE(BT_UUID_HTS_VAL)
    ), // Advertising Health Termometer Service
};

/* Data to scan in response packets */
static const struct bt_data response_pk[] = {
    BT_DATA(
            BT_DATA_NAME_COMPLETE,
            DEVICE_NAME,
            DEVICE_NAME_LEN
    ), // Scanning name of device
};

/**
 * Initializes the bluetooth device and starts advertisement
 */
static void bt_ready (int error) {
  bt_addr_le_t address = {0};
  char address_string[BT_ADDR_LE_STR_LEN];
  size_t counter = 1;

  if (error) {
    printk("Bluetooth initialization failed | Error: %d\n", error);
    exit(-1);
  }

  printk("Bluetooth successfully initialized\n");

  /* Start the advertisement */
  error = bt_le_adv_start(
		BT_LE_ADV_CONN,
		advertisement_pk,
		ARRAY_SIZE(advertisement_pk),
		response_pk,
		ARRAY_SIZE(response_pk)
  );
  if (error) {
    printk("Troubles while starting the advertisement | Error: %d\n", error);
    exit(-1);
  }

  bt_id_get(&address, &counter);
  bt_addr_le_to_str(&address, address_string, sizeof(address_string));

  printk("Beacon started | Advertising as %s\n", address_string);
}

/**
 * Callback function called when device connects via bluetooth
 *
 * @param connection Established bluetooth connection
 * @param error Error value 0 if everything ok
 */
static void bt_connected(struct bt_conn *connection, uint8_t error) {
  if (error) {
    printk("Troubles while connecting via bluetooth | Error: %d\n", error);
  }

  printk("Bluetooth connected\n");
}

/**
 * Callback function called when a device disconnecs via bluetooth
 *
 * @param connection Bluetooth connection that was removed
 * @param reason Reason why the connection was removed
 */
static void bt_disconnected(struct bt_conn *connection, uint8_t reason) {
  printk("Bluetooth disconnected | Reason: %d\n", reason);
}

/**
 * Struct with the callback functions for the
 * connection/disconnection of the bluetooth connections
 */
static struct bt_conn_cb bt_connection = {
    .connected = bt_connected,
    .disconnected = bt_disconnected,
};

/**
 * Send data to the server without expecting any acknowledgment
 */
void notify_server() {
  data[0] = 0;
  data[1] = 1;
  data[2] = 2;
  data[3] = 3;
  data[4] = 4;
  bt_gatt_notify(
	NULL,
	&health_thermometer_svc.attrs[1],
	data,
	sizeof(data)
  );
  printk("Server notified\n");
}

int bt_init() {
  // Initialize the bluetooth device
  int error = bt_enable(bt_ready);
  if (error) {
    printk("Bluetooth initialization failed | Error: %d\n", error);
    exit(-1);
  }

  // Register the callback functions for bluetooth connections
  bt_conn_cb_register(&bt_connection);
  return 0;
}
