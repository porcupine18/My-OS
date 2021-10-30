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

            Console::puts("     -> in constructor\n");

    /*_______ register new VMPool_______*/
    this->_page_table->register_pool(this);
            Console::puts("     -> constructor: registered VMPool in linked list\n");

    /*_______initialize instance variables_______*/
    this->_base_address = _base_address;
    this->_size         = _size;
    this->_frame_pool   = _frame_pool;
    this->_page_table   = _page_table;
            Console::puts("     -> constructor: base_address        = ");Console::puti(this->_base_address);Console::puts("\n");
            Console::puts("     -> constructor: size                = ");Console::puti(this->_size);Console::puts("\n");
            Console::puts("     -> constructor: initialized instance variables\n");

    /*_______create free and alloc lists and initialze the lists_______*/
    // get pointers for each of the arrays
    this->freelist_start_arr = (unsigned long*) this->_base_address;
    this->freelist_end_arr   = (unsigned long*) (this->_base_address + (PAGE_SIZE/2));
                //Console::puts("         -> initialized free list addr\n");

    this->alloclist_start_arr = (unsigned long*) (this->_base_address + PAGE_SIZE);
    this->alloclist_end_arr   = (unsigned long*) (this->_base_address + (PAGE_SIZE+PAGE_SIZE/2));
                //Console::puts("         -> initialized alloc list addr\n");


                Console::puts("     -> constructor: freelist_start_arr  = ");Console::puti((unsigned int)this->freelist_start_arr);Console::puts(" + 2047 = ");Console::puti((unsigned int)(this->freelist_start_arr) + 2047);Console::puts("\n");
                Console::puts("     -> constructor: freelist_end_arr    = ");Console::puti((unsigned int)this->freelist_end_arr);Console::puts(" + 2047 = ");Console::puti((unsigned int)(this->freelist_end_arr) + 2047);Console::puts("\n");
                Console::puts("     -> constructor: alloclist_start_arr = ");Console::puti((unsigned int)this->alloclist_start_arr);Console::puts(" + 2047 = ");Console::puti((unsigned int)(this->alloclist_start_arr) + 2047);Console::puts("\n");
                Console::puts("     -> constructor: alloclist_end_arr   = ");Console::puti((unsigned int)this->alloclist_end_arr);Console::puts(" + 2047 = ");Console::puti((unsigned int)(this->alloclist_end_arr) + 2047);Console::puts("\n");



    //initialize arrays
    
    // setting all elements to 0
    for (unsigned int i = 0; i < 512; i++){
        freelist_start_arr  [i] = NULL;
        freelist_end_arr    [i] = NULL;
        alloclist_start_arr [i] = NULL;
        alloclist_end_arr   [i] = NULL;
    }
                Console::puts("     -> cleaned lists\n");
    
    freelist_start_arr[0] = this->_base_address + (2*PAGE_SIZE);
    freelist_end_arr[0]   = this->_base_address + this->_size -1;

    alloclist_start_arr[0] = this->_base_address;
    alloclist_end_arr[0]   = this->_base_address + (2*PAGE_SIZE) - 1;
                Console::puts("     -> initialized free and alloc lists\n");
                
                Console::puts("     -> constructor: Free [0] = ");Console::puti((unsigned int)this->freelist_start_arr[0]);Console::puts(" ->"); Console::puti((unsigned int)this->freelist_end_arr[0]);Console::puts("\n");
                Console::puts("     -> constructor: Alloc[0] = ");Console::puti((unsigned int)this->alloclist_start_arr[0]);Console::puts(" ->"); Console::puti((unsigned int)this->alloclist_end_arr[0]);Console::puts("\n");

    Console::puts("     ~~~~~~~~~~~~~~~~~ Constructed new VMPool ~~~~~~~~~~~~~~~~~\n");    
}

