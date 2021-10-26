/*
 *   File       : page_table.C
 *   Author     : Mehul Varma
 *   Date       : Fall 2021
 *   Description: Basic Paging
 */

#include "assert.H"
#include "exceptions.H"
#include "console.H"
#include "paging_low.H"
#include "page_table.H"


PageTable*     PageTable::current_page_table = NULL;
unsigned int   PageTable::paging_enabled     = 0;
ContFramePool* PageTable::kernel_mem_pool    = NULL;
ContFramePool* PageTable::process_mem_pool   = NULL;
unsigned long  PageTable::shared_size        = 0;

// Class Functions ===============================================================================================================

// initalize static variables to enable paging
void PageTable::init_paging(ContFramePool * _kernel_mem_pool,
                            ContFramePool * _process_mem_pool,
                            const unsigned long _shared_size)
{

   // ! check if init_paging called again !
   assert(kernel_mem_pool == NULL && process_mem_pool == NULL && shared_size == 0);

   // init static instance variables
   kernel_mem_pool = _kernel_mem_pool;
   process_mem_pool = _process_mem_pool;
   shared_size = shared_size;

   Console::puts("++++++++++++++++ Initialized Paging System ++++++++++++++++\n");
}

// constructor for enabling paging (initializes the page directory page and page table pages)
PageTable::PageTable()
{  
   /*_______get frames from kernel pool_______*/
   unsigned long* page_dir_ptr   = (unsigned long*) (this->kernel_mem_pool->get_frames(1) * PAGE_SIZE);    // 1 frame for 1 Directory page requested
   unsigned long* page_table_ptr = (unsigned long*) (this->kernel_mem_pool->get_frames(1) * PAGE_SIZE);    // 1 frame for 1 Table page requested

   
   /*_______init Kernel memory area in PDE and PTE_______*/
   
   // init all PDE entries to map to Table pages & set Valid and R/W bit
   unsigned long i;   
   
   page_dir_ptr[0] = ((unsigned long)page_table_ptr) | 3; // first PDE valid

   for(i = 1; i < ENTRIES_PER_PAGE; i++){
      page_dir_ptr[i] = 2; // other PDEs are supervisor level
   };
   
   // init all PTE entries of first frame to map to physical address & set Valid and R/W bit

   unsigned long virtual_address = 0x00000; // starting virtual address
   unsigned long* page_table_ptr_tmp = page_table_ptr;

   for(i = 0; i < ENTRIES_PER_PAGE; i++){
      
      page_table_ptr_tmp[i] = (virtual_address) | 3;

      virtual_address = virtual_address + 4096;
   }   

   /*_______initialize page_directory to frame of Page Directory_______*/
   this->page_directory = page_dir_ptr;

   Console::puts("++++++++++++++ Constructed Page Table object ++++++++++++++\n");
}


// load directory to CR3
void PageTable::load()
{
   // writing to CR3
   current_page_table = this;
   write_cr3((unsigned long)(this->page_directory));

   Console::puts("++++++++++++++++++++ Loaded page table ++++++++++++++++++++\n");
}

// set paging flags
void PageTable::enable_paging()
{
   // paging bit = 32nd bit => 31 index
   // set the paging bit in CR0 to 1
   paging_enabled = 1;
   write_cr0(read_cr0() | 0x80000000);

   Console::puts("+++++++++++++++++++++ Enabled  paging +++++++++++++++++++++\n");
}

