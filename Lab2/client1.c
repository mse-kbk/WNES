#include <stdio.h>
#include <string.h>
#include "contiki.h"
#include "dev/leds.h"
#include "net/netstack.h"
#include "net/nullnet/nullnet.h"
#include "dev/adxl345.h"

#define ACCM_READ_INTERVAL (CLOCK_SECOND / 100)  
#define MOVEMENT_THRESHOLD 100  // Threshold
#define LED_INT_ONTIME CLOCK_SECOND

static process_event_t ledOff_event;

PROCESS(client_process, "Accelerometer Client");
PROCESS(led_process, "LED handling process");
AUTOSTART_PROCESSES(&client_process, &led_process);

static struct etimer ledETimer;

/* Callback function for received packets (not needed for client) */
static void recv(const void *data, uint16_t len,
  const linkaddr_t *src, const linkaddr_t *dest) {
}


PROCESS_THREAD(led_process, ev, data) {
  PROCESS_BEGIN();
  	etimer_set(&ledETimer, LED_INT_ONTIME);
  while(1){
	
    PROCESS_WAIT_EVENT_UNTIL(ev == ledOff_event || etimer_expired(&ledETimer) );
	if (ev == ledOff_event) {
		etimer_restart(&ledETimer);
	}
	if (etimer_expired(&ledETimer)) {
		leds_off(LEDS_RED);
	}
  }
  PROCESS_END();
}

/* Client process */
PROCESS_THREAD(client_process, ev, data) {
    static struct etimer et;
    static char payload[] = "ALARM";
    int16_t x, y, z;

    PROCESS_BEGIN();

    /* Initialize accelerometer */
    accm_init();
	ledOff_event = process_alloc_event();

    nullnet_buf = (uint8_t *)&payload;
    nullnet_len = sizeof(payload);
    nullnet_set_input_callback(recv);

    while (1) {
        /* Start timer for 100 Hz sampling */
        etimer_set(&et, ACCM_READ_INTERVAL);
        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

        /* Read acceleration values */
        x = accm_read_axis(X_AXIS);
	    y = accm_read_axis(Y_AXIS);
	    z = accm_read_axis(Z_AXIS);
	    printf("x: %d y: %d z: %d\n", x, y, z);

      if (x>MOVEMENT_THRESHOLD || x< -MOVEMENT_THRESHOLD){
		leds_on(LEDS_RED);
		process_post(&led_process, ledOff_event, NULL);
		NETSTACK_NETWORK.output(NULL);
	  }
		
	}
	
PROCESS_END();
}