unsigned long VMPool::allocate(unsigned long _size) {

            Console::puts("     -> allocate: start\n");

    /*_______ convert size to pages (ceiling) _______*/
    unsigned int pages_to_alloc = _size/PAGE_SIZE + (_size%PAGE_SIZE != 0);
            Console::puts("     -> allocate: pages_to_alloc = ");Console::puti(pages_to_alloc);Console::puts("\n");

    /*_______ initialize free and alloc lists _______*/
    bool found_free_region_free = false;

    unsigned int idx = 0;
    
    unsigned int j = 0; 
    // check all non-empty free list elements
    while( !((this->freelist_start_arr[idx]==0) && (this->freelist_end_arr[idx]==0)) && idx<512){

                Console::puts("     -> allocate:     Free [");Console::puti(idx);Console::puts("] = ");Console::puti((unsigned int)this->freelist_start_arr[idx]);Console::puts(" -> "); Console::puti((unsigned int)this->freelist_end_arr[idx]);Console::puts("\n");

        // found area in free list to use
        if( ((this->freelist_end_arr[idx] - this->freelist_start_arr[idx])/PAGE_SIZE) > pages_to_alloc ){
            found_free_region_free = true;
                Console::puts("     -> allocate: ^^^^^found^^^^^\n");

            // find right index in alloc list to insert new region
            j = 0; 

            while(j<512 && !(this->freelist_start_arr[j] == 0 && this->freelist_end_arr[j] == 0)){
                j++;
            }

                Console::puts("     -> allocate: use Alloc[");Console::puti(j);Console::puts("] = ");Console::puti((unsigned int)this->alloclist_start_arr[j]);Console::puts(" -> "); Console::puti((unsigned int)this->alloclist_end_arr[j]);Console::puts("\n");

            this->alloclist_start_arr[j] = this->freelist_start_arr[idx];
            this->alloclist_end_arr[j]   = this->freelist_start_arr[idx] + (pages_to_alloc * PAGE_SIZE) - 1;

            // take off memory from found region
            this->freelist_start_arr[idx] = this->freelist_start_arr[idx] + (pages_to_alloc * PAGE_SIZE);

            Console::puts("     -> allocate: now Alloc[");Console::puti(j);Console::puts("] = ");Console::puti((unsigned int)this->alloclist_start_arr[j]);Console::puts(" -> "); Console::puti((unsigned int)this->alloclist_end_arr[j]);Console::puts("\n");
            Console::puts("     -> allocate: now Free [");Console::puti(idx);Console::puts("] = ");Console::puti((unsigned int)this->freelist_start_arr[idx]);Console::puts(" -> "); Console::puti((unsigned int)this->freelist_end_arr[idx]);Console::puts("\n");

        }

        idx++;
    }

    if(found_free_region_free == false){
        Console::puts("     -> allocate: NO VALID FREE REGION FOUND\n");
        return 0;
    }
    else{
        Console::puts("     -> allocate: Done! Returning valid alloc start address\n");
        return this->alloclist_start_arr[j];
    }
}

void VMPool::release(unsigned long _start_address) {
    //assert(false);
    //Console::puts("Released region of memory.\n");
    Console::puts("Release skipped\n");
}

bool VMPool::is_legitimate(unsigned long _address) {

                Console::puts("             -> is_legitimate: base_address     =");Console::puti((unsigned int)this->_base_address);Console::puts("\n");
                Console::puts("             -> is_legitimate: first check end  =");Console::puti((unsigned int)(this->_base_address + PAGE_SIZE*2)-1);Console::puts("\n");
                Console::puts("             -> is_legitimate: checking address =");Console::puti((unsigned int)_address);Console::puts("\n");

    // if address belongs to free/alloc lists' region
    if((this->_base_address <= _address) && ((this->_base_address + 2*PAGE_SIZE) > _address)){
                Console::puts("             ~~~~~~~~~~~ is_legitimate: DONE (in Free/Alloc)~~~~~~~~~~~\n");
                
        return true; // validating first 2 pages
    }

                Console::puts("             -> is_legitimate: not in Free/Alloc\n");

    // if address is even in the region
    if( (this->_base_address+(2*PAGE_SIZE) <= _address) && ((this->_base_address + this->_size) > _address) ){
        unsigned int idx = 0;

        //iterating through all non-empty elements
        while( !((this->alloclist_start_arr[idx]==0) && (this->alloclist_end_arr[idx]==0)) && idx < 512){
        
            // found region where address belongs to
            if((_address >= this->alloclist_start_arr[idx]) && (_address < this->alloclist_end_arr[idx])){
                Console::puts("             ~~~~~~~~~~~ is_legitimate: DONE (in a VMPool)  ~~~~~~~~~~~\n");
                return true;
            }
            idx++;
        }
        Console::puts("             ~~~~~~~~~~~ is_legitimate: FAILED (in a VMPool) ~~~~~~~~~~\n");
        return false;
    }

    Console::puts("             ~~~~~~~~~~~~~~~~~ is_legitimate: FAILED  ~~~~~~~~~~~~~~~~~\n");
    return false;
}

