#include <stdio.h>
#include <string.h>
#include "contiki.h"
#include "dev/leds.h"
#include "dev/button-sensor.h"
#include "net/netstack.h"
#include "net/nullnet/nullnet.h"
#include "dev/adxl345.h"

#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO

#define ACCM_READ_INTERVAL (CLOCK_SECOND / 100)  
#define MOVEMENT_THRESHOLD 200  // Threshold
#define LED_INT_ONTIME CLOCK_SECOND

static process_event_t ledOff_event;

PROCESS(button_process, "Accelerometer Client");
PROCESS(accel_process, "Accelerometer Client");
PROCESS(led_process, "LED handling process");
AUTOSTART_PROCESSES(&button_process, &accel_process, &led_process);

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
PROCESS_THREAD(accel_process, ev, data) {
    static struct etimer et;
    static char payload_acc[] = "shkalarm";

    int16_t x;

    PROCESS_BEGIN();

    /* Initialize accelerometer */
    accm_init();
    
	  ledOff_event = process_alloc_event();

    nullnet_buf = (uint8_t *)&payload_acc;
    nullnet_len = sizeof(payload_acc);
    nullnet_set_input_callback(recv);

    while (1) {
        /* Start timer for 100 Hz sampling */
        etimer_set(&et, ACCM_READ_INTERVAL);
        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

        /* Read acceleration values */
      x = accm_read_axis(X_AXIS);
	    //y = accm_read_axis(Y_AXIS);
	    //z = accm_read_axis(Z_AXIS);
	    //printf("x: %d y: %d z: %d\n", x, y, z);

    if (x>MOVEMENT_THRESHOLD || x< -MOVEMENT_THRESHOLD){
   
		leds_on(LEDS_RED);
		process_post(&led_process, ledOff_event, NULL);
    strcpy(payload_acc, "shkalarm");
    memcpy(nullnet_buf, &payload_acc, sizeof(payload_acc));
    nullnet_len = sizeof(payload_acc);
    
		NETSTACK_NETWORK.output(NULL);



    char b[11];
    memcpy(&b, nullnet_buf, sizeof(b));
    LOG_INFO("SENT in shake process %s %s from \n ", b, payload_acc);
	  }
		
	}
	
PROCESS_END();
}


/* Client process */
PROCESS_THREAD(button_process, ev, data) {
    static char payload_btn[] = "btnalarm";
    

    PROCESS_BEGIN();

    /* Initialize accelerometer */
    SENSORS_ACTIVATE(button_sensor);

    nullnet_buf = (uint8_t *)&payload_btn;
    nullnet_len = sizeof(payload_btn);
    nullnet_set_input_callback(recv);

    while (1) {
        /* Start timer for 100 Hz sampling */
      PROCESS_WAIT_EVENT_UNTIL(ev == sensors_event &&
			data == &button_sensor);

		leds_toggle(LEDS_GREEN);
		/* Copy the string "hej" into the packet buffer. */
		memcpy(nullnet_buf, &payload_btn, sizeof(payload_btn));
    nullnet_len = sizeof(payload_btn);

		/* Send the content of the packet buffer using the
		 * broadcast handle. */
		NETSTACK_NETWORK.output(NULL);

  strcpy (payload_btn, "btnalarm");
  memcpy(nullnet_buf, &payload_btn, sizeof(payload_btn));
  nullnet_len = sizeof(payload_btn);


char a[11];
    memcpy(&a, nullnet_buf, sizeof(a));
    LOG_INFO("SENT in button process %s from \n ", a);
	  }
		

	
PROCESS_END();
}
