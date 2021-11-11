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

  Thread* curr = Thread::CurrentThread();

  /*__________ assert and pop current from ready __________*/
  assert(curr == this->ready_head);

  this->ready_head = curr->next;
  

  /*__________ dispatch to next __________*/
   
  Thread::dispatch_to(curr->next);
  
}

void Scheduler::resume(Thread * _thread) {

  Thread* curr = this->ready_head;
  while(curr){
    Console::puti(curr);Console::puts(" -> ");
  }
  Console::puts("\n");  

  if(this->ready_head == NULL){
    _thread->next = NULL;
    
    this->ready_head = _thread;
    this->ready_tail = _thread;

    return;
  }

  this->ready_tail->next = _thread;
  this->ready_tail = _thread;


  Console::puts("++++++++++++++++ Added thread to ready ++++++++++++++++\n");
}

void Scheduler::add(Thread * _thread) {
  
  resume(_thread);
}

void Scheduler::terminate(Thread * _thread) {
  assert(false);
}
