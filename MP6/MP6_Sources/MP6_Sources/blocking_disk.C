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
  // -- REPLACE THIS!!!
  SimpleDisk::read(_block_no, _buf);

}


void BlockingDisk::write(unsigned long _block_no, unsigned char * _buf) {
  // -- REPLACE THIS!!!
  SimpleDisk::write(_block_no, _buf);
}
