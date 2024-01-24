#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <stdlib.h>

#include "leds.h"

#define LED0_NODE DT_ALIAS(led0)
#define LED1_NODE DT_ALIAS(led1)
#define LED2_NODE DT_ALIAS(led2)

/*
 * A build error on this line means your board is unsupported.
 * See the sample documentation for information on how to fix this.
 */
static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(LED1_NODE, gpios);
static const struct gpio_dt_spec led2 = GPIO_DT_SPEC_GET(LED2_NODE, gpios);

RGBColors colors[NUM_COLORS] = {
    {0, 1, 1},
    {1, 0, 1},
    {1, 1, 0},
    {0, 0, 1},
    {0, 1, 0},
};

void turn_on_color(Color c) {
    gpio_pin_set_dt(&led0, colors[c].r);
    gpio_pin_set_dt(&led1, colors[c].g);
    gpio_pin_set_dt(&led2, colors[c].b);
}

int init_leds(void)
{
    if (!gpio_is_ready_dt(&led0) &&
        gpio_pin_configure_dt(&led0, GPIO_OUTPUT_ACTIVE) < 0) {
		return -1;
	}
    if (!gpio_is_ready_dt(&led1) &&
        gpio_pin_configure_dt(&led1, GPIO_OUTPUT_ACTIVE) < 0) {
		return -1;
	}
    if (!gpio_is_ready_dt(&led2) &&
        gpio_pin_configure_dt(&led2, GPIO_OUTPUT_ACTIVE) < 0) {
		return -1;
	}
    return 0;
}
