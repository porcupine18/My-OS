/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "interrupts.H"

class EOQTimer: public InterruptHandler{

private:

  /* How long has the system been running? */
  unsigned long seconds; 
  int           ticks;   /* ticks since last "seconds" update.    */

  /* At what frequency do we update the ticks counter? */
  int hz;                /* Actually, by defaults it is 18.22Hz.
                            In this way, a 16-bit counter wraps
                            around every hour.                    */

  void set_frequency(int _hz);
  /* Set the interrupt frequency for the simple timer. */

public :

  EOQTimer(int _hz);
  /* Initialize the simple timer, and set its frequency. */

  virtual void handle_interrupt(REGS *_r);
  /* This must be installed as the interrupt handler for the timer 
     when the system gets initialized. (e.g. in "kernel.C")  
  */

  void current(unsigned long * _seconds, int * _ticks);
  /* Return the current "time" since the system started. */

  void wait(unsigned long _seconds);
  /* Wait for a particular time to be passed. The implementation is based 
     on busy looping! */

};

/*
    Console::puts("\n\n!!!!! Timer interrupt --> Preempting !!!!! \n\n");

    SYSTEM_SCHEDULER->resume(Thread::CurrentThread());
    SYSTEM_SCHEDULER->yield();
    Machine::outportb(0x20, 0x20);
*/