// handles page faults, if page doesn't exist gets page from memory
void PageTable::handle_fault(REGS* _r)
{

   unsigned long virtual_address = read_cr2(); // 32 bit virtual address that caused fault

   unsigned long* curr_page_dir = (unsigned long *) read_cr3();
   //unsigned long* curr_page_dir = PDE_address(virtual_address);

   /*_______ extract 3 parts of logical address _______*/
   unsigned long dir_idx     = dir_ten_bits(virtual_address);
   unsigned long table_idx   = table_ten_bits(virtual_address);
   unsigned long offset_bits = offset_twelve_bits(virtual_address);

   /*_______ alloc new memory for page fault _______*/
   
   unsigned long* table_page_addr;   // address of Table page

   // make new Table page if it doesn't exist
   if((curr_page_dir[dir_idx] & 1) == 0){ // check Access bit = least significant bit in PDE

      // get new Table page from kernel pool
      unsigned long table_frame_num = current_page_table->kernel_mem_pool->get_frames(1);

      table_page_addr = (unsigned long*) (table_frame_num * PAGE_SIZE);

      // init pde and set in directory
      unsigned long new_pde = ((unsigned long)table_page_addr) | 3;
      curr_page_dir[dir_idx] = new_pde;

      // init all entries in Page Table page
      unsigned long v_addr = 0;
      for(int i = 0; i < ENTRIES_PER_PAGE; i++){
         table_page_addr[i] = (v_addr) | 4;
         v_addr = v_addr + 4096;
      }   

   }
      
   //get new frame for memory
   unsigned long memory_frame_num = current_page_table->process_mem_pool->get_frames(1); 

   // make new PTE for Page Table page
   unsigned long new_pte = memory_frame_num << 12;
   new_pte = new_pte | 3;

   // find address of relevant Page Table page
   table_page_addr = page_address(curr_page_dir[dir_idx]);      

   // init PTE in table page
   table_page_addr[table_idx] = new_pte;

   Console::puts("+++++++++++++++++++ Handled  page fault +++++++++++++++++++\n");

   return;
}                                

void PageTable::register_pool(VMPool * _vm_pool)
{
    assert(false);
    Console::puts("registered VM pool\n");
}

void PageTable::free_page(unsigned long _page_no) {
    assert(false);
    Console::puts("freed page\n");
}

// find PTE of a logical address
unsigned long* PageTable::PTE_address(unsigned long addr){
   
   unsigned long  tmp_addr = addr;
   
   tmp_addr = tmp_addr >> 10;
   tmp_addr = tmp_addr | 0xFFC00000; // 1024 : X : Y
   tmp_addr = tmp_addr >> 2;
   tmp_addr = tmp_addr << 2;
   
   return (unsigned long*) tmp_addr;
}

// find PDE of a logical address
unsigned long* PageTable::PDE_address(unsigned long addr){

   unsigned long  tmp_addr = addr;

   tmp_addr = tmp_addr >> 20;
   tmp_addr = tmp_addr | 0xFFFFF000; // 1024 : 1024 : X 
   tmp_addr = tmp_addr >> 2;
   tmp_addr = tmp_addr << 2; 

   return (unsigned long*) tmp_addr;
}



// Bit Manipulation Functions ===================================================================================================
unsigned long* page_address(unsigned long pde){ // PDE to page_address --> clearing the metadata
   unsigned long page_addr = pde ;

   page_addr = page_addr >> 12;
   page_addr = page_addr << 12;

   return ((unsigned long*)page_addr);
}

unsigned long dir_ten_bits(unsigned long v_addr){
   unsigned long dir_bits = v_addr & 0xFFC00000;

   dir_bits = dir_bits >> 22;

   return (dir_bits);
}

unsigned long table_ten_bits(unsigned long v_addr){
   unsigned long table_bits = v_addr & 0x3FF000;

   table_bits = table_bits >> 12;

   return (table_bits);
}

unsigned long offset_twelve_bits(unsigned long v_addr){
   unsigned long offset_bits = v_addr & 0xFFF;
   return (offset_bits);
}


void print_array_long(unsigned long* buf)
{
    Console::puts("----------------");
    unsigned long memory = *buf;
    for (unsigned long i = 0; i < 32; i++)
    {
        if(i%4 == 0){
            Console::puts("_");
        }

        Console::puti(memory >> i & 1);
        

    }
    Console::puts("----------------\n");

}