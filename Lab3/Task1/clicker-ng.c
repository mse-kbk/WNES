
#include <stdio.h>
#include <string.h>
#include "contiki.h"
#include "dev/button-sensor.h"
#include "dev/leds.h"
#include "net/netstack.h"
#include "net/nullnet/nullnet.h"

#include <stdlib.h>

struct event {
clock_time_t time;
linkaddr_t addr;
};
#define MAX_NUMBER_OF_EVENTS 3
struct event* event_history[MAX_NUMBER_OF_EVENTS];

/*---------------------------------------------------------------------------*/
PROCESS(clicker_ng_process, "Clicker NG Process");
AUTOSTART_PROCESSES(&clicker_ng_process);

/*---------------------------------------------------------------------------*/
static void DEBUG() {
  // size_t i;
    printf("Events: %lu %lu %lu\n", event_history[0]->time/CLOCK_SECOND, event_history[1]->time/CLOCK_SECOND, event_history[2]->time/CLOCK_SECOND);
    // printf("Events: %d %d %d\n", event_history[0]->addr, event_history[1]->addr, event_history[2]->addr);
  // for (i = 0; i < MAX_NUMBER_OF_EVENTS; i++)
  // {
  // }
  
}

static void recv(const void *data, uint16_t len,
  const linkaddr_t *src, const linkaddr_t *dest) {
  printf("Received: %s - from %d\n", (char*) data, src->u8[0]);
  int i;
  int offset = 0;
  for (i = 0; i < MAX_NUMBER_OF_EVENTS; i++) {
    //event_history[i] = event_history[i+1];
    // if (event_history[i] != 0) {
      if (linkaddr_cmp(src, &(event_history[i]->addr))) {
        offset = i;
        break;
      }
      // printf("CC: %d %d\n", src->u8[0], event_history[i]->addr);
  // printf("OFFSET: SHOULD %llu\n", *((uint64_t*)(&(event_history[i]->addr).u8)));
    // }
  }
  printf("OFFSET: %d\n", offset);
  
  for (i = offset; i < MAX_NUMBER_OF_EVENTS-1; i++) {
    event_history[i] = event_history[i+1];
  }
  struct event* ev = malloc(sizeof(clock_time_t)+sizeof(linkaddr_t));
  ev->time = clock_time();
  ev->addr = *src;
  event_history[MAX_NUMBER_OF_EVENTS-1] = ev;
  
  if (event_history[0] != 0 && event_history[MAX_NUMBER_OF_EVENTS-1] != 0)
  {
    if ((event_history[MAX_NUMBER_OF_EVENTS-1]->time-event_history[0]->time)/CLOCK_SECOND < 30) {
  leds_on(LEDS_YELLOW);
    } else {
		leds_off(LEDS_YELLOW);
  }
  DEBUG();
  leds_toggle(LEDS_GREEN);
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

  int i;
  for (i = 0; i < MAX_NUMBER_OF_EVENTS; i++) {
    event_history[i] = 0;
  }

  while(1) {
  
    PROCESS_WAIT_EVENT_UNTIL(ev == sensors_event &&
			data == &button_sensor);
			
    leds_toggle(LEDS_RED);
    
    
    memcpy(nullnet_buf, &payload, sizeof(payload));
    nullnet_len = sizeof(payload);

    /* Send the content of the packet buffer using the
     * broadcast handle. */
     NETSTACK_NETWORK.output(NULL);
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
