/*
    File: kernel.C

    Author: R. Bettati
            Department of Computer Science
            Texas A&M University
    Date  : 2021/11/28


    This file has the main entry point to the operating system.

    MAIN FILE FOR MACHINE PROBLEM "FILE SYSTEM"

*/

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

#define MB * (0x1 << 20)
#define KB * (0x1 << 10)

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "machine.H"         /* LOW-LEVEL STUFF   */
#include "console.H"
#include "gdt.H"
#include "idt.H"             /* EXCEPTION MGMT.   */
#include "irq.H"
#include "exceptions.H"     
#include "interrupts.H"

#include "assert.H"

#include "simple_timer.H"    /* TIMER MANAGEMENT  */

#include "frame_pool.H"      /* MEMORY MANAGEMENT */
#include "mem_pool.H"

#include "simple_disk.H"     /* DISK DEVICE */

#include "file_system.H"     /* FILE SYSTEM */
#include "file.H"

/*--------------------------------------------------------------------------*/
/* MEMORY MANAGEMENT */
/*--------------------------------------------------------------------------*/

/* -- A POOL OF FRAMES FOR THE SYSTEM TO USE */
FramePool * SYSTEM_FRAME_POOL;

/* -- A POOL OF CONTIGUOUS MEMORY FOR THE SYSTEM TO USE */
MemPool * MEMORY_POOL;

typedef long unsigned int size_t;

//replace the operator "new"
void * operator new (size_t size) {
    unsigned long a = MEMORY_POOL->allocate((unsigned long)size);
    return (void *)a;
}

//replace the operator "new[]"
void * operator new[] (size_t size) {
    unsigned long a = MEMORY_POOL->allocate((unsigned long)size);
    return (void *)a;
}

//replace the operator "delete"
void operator delete (void * p, size_t s) {
    MEMORY_POOL->release((unsigned long)p);
}


//replace the operator "delete[]"
void operator delete[] (void * p) {
    MEMORY_POOL->release((unsigned long)p);
}

/*--------------------------------------------------------------------------*/
/* DISK */
/*--------------------------------------------------------------------------*/

/* -- A POINTER TO THE SYSTEM DISK */
SimpleDisk * SYSTEM_DISK;

#define SYSTEM_DISK_SIZE (10 MB)

/*--------------------------------------------------------------------------*/
/* FILE SYSTEM */
/*--------------------------------------------------------------------------*/

/* -- A POINTER TO THE SYSTEM FILE SYSTEM */
FileSystem * FILE_SYSTEM;

/*--------------------------------------------------------------------------*/
/* CODE TO EXERCISE THE FILE SYSTEM */
/*--------------------------------------------------------------------------*/

void exercise_file_system(FileSystem * _file_system) {

    const char* str1 = "01234567890123456789";
    const char* str2 = "abcdefghijabcdefghij";
    const char* str3 = "999";
    const char* str4 = "zzz";
    const char* str5 = "11111111111111";
    const char* str6 = "bbbbbbbbbbbbbb";
    
    const char * STRING1;
    const char * STRING2;

    for(int i = 0; i < 3; i++){

    if(i==0){
        STRING1 = str1;
        STRING2 = str2;
    }
    if(i==1){
        STRING1 = str3;
        STRING2 = str4;
    }
    if(i==2){
        STRING1 = str5;
        STRING2 = str6;
    }
    
    /* -- Create two files -- */
    Console::puts("++++++++++++++++++++++++++++++ WILL CREATE FILES ++++++++++++++++++++++++++++++\n");
    assert(_file_system->CreateFile(1));
    assert(_file_system->CreateFile(2));
    Console::puts("++++++++++++++++++++++++++++++++ FILES CREATED ++++++++++++++++++++++++++++++++\n");

    /* -- "Open" the two files -- */
    
    {
        Console::puts("+++++++++++++++++++++++++++++++ 1:WILL OPEN FILE 1 ++++++++++++++++++++++++++++\n");

        File file1(_file_system, 1);

        Console::puts("+++++++++++++++++++++++++++++++ 1:WILL OPEN FILE 2 ++++++++++++++++++++++++++++\n");
        File file2(_file_system, 2);

        /* -- Write into File 1 -- */
        Console::puts("+++++++++++++++++++++++++++++ WILL WRITE TO FILE 1 ++++++++++++++++++++++++++\n");
        file1.Write(20, STRING1);
        Console::puts("++++++++++++++++++++++++++++ 1:DONE WRITING TO FILE 1 +++++++++++++++++++++++++\n");

        /* -- Write into File 2 -- */
        Console::puts("+++++++++++++++++++++++++++++ 1:WILL WRITE TO FILE 2 ++++++++++++++++++++++++++\n");
        file2.Write(20, STRING2);
        Console::puts("++++++++++++++++++++++++++++ 1:DONE WRITING TO FILE 2 +++++++++++++++++++++++++\n");

        /* -- Files will get automatically closed when we leave scope  -- */
    }

    {
        Console::puts("+++++++++++++++++++++++++++++++ 2:WILL OPEN FILE 1 ++++++++++++++++++++++++++++\n");

        File file1(_file_system, 1);

        Console::puts("+++++++++++++++++++++++++++++++ 2:WILL OPEN FILE 2 ++++++++++++++++++++++++++++\n");
        File file2(_file_system, 2);

        /* -- Write into File 1 -- */
        Console::puts("+++++++++++++++++++++++++++++ 2:WILL WRITE TO FILE 1 ++++++++++++++++++++++++++\n");
        file1.Write(16, "hello mere bande");
        Console::puts("++++++++++++++++++++++++++++ 2:DONE WRITING TO FILE 1 +++++++++++++++++++++++++\n");

        /* -- Write into File 2 -- */
        Console::puts("+++++++++++++++++++++++++++++ 2:WILL WRITE TO FILE 2 ++++++++++++++++++++++++++\n");
        file2.Write(14, "bye mere bande");
        Console::puts("++++++++++++++++++++++++++++ 2:DONE WRITING TO FILE 2 +++++++++++++++++++++++++\n");
    }

    {   
        /* -- "Open files again -- */
        Console::puts("++++++++++++++++++++++++++++++ OPENING BOTH FILES +++++++++++++++++++++++++++++\n");
        File file1(_file_system, 1);
        File file2(_file_system, 2);
        Console::puts("++++++++++++++++++++++++++++++ DONE OPENING FILES +++++++++++++++++++++++++++++\n");


        /* -- Read from File 1 and check result -- */
        file1.Reset();
        char result1[30];
        assert(file1.Read(20, result1) == 20);
        for(int i = 0; i < 20; i++) {
             assert(result1[i] == STRING1[i]);
        }
        Console::puts("++++++++++++++++++++++++++++++++ FILE 1 TESTED ++++++++++++++++++++++++++++++++\n");
    
        /* -- Read from File 2 and check result -- */
        file2.Reset();
        char result2[30];
        assert(file2.Read(20, result2) == 20);
        for(int i = 0; i < 20; i++) {
            assert(result2[i] == STRING2[i]);
        }
        Console::puts("++++++++++++++++++++++++++++++++ FILE 2 TESTED ++++++++++++++++++++++++++++++++\n");

    
        /* -- "Close" files again -- */
    }

    /* -- Delete both files -- */
    assert(_file_system->DeleteFile(1));
    assert(_file_system->DeleteFile(2));
    Console::puts("++++++++++++++++++++++++++++++ DELETED BOTH FILES +++++++++++++++++++++++++++++\n");
    }
}

