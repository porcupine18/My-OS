/*
 *   File       : page_table.H
 *   Author     : Mehul Varma
 *   Date       : Fall 2021
 *   Description: Basic Paging.
 */

#ifndef _page_table_H_                   // include file only once
#define _page_table_H_


/* DEFINES -----------------------------------------------------------------*/

/* INCLUDES ----------------------------------------------------------------*/
#include "machine.H"
#include "exceptions.H"
#include "cont_frame_pool.H"
#include "vm_pool.H"


/* FORWARDS ----------------------------------------------------------------*/
/* Forward declaration of class VMPool */
/* We need this to break a circular include sequence. */
class VMPool;


/* HELPER FUNCTIONS --------------------------------------------------------*/

unsigned long* page_address(unsigned long pde);
unsigned long dir_ten_bits(unsigned long v_addr);
unsigned long table_ten_bits(unsigned long v_addr);
unsigned long offset_twelve_bits(unsigned long v_addr);
void print_array_long(unsigned long* buf);


/* CLASS: PageTable --------------------------------------------------------*/
class PageTable {

private:

   /* THESE MEMBERS ARE COMMON TO ENTIRE PAGING SUBSYSTEM */
   static PageTable*      current_page_table; /* pointer to currently loaded page table object */
   static unsigned int    paging_enabled;     /* is paging turned on (i.e. are addresses logical)? */
   static ContFramePool*  kernel_mem_pool;    /* Frame pool for the kernel memory */
   static ContFramePool*  process_mem_pool;   /* Frame pool for the process memory */
   static unsigned long   shared_size;        /* size of shared address space */

   /* DATA FOR CURRENT PAGE TABLE */
   unsigned long*         page_directory;     /* where is page directory located? */

public:
   static const unsigned int PAGE_SIZE        = Machine::PAGE_SIZE;  			//in bytes
   static const unsigned int ENTRIES_PER_PAGE = Machine::PT_ENTRIES_PER_PAGE; //in entries

   static void init_paging(ContFramePool * _kernel_mem_pool,
						  ContFramePool * _process_mem_pool,
						  const unsigned long _shared_size);
   
   PageTable();
   
   void load();
   
   static void enable_paging();
   
   static void handle_fault(REGS* _r);
   
	void register_pool(VMPool * _vm_pool);
	/* Register a virtual memory pool with the page table. */
	
	void free_page(unsigned long _page_no);
	/* If page is valid, release frame and mark page invalid. */

	static unsigned long* PageTable::PTE_address(unsigned long addr);
	static unsigned long* PageTable::PDE_address(unsigned long addr);

};

#endif

