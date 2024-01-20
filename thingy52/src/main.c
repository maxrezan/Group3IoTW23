// Common includes
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/util.h>
#include <zephyr/types.h>

#include "leds.h"
#include "sensor.h"
#include "thingy_bt.h"

void main(void) {
  if (init_leds()) {
    printk("Having some troubles with the initialization of the leds... Help");
    exit(-1);
  }

  if (init_sensor()) {
    printk("Having some troubles with the initialization of the sensor... Help");
    exit(-1);
  }

  if (bt_init()) {
    printk("Having some troubles with the bluetooth thing... Help");
    exit(-1);
  }

  while (1) {
    notify_server();
    k_sleep(K_SECONDS(5));
  }
}
