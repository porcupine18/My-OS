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


// static function
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

// constructor
PageTable::PageTable()
{  
   unsigned long* page_dir_ptr = NULL;
   unsigned long* page_table_ptr = NULL;

   unsigned long i;

   /*_______get frames from kernel pool_______*/
   page_dir_ptr   = (unsigned long*) (this->kernel_mem_pool->get_frames(1) * PAGE_SIZE);    // 1 frame for 1 Directory page requested
   page_table_ptr = (unsigned long*) (this->kernel_mem_pool->get_frames(1) * PAGE_SIZE);    // 1 frame for 1 Table page requested

   Console::puts("      Directory frame = "); Console::putui((unsigned int)((unsigned long)page_dir_ptr/PAGE_SIZE)); Console::puts("\n");
   Console::puts("      Table frame     = "); Console::putui((unsigned int)((unsigned long)page_table_ptr/PAGE_SIZE)); Console::puts("\n");

   /*_______init Kernel memory area in PDE and PTE_______*/
   
   // init all PDE entries to map to Table pages 
   // set Valid and R/W bit
   page_dir_ptr[0] = ((unsigned long)page_table_ptr) | 3;

   for(i = 1; i < ENTRIES_PER_PAGE; i++){
      page_dir_ptr[i] = 2; // attribute set to: supervisor level, read/write, not present(010 in binary)
   };
   
   // init all PTE entries of first frame to map to physical address 
   // set Valid and R/W bit

   unsigned long virtual_address = 0x00000; // starting virtual address
   unsigned long* page_table_ptr_tmp = page_table_ptr;

   for(i = 0; i < ENTRIES_PER_PAGE; i++){

      //Console::puts("idx="); Console::puti(i); Console::puts("  addr= 0d"); Console::puti((unsigned int) (virtual_address) | 3); Console::puts("\n");
      
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

void PageTable::handle_fault(REGS* _r)
{

   unsigned long virtual_address = read_cr2(); // 32 bit address that caused fault
   Console::puts("   fault addr= 0d"); Console::putui((unsigned int)(virtual_address)); Console::puts("\n");
   //Console::puts("   Virtual address\n");
   //print_array_long(&virtual_address);

   unsigned long* curr_page_dir = (unsigned long *) read_cr3();
   

   /*_______ extract 3 parts of logical address _______*/
   // bits converted into decimal numbers => using as index
   unsigned long dir_idx     = dir_ten_bits(virtual_address);
   unsigned long table_idx   = table_ten_bits(virtual_address);
   unsigned long offset_bits = offset_twelve_bits(virtual_address);

   Console::puts("   dir_idx     = "); Console::putui((unsigned int)(dir_idx)); Console::puts("    ");
   Console::puts("table_idx   = "); Console::putui((unsigned int)(table_idx)); Console::puts("     ");
   Console::puts("offset_bits = "); Console::putui((unsigned int)(offset_bits)); Console::puts("\n");


   /*_______ alloc new memory for page fault _______*/
   
   // if Directory entry is invalid for dir_idx
      // get new Table page from kernel pool
      // init PDE

   // goto Directory entry at dir_idx =>(sure to be valid now)
   // get new frame from process pool
   // get address of relevant Table page from Directory entry
   // init PTE in Table page at table_idx

   unsigned long* table_page_addr;   // address of Table page
   unsigned long memory_frame_num;  // frame number of new memory page
   unsigned long new_pte;           // new PTE in Table page

   // make new Table page if it doesn't exist

   //Console::puts("      Start -->\n");

   if((curr_page_dir[dir_idx] & 1) == 0){ // check Access bit = least significant bit in PDE

      Console::puts("      Making Table page-->\n");

      // get new Table page from kernel pool
      unsigned long table_frame_num = current_page_table->kernel_mem_pool->get_frames(1);
      Console::puts("      table frame num = "); Console::putui((unsigned int)(table_frame_num)); Console::puts("\n");


      table_page_addr = (unsigned long*) (table_frame_num * PAGE_SIZE);

      // init pde and set in directory
      unsigned long new_pde = ((unsigned long)table_page_addr) | 3;
      curr_page_dir[dir_idx] = new_pde;

      //Console::puts("   PDE :\n");
      //print_array_long(&new_pde);

      // init all entried in Table page
      unsigned long v_addr = 0;
      for(int i = 0; i < ENTRIES_PER_PAGE; i++){
         table_page_addr[i] = (v_addr) | 4;
         v_addr = v_addr + 4096;
      }   

   }
      
   //Console::puts("      Making Memory page-->\n");

   //get new frame for memory
   memory_frame_num = current_page_table->process_mem_pool->get_frames(1); 
   //Console::puts("      memory frame num = "); Console::putui((unsigned int)(memory_frame_num)); Console::puts("\n");

   // make PTE for Table page
   new_pte = memory_frame_num << 12;
   new_pte = new_pte | 3;

   // find address of relevant Table page
   table_page_addr = page_address(curr_page_dir[dir_idx]);      

   // init PTE in table page
   table_page_addr[table_idx] = new_pte;

   //Console::puts("   PTE:\n");
   //print_array_long(&new_pte);

   Console::puts("+++++++++++++++++++ Handled  page fault +++++++++++++++++++\n");

   return;
}                                

unsigned long* page_address(unsigned long pde){
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