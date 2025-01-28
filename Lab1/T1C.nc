/* A Simple Accelerometer Example
 *
 * Values only in the x-axis are detected in the following example.
 * For your Lab1, use extend the code for Y and Z axes.
 * Finally, interface them with a button so that the sensing starts onlt after the press of a button.
 *
 */
 
#include "UserButton.h"
#include "printfZ1.h"


module T1C @safe()
{
  	uses interface Leds;
  	uses interface Boot;

  	/* We use millisecond timer to check the shaking of client.*/
	uses interface Timer<TMilli> as TimerAccel;

  	/*Accelerometer Interface*/
	uses interface Read<uint16_t> as Xaxis;
	uses interface SplitControl as AccelControl;
}


implementation
{
    uint16_t error=100; //Set the error value
    uint16_t x;

    event void Boot.booted() 
    {
		call AccelControl.start(); //Starts accelerometer
   		call TimerAccel.startPeriodic(1000); //Starts timer

    }

    event void AccelControl.startDone(error_t err)
	{
		printfz1("  +  Accelerometer Started\n");
		x = 0;
		printfz1_init();
	}

	event void AccelControl.stopDone(error_t err) 
	{
		printfz1("Accelerometer Stopped\n");

	}

	event void TimerAccel.fired()
	{
		call Xaxis.read(); //Takes input from the x axis of the accelerometer
	}

    event void Xaxis.readDone(error_t result, uint16_t data)
	{
		printfz1("  +  X (%d) ", data);
		if (abs(x - data) > error) 
    	{
      		call Leds.led0On(); //LED correponding to the x-axis
		}
    
    	else
    	{
      		call Leds.led0Off(); //If difference is less than the error turn the LED off.
    	}
		
		x = data; //Store current sensor input to compare with the next.  
	}
}

