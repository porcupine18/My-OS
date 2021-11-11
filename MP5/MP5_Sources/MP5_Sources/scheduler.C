/*
 File: scheduler.C
 
 Author:
 Date  :
 
 */

/* DEFINES -----------------------------------------------------------------*/

/* INCLUDES ----------------------------------------------------------------*/
#include "scheduler.H"
#include "thread.H"
#include "console.H"
#include "utils.H"
#include "assert.H"
#include "simple_keyboard.H"

/* DATA STRUCTURES ---------------------------------------------------------*/

/* CONSTANTS ---------------------------------------------------------------*/

/* FORWARDS ----------------------------------------------------------------*/

/* CLASS: Scheduler --------------------------------------------------------*/
Scheduler::Scheduler() {

  /*__________ create linked list of threads __________*/
  this->ready_head = NULL;
  this->ready_tail = NULL;

  /*__________ init EOQ handler __________*/
  // later

  Console::puts("++++++++++++++++ Constructed Scheduler ++++++++++++++++\n");
}

void Scheduler::yield(){

        Console::puts("     -> yield: start\n");

  Thread* curr = Thread::CurrentThread();


  /*__________ assert and pop current from ready __________*/
  assert(curr == this->ready_head);

  this->ready_head = curr->next;

        Console::puts("     -> yield: asserted and popped curr\n");  

  /*__________ dispatch to next __________*/
   
        Console::puts("     -> yield: going to dispatch\n");  

  Thread::dispatch_to(curr->next);
  
}

void Scheduler::resume(Thread * _thread) {

        Console::puts("     -> resume: start\n");

  _thread->next = NULL;

  Thread* curr = this->ready_head;
  while(curr){
    Console::puti((unsigned int)curr);Console::puts(" -> ");
  }

  if(this->ready_head == NULL){

    this->ready_head = _thread;
    this->ready_tail = _thread;

    Console::puts("     -> resume: first element added\n");
    Console::puts("++++++++++++++++ Added thread to ready ++++++++++++++++\n");
    return;
  }

  Console::puts("\n");  
  this->ready_tail->next = _thread;
  this->ready_tail = _thread;

  Console::puts("     -> resume: added to end\n");
  Console::puts("++++++++++++++++ Added thread to ready ++++++++++++++++\n");
}

void Scheduler::add(Thread * _thread) {
  Console::puts("     -> add: start\n");
  
  resume(_thread);

  Console::puts("     -> add: end\n");

}

void Scheduler::terminate(Thread * _thread) {
  assert(false);
}
