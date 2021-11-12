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

    Console::puts("++++++++++++++++ Constructed Scheduler ++++++++++++++++\n");
}

void Scheduler::yield() {

    Thread* next = this->ready_head;

    this->ready_head = this->ready_head->next;

    Thread::dispatch_to(next);
}

void Scheduler::resume(Thread * _thread) {
        
    if(this->ready_head == NULL){
        this->ready_head = _thread;
        this->ready_tail = _thread;
    }
    else{
        this->ready_tail->next = _thread;
        this->ready_tail = _thread;
    }
}

void Scheduler::add(Thread * _thread) {
    
    _thread->next = NULL;
    
    if(this->ready_head == NULL){
        this->ready_head = _thread;
        this->ready_tail = _thread;
    }
    else{
        this->ready_tail->next = _thread;
        this->ready_tail = _thread;
    }
}

void Scheduler::terminate(Thread * _thread) {
    assert(false);
}