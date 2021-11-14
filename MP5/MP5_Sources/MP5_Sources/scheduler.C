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


	//Console::puts("\n       -> yield: start\n");

	/*__________ return if ready queue is empty __________*/
	if(!this->ready_head){
		return;
	}

	/*__________ pop thread from ready queue __________*/

	Thread* next = this->ready_head;

	this->ready_head = this->ready_head->next;

	/*__________ set current thread's next to NULL as it is in the end too once popped __________*/
	Thread::CurrentThread()->next = NULL;

										// print -------------------------------------------------------
												//Console::puts("       -> yield:     LL [ ");  
												Thread* curr = this->ready_head;
												while(curr){
													//Console::puti((unsigned int)curr);Console::puts(" -> ");
													curr = curr->next;
												}
												//Console::puts("]\n\n");  
										// print -------------------------------------------------------

	/*__________ dispatch to next thread __________*/
	Thread::dispatch_to(next);

	return;
}

void Scheduler::resume(Thread * _thread) {

	                    // print -------------------------------------------------------
                          //Console::puts("\n       -> resume:     LL [ ");  
                          Thread* curr = this->ready_head;
                          while(curr){
                            //Console::puti((unsigned int)curr);Console::puts(" -> ");
                            curr = curr->next;
                          }
                          //Console::puts("]\n");  
                      // print -------------------------------------------------------
  
	/*__________ add thread to ready queue __________*/
	//Console::puts("       -> resume: start\n");

	if(this->ready_head == NULL){
		//Console::puts("       -> resume: add to start\n");
		this->ready_head = _thread;
		this->ready_tail = _thread;
	}
	else{
		//Console::puts("       -> resume: add to end\n");
		this->ready_tail->next = _thread;
		this->ready_tail = _thread;
	}

	/*__________ clean zombie queue __________*/
	Thread* tmp = this->zombie_head;
	Thread* tmp2 = NULL;

	while(tmp){
		tmp2 = tmp->next;
		delete tmp;
		tmp = tmp2;
	}

	this->zombie_head = NULL;
	this->zombie_tail = NULL;


}

void Scheduler::add(Thread * _thread) {
	
	/*__________ add thread to ready queue __________*/
	//Console::puts("\n       -> add: start\n");

	_thread->next = NULL;
	
	if(this->ready_head == NULL){
		//Console::puts("       -> add: add to start\n");
		this->ready_head = _thread;
		this->ready_tail = _thread;
	}
	else{
		//Console::puts("       -> add: add to end\n");
		this->ready_tail->next = _thread;
		this->ready_tail = _thread;
	}

										// print -------------------------------------------------------
												//Console::puts("       -> add:     LL [ ");  
												Thread* curr = this->ready_head;
												while(curr){
													//Console::puti((unsigned int)curr);Console::puts(" -> ");
													curr = curr->next;
												}
												//Console::puts("]\n");  
										// print -------------------------------------------------------
}

void Scheduler::terminate(Thread * _thread) {
    
	/*__________ if not current thread, remove from linked list __________*/
	if(_thread != Thread::CurrentThread()){

		if(_thread == this->ready_head){
			_thread->next = NULL;
			this->ready_head = NULL;
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
		return;
	}

	/*__________ if current thread, add to zombie list __________*/

	_thread->next = NULL;

	if(this->zombie_head == NULL){
		this->zombie_head = _thread;
		this->zombie_tail = _thread;
		return;
	}

	this->zombie_tail->next = _thread;
	this->zombie_tail = _thread;

	yield();

}