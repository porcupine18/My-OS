/*
     File        : blocking_disk.c

     Author      : 
     Modified    : 

     Description : 

*/

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "assert.H"
#include "utils.H"
#include "console.H"

#include "blocking_disk.H"

#include "scheduler.H"

extern Scheduler*  SYSTEM_SCHEDULER;


void BlockingDisk::wait_until_ready(){
  if(!SimpleDisk::is_ready()){
      
      Thread* curr = Thread::CurrentThread();
      
      this->linkedlist_head->push(curr);
      SYSTEM_SCHEDULER->yield(); 
      
      Console::puts("       -> wait_until_ready: added self <"); Console::puti((unsigned int)curr); Console::puts("> and Yielding\n"); 
  }
  return;
}

/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR */
/*--------------------------------------------------------------------------*/

BlockingDisk::BlockingDisk(DISK_ID _disk_id, unsigned int _size) : SimpleDisk(_disk_id, _size) {
 
  this->linkedlist_head = new ListItem(NULL);
}

/*--------------------------------------------------------------------------*/
/* SIMPLE_DISK FUNCTIONS */
/*--------------------------------------------------------------------------*/

void BlockingDisk::read(unsigned long _block_no, unsigned char * _buf) {

  Console::puts("       -> read(): start\n"); 

  issue_operation(DISK_OPERATION::READ, _block_no);

  Console::puts("       -> read(): yielding\n"); 


  wait_until_ready();

  Console::puts("       -> read(): came back\n"); 

  /* read data from port */
  int i;
  unsigned short tmpw;
  for (i = 0; i < 256; i++) {
    tmpw = Machine::inportw(0x1F0);
    _buf[i*2]   = (unsigned char)tmpw;
    _buf[i*2+1] = (unsigned char)(tmpw >> 8);
  }

  Console::puts("       -> read(): done with read\n"); 

}


void BlockingDisk::write(unsigned long _block_no, unsigned char * _buf) {

  Console::puts("       -> write(): start\n"); 

  issue_operation(DISK_OPERATION::WRITE, _block_no);

  Console::puts("       -> write(): yielding\n"); 

  wait_until_ready();

  Console::puts("       -> write(): came back\n"); 

  /* write data to port */
  int i; 
  unsigned short tmpw;
  for (i = 0; i < 256; i++) {
    tmpw = _buf[2*i] | (_buf[2*i+1] << 8);
    Machine::outportw(0x1F0, tmpw);
  }

  Console::puts("       -> write(): done with read\n"); 
}
