/*
 *   File       : page_table.C
 *   Author     : Mehul Varma
 *   Date       : Fall 2021
 *   Description: Basic Paging
 */

/* DEFINES -----------------------------------------------------------------*/

/* INCLUDES ----------------------------------------------------------------*/
#include "assert.H"
#include "exceptions.H"
#include "console.H"
#include "paging_low.H"
#include "page_table.H"

/* DATA STRUCTURES ---------------------------------------------------------*/
static VMPool* vm_pool_head; // head of linkedlist of VMPools
static VMPool* vm_pool_tail; // tail of linkedlist of VMPools

PageTable*     PageTable::current_page_table = NULL;
unsigned int   PageTable::paging_enabled     = 0;
ContFramePool* PageTable::kernel_mem_pool    = NULL;
ContFramePool* PageTable::process_mem_pool   = NULL;
unsigned long  PageTable::shared_size        = 0;

/* CONSTANTS ---------------------------------------------------------------*/

/* FORWARDS ----------------------------------------------------------------*/


/* CLASS: Page Table -------------------------------------------------------*/

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
// PAGING NOT TURNED ON YET
PageTable::PageTable()
{  
   /*_______get frames from kernel pool_______*/
   // getting physical address from process mem pool
   unsigned long* page_dir_ptr      = (unsigned long*) (this->process_mem_pool->get_frames(1) * PAGE_SIZE);    // 1 frame for 1 Directory page requested
   unsigned long* page_table_pg_ptr = (unsigned long*) (this->process_mem_pool->get_frames(1) * PAGE_SIZE);    // 1 frame for 1 Table page requested

   
   /*_______init Kernel memory area in PDE_______*/
   // init all PDE entries to map to Table pages & set Valid and R/W bit
   unsigned long i;   
   
   page_dir_ptr[0] = ((unsigned long)page_table_pg_ptr) | 3; // first PDE points to first PTP and valid

   for(i = 1; i < ENTRIES_PER_PAGE; i++){
      page_dir_ptr[i] = 2; // other PDEs are supervisor level
   };

   page_dir_ptr[ENTRIES_PER_PAGE - 1] = ((unsigned long)page_dir_ptr) | 3; // last PDE points to itself and valid
   
   /*_______init Kernel memory area in PTE_______*/
   // init all PTE entries of first frame to map to physical address & set Valid and R/W bit

   unsigned long virtual_address = 0x00000; // starting virtual address
   unsigned long* page_table_ptr_tmp = page_table_pg_ptr;

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
         //Console::puts("   fault addr= 0d"); Console::putui((unsigned int)(virtual_address)); Console::puts("\n");

   /*_______ check if virtual address is legitimate for any of the VMPools _______*/
   // iterating through linkedlist of VMPools
   
   VMPool* curr = vm_pool_head;
   
   Console::puts("      -> handle_fault: vm_pool_head = ");Console::puti((unsigned int)vm_pool_head);Console::puts("\n");
   Console::puts("      -> handle_fault: base_address = ");Console::puti((unsigned int)vm_pool_head->_base_address);Console::puts("\n");
   
   bool vaddr_legit = false;
   while(curr != NULL){
      if(curr->next->is_legitimate(virtual_address)){
         vaddr_legit = true;
      }
      curr = curr->next;
   }
   
   assert(vaddr_legit); // kernel aborting
   
   /*_______ get PTE and PDE of vddr that faulted _______*/
   unsigned long* pde_of_vaddr = PDE_address(virtual_address); // logical address of PDE for vaddress that page faulted
   unsigned long* pte_of_vaddr = PTE_address(virtual_address); // logical address of PTE for vaddress that page faulted

   /*_______ alloc new memory for page fault _______*/

         //Console::puts("      Start -->\n");

   // make new Table page if it doesn't exist
   if( ( *pde_of_vaddr & 1) == 0){ // check Access bit = least significant bit in PDE
            //Console::puts("      Making Table page-->\n");

      // get new Table page from kernel pool
      unsigned long new_ptp_frame_num_phy = current_page_table->process_mem_pool->get_frames(1);
            //Console::puts("      new PTP frame num phy = "); Console::putui((unsigned int)(new_ptp_frame_num_phy)); Console::puts("\n");

      unsigned long* new_ptp_addr_phy = (unsigned long*) (new_ptp_frame_num_phy * PAGE_SIZE);

      // init pde and set in directory
      unsigned long new_pde_value = ((unsigned long)new_ptp_addr_phy) | 3;
      *pde_of_vaddr = new_pde_value;

      // init all entries in new Page Table page
      unsigned long iter_vaddr = 0;
      unsigned long* new_ptp_vaddr = page_address(*pde_of_vaddr); // getting vaddr of ptp for iteration
            //Console::puts("   PDE :\n"); print_array_long(&new_pde_value);

      for(int i = 0; i < ENTRIES_PER_PAGE; i++){
         new_ptp_vaddr[i] = (iter_vaddr) | 4;
         iter_vaddr += 4096;
      }   

   }
         //Console::puts("      Making Memory page-->\n");

   //get new frame for memory
   unsigned long memory_frame_num = current_page_table->process_mem_pool->get_frames(1); 
         //Console::puts("      memory frame num = "); Console::putui((unsigned int)(memory_frame_num)); Console::puts("\n");

   // make new PTE for Page Table page
   unsigned long new_pte_value = memory_frame_num << 12;
   new_pte_value = new_pte_value | 3;

   // init PTE in table page
   *pte_of_vaddr = new_pte_value;
         //Console::puts("   PTE :\n"); print_array_long(&new_pte_value);

   Console::puts("+++++++++++++++++++ Handled  page fault +++++++++++++++++++\n");

   return;
}                                

void PageTable::register_pool(VMPool * _vm_pool)
{
   if(vm_pool_head == NULL){
      vm_pool_head = _vm_pool;
   }
   else{
      _vm_pool->next = NULL;
      vm_pool_tail->next = _vm_pool;
      vm_pool_tail = _vm_pool;
   }

   Console::puts("         ++++++++++ Registered new VM pool ++++++++++\n");
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