/*--------------------------------------------------------------------------*/
/* MAIN ENTRY INTO THE OS */
/*--------------------------------------------------------------------------*/

int main() {

    GDT::init();
    Console::init();
    IDT::init();
    ExceptionHandler::init_dispatcher();
    IRQ::init();
    InterruptHandler::init_dispatcher();

    Console::output_redirection(true);

    /* -- EXAMPLE OF AN EXCEPTION HANDLER -- */

    class DBZ_Handler : public ExceptionHandler {
      public:
      virtual void handle_exception(REGS * _regs) {
        Console::puts("DIVISION BY ZERO!\n");
        for(;;);
      }
    } dbz_handler;

    ExceptionHandler::register_handler(0, &dbz_handler);

    /* -- INITIALIZE MEMORY -- */
    /*    NOTE: We don't have paging enabled in this MP. */
    /*    NOTE2: This is not an exercise in memory management. The implementation
                of the memory management is accordingly *very* primitive! */

    /* ---- Initialize a frame pool; details are in its implementation */
    FramePool system_frame_pool;
    SYSTEM_FRAME_POOL = &system_frame_pool;
   
    /* ---- Create a memory pool of 256 frames. */
    MemPool memory_pool(SYSTEM_FRAME_POOL, 256);
    MEMORY_POOL = &memory_pool;

    /* -- MEMORY ALLOCATOR SET UP. WE CAN N01234567890123456789 have it to make sure that 
                 we enable interrupts correctly. If we forget to do it,
                 the timer "dies". */
const 
    SimpleTimer timer(100); /* t"abcdefghijabcdefghij"
      virtual void handle_interrupt(REGS * _regs) {
        // we do nothing here. Just consume the interrupt
      }
    } disk_silencer;

    InterruptHandler::register_handler(14, &disk_silencer);


    /* -- FILE SYSTEM -- */

    FILE_SYSTEM = new FileSystem();

    /* NOTE: The timer chip starts periodically firing as 
             soon as we enable interrupts.
             It is important to install a timer handler, as we 
             would get a lot of uncaptured interrupts otherwise. */  

    /* -- ENABLE INTERRUPTS -- */

     Machine::enable_interrupts();

    /* -- MOST OF WHAT WE NEED IS SETUP. THE KERNEL CAN START. */

    Console::puts("START\n");

    /* -- HERE WE STRESS TEST THE FILE SYSTEM -- */

    assert(FileSystem::Format(SYSTEM_DISK, (128 KB), FILE_SYSTEM)); // Don't try this at home!
    Console::puts("FORMAT DONE\n");

    /* This is a really small file system. This allows you to use a very crude
       implementation for the free block list. */
    
    assert(FILE_SYSTEM->Mount(SYSTEM_DISK)); // 'connect' disk to file system.
    Console::puts("MOUNT DONE\n");

    Console::puts("GOING TO EXERCISE\n");

    const char* str1 = "01234567890123456789";
    const char* str2 = "abcdefghijabcdefghij";
    const char* str3 = "999";
    const char* str4 = "zzz";
    const char* str5 = "1111111111111111111111111";
    const char* str6 = "bbbbbbbbbbbbbbbbbbbbbbbb";

    for(int j = 0; j<1; j++) {
        Console::puts("\n\nITERATION["); Console::puti(j); Console::puts("]: START ==========================================================================================\n");
        exercise_file_system(FILE_SYSTEM);
        //exercise_file_system(FILE_SYSTEM, str3, str4);
        //exercise_file_system(FILE_SYSTEM, str5, str6);
    }

    Console::puts("EXERCISE DONE==========================================================================================\n");


    /* -- AND ALL THE REST SHOULD FOLLOW ... */
 
    //assert(false); /* WE SHOULD NEVER REACH THIS POINT. */

    /* -- WE DO THE FOLLOWING TO KEEP THE COMPILER HAPPY. */
    return 1;
}
