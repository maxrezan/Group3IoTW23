#ifndef leds_h
#define leds_h

typedef enum {
    red,
    green,
    blue,
    yellow,
    purple,
    NUM_COLORS
} Color;

/**
 * @brief 
 * 
 */
typedef struct {
    unsigned int r;
    unsigned int g;
    unsigned int b;
} RGBColors;

/**
 * @brief Displays the given color on the thingy52
 * 
 * @param c Color to display
 */
void turn_on_color(Color c);

/**
 * @brief Initializes the leds
 * 
 */
int init_leds(void);

#endif
