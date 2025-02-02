#include <stdio.h>
#include <string.h>
#include "contiki.h"
#include "dev/leds.h"
#include "net/netstack.h"
#include "net/nullnet/nullnet.h"

#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO

#define LED_INT_ONTIME        CLOCK_SECOND*10


/* Declare our "main" process, the basestation_process */
PROCESS(basestation_process, "Clicker basestation");
PROCESS(led_btn_process, "LED BTN handling process");
PROCESS(led_shk_process, "LED SHK handling process");
/* The basestation process should be started automatically when
 * the node has booted. */
AUTOSTART_PROCESSES(&basestation_process, &led_shk_process, &led_btn_process);

/* Holds the number of packets received. */
static struct etimer led_btn_timer;
static struct etimer led_shk_timer;

static int btn = 0;
static int shk = 0;


/* Callback function for received packets.
 *
 * Whenever this node receives a packet for its broadcast handle,
 * this function will be called.
 *
 * As the client does not need to receive, the function does not do anything
 */
static void recv(const void *data, uint16_t len,
  const linkaddr_t *src, const linkaddr_t *dest) {
   
   char a[20];
   memcpy(&a, data, sizeof(a));
   //LOG_INFO("GOT: %s", a);
   if (strcmp(a, "shkalarm") == 0) {
    LOG_INFO("SHKALARM: %s, %d, %d\n", a, btn, shk);
      /* 0bxxxxx allows us to write binary values */
      /* for example, 0b10 is 2 */
    //   leds_off(LEDS_ALL);
      leds_on(0b0001);
      shk=1;
   if (btn==1 && shk==1) {
    leds_on(0b0100);
   }

      process_poll(&led_shk_process);
   } else if (strcmp(a, "btnalarm") == 0) {
    LOG_INFO("BTNALARM, %s, %d, %d\n", a, btn, shk);
      btn=1;
    //   leds_off(LEDS_ALL);
      leds_on(0b0010);
   if (btn==1 && shk==1) {
    leds_on(0b0100);
   }

      process_poll(&led_btn_process);
   }

}

// static struct etimer ledETimer;
PROCESS_THREAD(led_shk_process, ev, data) {
  PROCESS_BEGIN();

  etimer_set(&led_shk_timer, CLOCK_SECOND*10);
  
  while(1){
    PROCESS_WAIT_EVENT_UNTIL(ev==PROCESS_EVENT_POLL || etimer_expired(&led_shk_timer) );

    if(ev == PROCESS_EVENT_POLL){
      etimer_restart(&led_shk_timer);
    }

    if (etimer_expired(&led_shk_timer) && shk==1) {
      leds_off(0b0101);
    shk=0;
    }
    
  }
  PROCESS_END();
}

PROCESS_THREAD(led_btn_process, ev, data) {
  PROCESS_BEGIN();

  etimer_set(&led_btn_timer, CLOCK_SECOND*10);
  
  while(1){
    PROCESS_WAIT_EVENT_UNTIL(ev==PROCESS_EVENT_POLL || etimer_expired(&led_btn_timer) );

    if(ev == PROCESS_EVENT_POLL){
      etimer_restart(&led_btn_timer);
    }

    if (etimer_expired(&led_btn_timer) && btn==1) {
      leds_off(0b0110);
    btn=0;
    }
    
  }
  PROCESS_END();
}

/* Our main process. */
PROCESS_THREAD(basestation_process, ev, data) {
	PROCESS_BEGIN();
    leds_off(0b1111);
	/* Initialize NullNet */
	nullnet_set_input_callback(recv);

	PROCESS_END();
}
