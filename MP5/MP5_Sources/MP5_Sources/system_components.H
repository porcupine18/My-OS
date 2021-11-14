#include "scheduler.H"
#include "frame_pool.H"      /* MEMORY MANAGEMENT */
#include "mem_pool.H"

extern Scheduler * SYSTEM_SCHEDULER;

/* -- A POOL OF FRAMES FOR THE SYSTEM TO USE */
extern FramePool * SYSTEM_FRAME_POOL;

/* -- A POOL OF CONTIGUOUS MEMORY FOR THE SYSTEM TO USE */
extern MemPool * MEMORY_POOL;