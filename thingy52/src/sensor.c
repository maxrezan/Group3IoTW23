#include "sensor.h"
#include "leds.h"
#include <zephyr/drivers/sensor.h>

// https://docs.zephyrproject.org/2.7.5/boards/arm/thingy52_nrf52832/doc/index.html
// example in ncs/zepyhr/samples/hts221/src/main.c
// https://docs.zephyrproject.org/latest/samples/sensor/hts221/README.html
static const struct device *sensor = DEVICE_DT_GET_ONE(st_hts221);
double temperature;
double humidity;
double gas;
double power;
double co2;

/**
 * @brief Get the temperature from the sensor and write it on <VARIABLE NAME>
 * https://docs.zephyrproject.org/latest/hardware/peripherals/sensor.html#triggers
 * 
 * @param dev temperature device
 * @param trig trigger configuration
 */
static void get_measurements(const struct device *dev,
                            const struct sensor_trigger *trig) {
    struct sensor_value temp, hum, dvc_temp;
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

    if (sensor_channel_get(dev, SENSOR_CHAN_GAS_RES, &gas) < 0) {
        printf("Cannot read HTS221 GAS channel\n");
        return;
    }

    if (sensor_channel_get(dev, SENSOR_CHAN_POWER, &power) < 0) {
        printf("Cannot read HTS221 power channel\n");
        return;
    }

        if (sensor_channel_get(dev, SENSOR_CHAN_CO2, &co2) < 0) {
        printf("Cannot read HTS221 co2 channel\n");
        return;
    }

    temperature = sensor_value_to_double(&temp);
    humidity = sensor_value_to_double(&hum);
    gas = sensor_value_to_double(&gas);
    power = sensor_value_to_double(&power);
    co2 = sensor_value_to_double(&co2);

    
    turn_on_color(green);
    printk("Temperature/Humidity/Gas/Power/CO2 are approximately %d.%02dC|%d.%02d%%|%d.%02d|%d.%02d|%d.%02d\n",
            temp.val1, temp.val2, hum.val1, hum.val2, gas.val1, gas.val2, power.val1, power.val2, co2.val1, co2.val2);
}

int init_sensor() {
    if (!device_is_ready(sensor)) {
        printk("init_sensor failed: sensor not ready\n");
        return -1;
    }

    if (IS_ENABLED(CONFIG_HTS221_TRIGGER)) {
        // https://docs.zephyrproject.org/latest/hardware/peripherals/sensor.html#c.sensor_trigger
        // it doesn't work if I put two triggers
        struct sensor_trigger temperature_trigger = {
            .type = SENSOR_TRIG_DATA_READY,
            .chan = SENSOR_CHAN_ALL,
        };
        if (sensor_trigger_set(sensor, &temperature_trigger, get_measurements) < 0) {
            printk("init_sensor failed: cannot configure the temperature_trigger\n");
            return -1;
        }

        printk("Sensor successfully initialized.\n");
        return 0;
    }

    printk("init_sensor failed: trigger config not enabled\n");
    return -1;
}
