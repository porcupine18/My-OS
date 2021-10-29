/*
 *   File       : vm_pool.H
 *   Author     : Mehul Varma
 *   Date       : Fall 2021
 *   Description: Management of the Virtual Memory Pool
 */

#ifndef _VM_POOL_H_                   // include file only once
#define _VM_POOL_H_

/* DEFINES -----------------------------------------------------------------*/

/* INCLUDES ----------------------------------------------------------------*/
#include "utils.H"
#include "cont_frame_pool.H"
#include "page_table.H"

/* FORWARDS ----------------------------------------------------------------*/
/* Forward declaration of class PageTable */
/* We need this to break a circular include sequence. */
class PageTable;

/* HELPER FUNCTIONS --------------------------------------------------------*/

/* CLASS: VMPool -----------------------------------------------------------*/
class VMPool { /* Virtual Memory Pool */
private:

   unsigned long* freelist_start_arr;
   unsigned long* freelist_end_arr;

   unsigned long* alloclist_start_arr;
   unsigned long* alloclist_end_arr;
    

   unsigned long  _base_address;
   unsigned long  _size;
   ContFramePool *_frame_pool;
   PageTable     *_page_table;

public:
   
   VMPool* next; // next VMPool in linked list of VMPools

   static const unsigned int PAGE_SIZE        = PageTable::PAGE_SIZE;  			//in bytes
   static const unsigned int ENTRIES_PER_PAGE = PageTable::ENTRIES_PER_PAGE;  //in entries

   VMPool(unsigned long  _base_address,
          unsigned long  _size,
          ContFramePool *_frame_pool,
          PageTable     *_page_table);
   /* Initializes the data structures needed for the management of this
    * virtual-memory pool.
    * _base_address is the logical start address of the pool.
    * _size is the size of the pool in bytes.
    * _frame_pool points to the frame pool that provides the virtual
    * memory pool with physical memory frames.
    * _page_table points to the page table that maps the logical memory
    * references to physical addresses. */

   unsigned long allocate(unsigned long _size);
   /* Allocates a region of _size bytes of memory from the virtual
    * memory pool. If successful, returns the virtual address of the
    * start of the allocated region of memory. If fails, returns 0. */

   void release(unsigned long _start_address);
   /* Releases a region of previously allocated memory. The region
    * is identified by its start address, which was returned when the
    * region was allocated. */

   bool is_legitimate(unsigned long _address);
   /* Returns false if the address is not valid. An address is not valid
    * if it is not part of a region that is currently allocated. */

 };

#endif
