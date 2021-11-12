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

    Console::puts("\n\n++++++++++++++++ Constructed Scheduler ++++++++++++++++\n\n");
}

void Scheduler::yield() {

    Console::puts("\n       -> yield: start\n");
    Thread* next = this->ready_head;

    this->ready_head = this->ready_head->next;

    Thread::CurrentThread()->next = NULL;

                      // print -------------------------------------------------------
                          Console::puts("       -> yield:     LL [ ");  
                          Thread* curr = this->ready_head;
                          while(curr){
                            Console::puti((unsigned int)curr);Console::puts(" -> ");
                            curr = curr->next;
                          }
                          Console::puts("]\n");  
                      // print -------------------------------------------------------


    Thread::dispatch_to(next);
}

void Scheduler::resume(Thread * _thread) {

                      // print -------------------------------------------------------
                          Console::puts("\n       -> resume:     LL [ ");  
                          Thread* curr = this->ready_head;
                          while(curr){
                            Console::puti((unsigned int)curr);Console::puts(" -> ");
                            curr = curr->next;
                          }
                          Console::puts("]\n");  
                      // print -------------------------------------------------------
  
    Console::puts("       -> resume: start\n");

    if(this->ready_head == NULL){
        Console::puts("       -> resume: add to start\n");
        this->ready_head = _thread;
        this->ready_tail = _thread;
    }
    else{
        Console::puts("       -> resume: add to end\n");
        this->ready_tail->next = _thread;
        this->ready_tail = _thread;
    }

}

void Scheduler::add(Thread * _thread) {
    
    Console::puts("\n       -> add: start\n");

    _thread->next = NULL;
    
    if(this->ready_head == NULL){
        Console::puts("       -> add: add to start\n");
        this->ready_head = _thread;
        this->ready_tail = _thread;
    }
    else{
        Console::puts("       -> add: add to end\n");
        this->ready_tail->next = _thread;
        this->ready_tail = _thread;
    }

                      // print -------------------------------------------------------
                          Console::puts("       -> add:     LL [ ");  
                          Thread* curr = this->ready_head;
                          while(curr){
                            Console::puti((unsigned int)curr);Console::puts(" -> ");
                            curr = curr->next;
                          }
                          Console::puts("]\n");  
                      // print -------------------------------------------------------
}

void Scheduler::terminate(Thread * _thread) {
    assert(false);
}