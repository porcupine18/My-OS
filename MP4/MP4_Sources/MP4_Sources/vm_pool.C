/*
 *   File       : vm_pool.C
 *   Author     : Mehul Varma
 *   Date       : Fall 2021
 *   Description: Management of the Virtual Memory Pool
 */

/* DEFINES -----------------------------------------------------------------*/

/* INCLUDES ----------------------------------------------------------------*/
#include "vm_pool.H"
#include "console.H"
#include "utils.H"
#include "assert.H"
#include "simple_keyboard.H"

/* DATA STRUCTURES ---------------------------------------------------------*/

/* CONSTANTS ---------------------------------------------------------------*/


/* FORWARDS ----------------------------------------------------------------*/


/* CLASS: VMPool -----------------------------------------------------------*/

VMPool::VMPool(unsigned long  _base_address,
               unsigned long  _size,
               ContFramePool *_frame_pool,
               PageTable     *_page_table) {

            Console::puts("         -> in constructor\n");
    /*_______initialize instance variables_______*/
    this->_base_address = _base_address;
    this->_size         = _size;
    this->_frame_pool   = _frame_pool;
    this->_page_table   = _page_table;
            Console::puts("         -> initialized instance variables\n");


    /*_______create free and alloc lists and initialze the lists_______*/
    
    // get pointers for each of the arrays
    this->freelist_start_arr = (unsigned long*) this->_base_address;
    this->freelist_end_arr   = (unsigned long*) this->_base_address + (PAGE_SIZE/2);
                Console::puts("         -> initialized free list addr\n");

    this->alloclist_start_arr = (unsigned long*) this->_base_address + PAGE_SIZE;
    this->alloclist_end_arr   = (unsigned long*) this->_base_address + (PAGE_SIZE+PAGE_SIZE/2);
                Console::puts("         -> initialized alloc list addr\n");


    //initialize arrays
    
    // setting all elements to 0
    /*
    for (unsigned int i = 0; i < 512; i++){
        freelist_start_arr  [i] = NULL;
        freelist_end_arr    [i] = NULL;
        alloclist_start_arr [i] = NULL;
        alloclist_end_arr   [i] = NULL;
    }
    */
                Console::puts("         -> cleaned lists\n");


    freelist_start_arr[0] = this->_base_address + (2*PAGE_SIZE);
    freelist_end_arr[0]   = this->_base_address + this->_size -1;
                Console::puts("         -> first free list value init\n");

    alloclist_start_arr[0] = this->_base_address;
    alloclist_end_arr[0]   = this->_base_address + (2*PAGE_SIZE) - 1;
                Console::puts("         -> first alloc list value init\n");


    /*_______ register new VMPool_______*/
    this->_page_table->register_pool(this);
    
    Console::puts("+++++++++++++++++ Constructed  new VMPool +++++++++++++++++\n");
}

unsigned long VMPool::allocate(unsigned long _size) {

    /*_______ convert size to pages (ceiling) _______*/
    unsigned int pages_to_alloc = _size/PAGE_SIZE + (_size%PAGE_SIZE != 0);

    /*_______ initialize free and alloc lists _______*/
    bool found_free_region = false;
    unsigned int idx = 0;
    
    unsigned int j = 0; 
    // check all non-empty free list elements
    while( !((this->freelist_start_arr[idx]==0) && (this->freelist_end_arr[idx]==0)) && idx<512){

        // found area in free list to use
        if( ((this->freelist_end_arr[idx] - this->freelist_start_arr[idx])/PAGE_SIZE) > pages_to_alloc ){
            found_free_region = true;

            // find right index in alloc list to insert new region
            j = 0; 
            while(this->freelist_start_arr[j] == 0 && this->freelist_end_arr[j] == 0){
                j++;
            }
            
            this->alloclist_start_arr[j] = this->freelist_start_arr[idx];
            this->alloclist_end_arr[j]   = this->freelist_start_arr[idx] + (pages_to_alloc * PAGE_SIZE) - 1;

            // take off memory from found region
            this->freelist_start_arr[idx] = this->freelist_start_arr[idx] + (pages_to_alloc * PAGE_SIZE);

        }

        idx++;
    }

    if(found_free_region == false)
        return 0;
    else
        return this->alloclist_start_arr[j];
    

}

void VMPool::release(unsigned long _start_address) {
    assert(false);
    Console::puts("Released region of memory.\n");
}

bool VMPool::is_legitimate(unsigned long _address) {

    // if address is even in the region
    if( (this->_base_address+(2*PAGE_SIZE) <= _address) && ((this->_base_address + this->_size) > _address) ){
        unsigned int idx = 0;

        //iterating through all non-empty elements
        while( !((this->alloclist_start_arr[idx]==0) && (this->alloclist_end_arr[idx]==0)) && idx < 512){
        
            // found region where address belongs to
            if((_address >= this->alloclist_start_arr[idx]) && (_address < this->alloclist_end_arr[idx])){
                Console::puts("+++++++++++++++++ Legitimate address found! +++++++++++++++\n");
                
                return true;
            }
            idx++;
        }
        Console::puts("++++++++++++++ Legitimate address NOT found  ++++++++++++++\n");
        return false;
    }

    // if address belongs to free/alloc lists' region
    if((this->_base_address <= _address) && ((this->_base_address + 2*PAGE_SIZE) > _address)){
        Console::puts("++++++++++++++++ Free/Alloc list validated ++++++++++++++++\n");
        return true; // validating first 2 pages
    }

    Console::puts("+++++++++++++++ Illegitimate address passed +++++++++++++++\n");
    return false;
}

