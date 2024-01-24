#include "sensor.h"
#include "leds.h"
#include <zephyr/drivers/sensor.h>

// https://docs.zephyrproject.org/2.7.5/boards/arm/thingy52_nrf52832/doc/index.html
// example in ncs/zepyhr/samples/hts221/src/main.c
// https://docs.zephyrproject.org/latest/samples/sensor/hts221/README.html
static const struct device *sensor = DEVICE_DT_GET_ONE(st_hts221);
double temperature;
double humidity;
uint32_t timestamp;

/**
 * @brief Get the temperature from the sensor and write it on <VARIABLE NAME>
 * https://docs.zephyrproject.org/latest/hardware/peripherals/sensor.html#triggers
 * https://docs.zephyrproject.org/2.7.5/reference/kernel/timing/clocks.html?highlight=time#uptime
 * @param dev temperature device
 * @param trig trigger configuration
 */
static void get_measurements(const struct device *dev,
                            const struct sensor_trigger *trig) {
    struct sensor_value temp, hum;
    int error;
    if ((error = sensor_sample_fetch(dev)) < 0) {
        printk("Sensor sample_fetch error: %d\n", error);
        return;
    }

    if ((error = sensor_channel_get(dev, SENSOR_CHAN_AMBIENT_TEMP, &temp)) < 0) {
        printk("Error while getting the temperature from the AMBIENT_TEMP channel: %d\n", error);
        return;
    }
    if (sensor_channel_get(dev, SENSOR_CHAN_HUMIDITY, &hum) < 0) {
        printf("Cannot read HTS221 humidity channel\n");
        return;
    }
    timestamp = k_uptime_get_32();
    temperature = sensor_value_to_double(&temp);
    humidity = sensor_value_to_double(&hum);

    turn_on_color(green);
    printk("<%d> Temperature/Humidity are approximately %d.%02dC|%d.%02d%%\n", timestamp, temp.val1, temp.val2, hum.val1, hum.val2);
}

int init_sensor() {
    if (!device_is_ready(sensor)) {
        printk("init_sensor failed: sensor not ready\n");
        return -1;
    }

    if (IS_ENABLED(CONFIG_HTS221_TRIGGER)) {
        // https://docs.zephyrproject.org/latest/hardware/peripherals/sensor.html#c.sensor_trigger
        // it doesn't work if I put two triggers
        struct sensor_trigger temp_hum_trigger = {
            .type = SENSOR_TRIG_DATA_READY,
            .chan = SENSOR_CHAN_ALL,
        };
        if (sensor_trigger_set(sensor, &temp_hum_trigger, get_measurements) < 0) {
            printk("init_sensor failed: cannot configure the temp_hum_trigger\n");
            return -1;
        }

        printk("Temperature/Humidity sensor successfully initialized.\n");
        return 0;
    }

    printk("init_sensor failed: trigger config not enabled\n");
    return -1;
}
