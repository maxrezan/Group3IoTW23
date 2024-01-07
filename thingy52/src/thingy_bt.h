#ifndef thingy_bt_h
#define thingy_bt_h

/**
 * Sends data to the server without expecting any acknowledgment
 */
void notify_server();

/**
 * Init bluetooth and wait for conn
 */
int bt_init();

#endif
