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
	/*__________ init head and tail of linked list of threads __________*/
	this->ready_head = NULL;
	this->ready_tail = NULL;

	this->zombie_head = NULL;
	this->zombie_tail = NULL;

	Console::puts("\n\n++++++++++++++++ Constructed Scheduler ++++++++++++++++\n\n");
}

void Scheduler::yield() {


	Console::puts("       -> yield: start\n");

	/*__________ return if ready queue is empty __________*/
	if(!this->ready_head){
		Console::puts("       -> yield: ready queue empty, NOTHING TO YIELD TO!\n");
		return;
	}

	/*__________ pop thread from ready queue __________*/

	Thread* next = this->ready_head;

	this->ready_head = this->ready_head->next;

	/*__________ set current thread's next to NULL as it is in the end too once popped __________*/
	Thread::CurrentThread()->next = NULL;

	Console::puts("       -> yield:     LL [ "); print_ll(this->ready_head); Console::puts("]\n");  

	/*__________ dispatch to next thread __________*/

	Console::puts("       -> yield: yeilding to :"); Console::puti((unsigned int)next);Console::puts("\n");

	Thread::dispatch_to(next);

	return;
}

void Scheduler::resume(Thread * _thread) {

	Console::puts("       -> resume:     LL [ "); print_ll(this->ready_head); Console::puts("]\n");  
  
	/*__________ add thread to ready queue __________*/
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

	/*__________ clean zombie queue __________*/
	Thread* tmp = this->zombie_head;
	Thread* tmp2 = NULL;

	Console::puts("       -> resume: cleaned zombie : [");
	
	while(tmp){
		Console::puti((int)tmp);Console::puts(" -> ");

		tmp2 = tmp->next;
		delete tmp;
		tmp = tmp2;
	}

	Console::puts("]\n");

	this->zombie_head = NULL;
	this->zombie_tail = NULL;

}

void Scheduler::add(Thread * _thread) {
	
	/*__________ add thread to ready queue __________*/
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

	Console::puts("       -> add:     LL [ "); print_ll(this->ready_head); Console::puts("]\n");  
}

void Scheduler::terminate(Thread * _thread) {
    
	Console::puts("\n       -> terminate: start\n");

	/*__________ if not current thread, remove from linked list __________*/
	if(_thread != Thread::CurrentThread()){

		Console::puts("       -> terminate: not current thread, removing from ready\n");

		if(_thread == this->ready_head){ //check if thread to be delete is first thread

			this->ready_head = this->ready_head->next;

			if(this->ready_tail == _thread)
				this->ready_tail = NULL;

			delete _thread;

			return;
		}

		Thread* tmp = this->ready_head;
		while(tmp->next){
			if(tmp->next == _thread){
				tmp->next = _thread->next;
			}
			tmp = tmp->next;
		}
		delete _thread;

		Console::puts("       -> terminate: DONE\n");

		return;
	}

	Console::puts("       -> terminate: making current zombie\n");

	/*__________ if current thread, add to zombie list __________*/

	_thread->next = NULL;

	if(this->zombie_head == NULL){
		this->zombie_head = _thread;
		this->zombie_tail = _thread;
	}
	else{
		this->zombie_tail->next = _thread;
		this->zombie_tail = _thread;
	}

	Console::puts("       -> terminate: ZOMBIE LL [ "); print_ll(this->zombie_head); Console::puts("]\n");  
	Console::puts("       -> terminate: yielding\n");

	yield();

	Console::puts("       -> terminate: READY EMPTY, CURRENT THREAD IS ZOMBIE\n");

}

void print_ll(Thread* head){
	Thread* curr = head;
	while(curr){
		Console::puti((unsigned int)curr);Console::puts(" -> ");
		curr = curr->next;
	}
}

