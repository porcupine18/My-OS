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

        Console::puts("     -> yield: start -           curr = ");Console::puti((int)curr);Console::puts("\n");  


    Thread* tmp = this->ready_head;
    while(tmp->next){
      if(tmp->next == curr){
        tmp->next = curr->next;
        Console::puts("     -> yield: popped \n");
        break;
      }
      tmp = tmp->next;
    }
    
    Console::puts("     -> yield: LL [ ");  
    tmp = this->ready_head;
    while(tmp){
      Console::puti((unsigned int)tmp);Console::puts(" -> ");
      tmp = tmp->next;
    }
    Console::puts(" ]\n");  


  if(curr->next == NULL){

    this->ready_head = curr->next;
    Thread::dispatch_to(curr);
        Console::puts("     -> yield: same continued - curr = ");Console::puti((int)curr);Console::puts("\n");  
  }
  else{
        Console::puts("     -> yield: yield to -   new curr = ");Console::puti((int)curr);Console::puts("\n");  
    this->ready_head = curr->next;
    Thread::dispatch_to(curr->next);    
    assert(false);
  }
  
  Console::puts("+++++++++++++++++++++ Yield  Done +++++++++++++++++++++\n");

}

void Scheduler::resume(Thread * _thread) {

        Console::puts("     -> resume: LL [ ");  
        Thread* curr = this->ready_head;
        while(curr){
          Console::puti((unsigned int)curr);Console::puts(" -> ");
          curr = curr->next;
        }
        Console::puti((unsigned int)_thread);Console::puts(" ]\n");  

  _thread->next = NULL;

  if(this->ready_head == NULL){
    this->ready_head = _thread;
    this->ready_tail = _thread;
    Console::puts("     ++++++++++++++++ Added first to ready +++++++++++++++++\n");
    return;
  }

  this->ready_tail->next = _thread;
  this->ready_tail = _thread;
  Console::puts("     ++++++++++++++++ Added thread to ready ++++++++++++++++\n");
}

void Scheduler::add(Thread * _thread) {
  Console::puts("     -> add: start");
  
  resume(_thread);

}

void Scheduler::terminate(Thread * _thread) {
  assert(false);
}
