
#include <stdio.h>
#include <string.h>
#include "contiki.h"
#include "dev/button-sensor.h"
#include "dev/leds.h"
#include "net/netstack.h"
#include "sys/etimer.h"
#include "net/nullnet/nullnet.h"

#define MAX_NUMBER_OF_EVENTS 3 
#define YELLOW_LED_DURATION (10*CLOCK_SECOND)


/*---------------------------------------------------------------------------*/
PROCESS(clicker_ng_process, "Clicker NG Process");
PROCESS (yellow_led_timer_process, "BLue LED Timer process");
AUTOSTART_PROCESSES(&clicker_ng_process, &yellow_led_timer_process);

/*---------------------------------------------------------------------------*/
static uint8_t diffaddress = 0;
static uint8_t lessthan30 = 0;
static uint8_t firstrcv = 1;
static uint8_t yellow_on = 0;
static struct etimer yellow_led_timer;
const linkaddr_t linkaddr_null;

struct event {
  clock_time_t time;
  linkaddr_t addr;
};
struct event event_history[MAX_NUMBER_OF_EVENTS];


void init_event_array(){
  uint8_t i;
  for (i=1; i<MAX_NUMBER_OF_EVENTS; i++) {
    event_history[i].time = 0;
    event_history[i].addr = linkaddr_null;
  }
}
void handle_event(const linkaddr_t *src) {
  diffaddress = 0;
  lessthan30 = 0;
  // If the same nodes are clicket twice, only update time
  if (linkaddr_cmp(&(event_history[2].addr), src) == 1) {
    event_history[2].time = clock_time();
  }
  else {
    event_history[0] = event_history[1];
    event_history[1] = event_history[2];
    event_history[2].time = clock_time();
    event_history[2].addr = *src;
  }

  if (((linkaddr_cmp(&(event_history[0].addr), &linkaddr_null)) == 0) &&
    (linkaddr_cmp (&(event_history[0].addr), &(event_history[1].addr)) == 0) && 
    (linkaddr_cmp (&(event_history[1].addr), &(event_history[2].addr)) == 0) && 
    (linkaddr_cmp (&(event_history[0].addr), &(event_history[2].addr)) == 0))
    {
        diffaddress = 1;
    }
  
  

  clock_time_t timedif= (event_history[2].time) - (event_history[0].time);
  
  if (timedif / CLOCK_SECOND < 30) {
    lessthan30 = 1;
  }

  printf("diffaddress: %d\n ",diffaddress );
  printf("lessthan30: %d\n ",lessthan30 );

  if ((lessthan30==1) && (diffaddress == 1)) {
    process_post(&yellow_led_timer_process, PROCESS_EVENT_CONTINUE, NULL);
  }

}


static void recv(const void *data, uint16_t len,
  const linkaddr_t *src, const linkaddr_t *dest) {
  printf("Received: %s - from %d\n", (char*) data, src->u8[0]);
  if (firstrcv ){
    init_event_array();
    firstrcv = 0;
  }
  handle_event(src);

}
/*---------------------------------------------------------------------------*/

PROCESS_THREAD(clicker_ng_process, ev, data)
{
  static char payload[] = "hej";

  PROCESS_BEGIN();


  /* Initialize NullNet */
   nullnet_buf = (uint8_t *)&payload;
   nullnet_len = sizeof(payload);
   nullnet_set_input_callback(recv);
  
  /* Activate the button sensor. */
  SENSORS_ACTIVATE(button_sensor);

  while(1) {
  
  PROCESS_WAIT_EVENT_UNTIL(ev == sensors_event && data == &button_sensor);
    
    leds_toggle(LEDS_RED);
    memcpy(nullnet_buf, &payload, sizeof(payload));
    nullnet_len = sizeof(payload);

    /* Send the content of the packet buffer using the
     * broadcast handle. */
     NETSTACK_NETWORK.output(NULL);
  }


  PROCESS_END();
}


PROCESS_THREAD(yellow_led_timer_process, ev, data) {
  PROCESS_BEGIN();

  while(1) {
  PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_CONTINUE ||etimer_expired(&yellow_led_timer) );
  if (ev == PROCESS_EVENT_CONTINUE) {
    if (!yellow_on){
      etimer_set (&yellow_led_timer, YELLOW_LED_DURATION );
      yellow_on = 1;
      leds_on(LEDS_YELLOW);
    }
    else {
      etimer_reset(&yellow_led_timer);
    }
  }
  else if (etimer_expired(&yellow_led_timer)) {
    leds_off(LEDS_YELLOW);
    yellow_on = 0;
  }
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
