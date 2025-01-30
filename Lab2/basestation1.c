#include <stdio.h>
#include <string.h>
#include "contiki.h"
#include "dev/leds.h"
#include "net/netstack.h"
#include "net/nullnet/nullnet.h"

#define LED_INT_ONTIME        CLOCK_SECOND*10


/* Declare our "main" process, the basestation_process */
PROCESS(basestation_process, "Clicker basestation");
PROCESS(led_process, "LED handling process");
/* The basestation process should be started automatically when
 * the node has booted. */
AUTOSTART_PROCESSES(&basestation_process, &led_process);

/* Holds the number of packets received. */
static struct etimer led_timer;
static volatile int received_alarm = 0;
static int count = 0;


/* Callback function for received packets.
 *
 * Whenever this node receives a packet for its broadcast handle,
 * this function will be called.
 *
 * As the client does not need to receive, the function does not do anything
 */
static void recv(const void *data, uint16_t len,
  const linkaddr_t *src, const linkaddr_t *dest) {
   
    count++;
    /* 0bxxxxx allows us to write binary values */
    /* for example, 0b10 is 2 */
    leds_off(LEDS_ALL);
    leds_on(count & 0b1111);

    received_alarm=1;
    // etimer_restart(&led_timer);

    process_poll(&led_process);
}

// static struct etimer ledETimer;
PROCESS_THREAD(led_process, ev, data) {
  PROCESS_BEGIN();

  etimer_set(&led_timer, CLOCK_SECOND*10);
  
  while(1){
    PROCESS_WAIT_EVENT_UNTIL(ev==PROCESS_EVENT_POLL || etimer_expired(&led_timer) );

    if(ev == PROCESS_EVENT_POLL){
      etimer_restart(&led_timer);
    }

    if (etimer_expired(&led_timer) && !received_alarm) {
      leds_off(LEDS_ALL);
    }
    received_alarm=0;
    
  }
  PROCESS_END();
}

/* Our main process. */
PROCESS_THREAD(basestation_process, ev, data) {
	PROCESS_BEGIN();

	/* Initialize NullNet */
	nullnet_set_input_callback(recv);

	PROCESS_END();
}
