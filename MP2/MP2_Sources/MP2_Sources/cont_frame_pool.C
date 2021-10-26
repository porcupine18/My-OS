/*
 * File: ContFramePool.C 
 * Author: Mehul Varma
 * Date  : 9/24/2021 
 */

/* DEFINES ---------------------------------------------------------------------------*/
#define WORK_FREE 0
#define WORK_BUSY 1

#define HOS_NOT 0
#define HOS_YES 1

/* INCLUDES --------------------------------------------------------------------------*/
#include "cont_frame_pool.H"
#include "console.H"
#include "utils.H"
#include "assert.H"

/* DATA STRUCTURES--------------------------------------------------------------------*/

static ContFramePool* head = NULL;
static ContFramePool* tail = NULL;

/* CONSTANTS -------------------------------------------------------------------------*/
/* FORWARDS --------------------------------------------------------------------------*/


/* ===================================================================================*/
/* CLASS:   ContFramePool ============================================================*/


/* Constructor -----------------------------------------------------------------------*/
ContFramePool::ContFramePool(unsigned long _base_frame_no, // : number of first frame managed 
                             unsigned long _n_frames,      // : number of frames in pool
                             unsigned long _info_frame_no, // : frame number that has management info (bitmap)
                             unsigned long _n_info_frames) // : number of frames needed to store management info, 0 if stored internally
{
    
    // init instance variables ---------------------------------
    this->base_frame_no = _base_frame_no;
    this->nframes = _n_frames;
    this->info_frame_no = _info_frame_no;
    this->_n_info_frames = _n_info_frames;

    
    // set bitmaps ---------------------------------------------  
    // called from Kernel Pool --> info frame if first frame
    if(_info_frame_no == 0 && _n_info_frames == 0){
        Console::puts("******************** Kernel pool made ********************\n");
        bitmap_work = (unsigned char*)   ( this->base_frame_no * FRAME_SIZE);
        bitmap_hos  = bitmap_work + _n_frames/8 + 1;
    }

    // called from Process Pool --> info frame is external
    else{ 
        Console::puts("******************** Process pool made ********************\n");
        bitmap_work = (unsigned char*)  (_info_frame_no * FRAME_SIZE);
        bitmap_hos  =  bitmap_work + _n_frames / 8 + 1;
    }

    // init bitmap ---------------------------------------------
    // set all frames to free and not HOS        assert(get_bit(bitmap_work,i) == WORK_BUSY); // frame marking inaccessible is already busy!

    for(int i = 0; i < _n_frames; i++) {
        set_work_status(bitmap_work, i, WORK_FREE);
        set_hos_status(bitmap_hos, i, HOS_NOT);
    }
    
    // set info frame busy and first info frame HOS if called from kernel mode
    if(_info_frame_no == 0 && _n_info_frames == 0){
        set_work_status(bitmap_work, 0, WORK_BUSY);
        set_hos_status(bitmap_hos, 0, HOS_YES);        
    }

    
    // insert pool object in Linked List ---------------------------------
    if( head == NULL){
        head = this;
        tail = this;
        this->next = NULL;
    }
    else{
        this->next = NULL;
        tail->next = this;
        tail = this;
    }

    // trav linked list
    ContFramePool* curr = head;
    //Console::puts("         Head = "); Console::puti((const int) head); Console::puts("\n");
    //Console::puts("         Tail = "); Console::puti((const int)tail); Console::puts("\n");

    int i = 0;
    do{
        //Console::puts("         "); Console::puti(i); Console::puts(": "); Console::puti((const int) curr); Console::puts("\n");
        curr = curr->next;
        i++;
    } while(curr != NULL);
    
    Console::puts("     New Pool = "); Console::puti((const int) base_frame_no); Console::puts(":"); Console::puti((const int) base_frame_no + _n_frames ); Console::puts("\n"); 
}

/* Get Frames ------------------------------------------------------------------------*/
unsigned long ContFramePool::get_frames(unsigned int _n_frames)
{

    //Console::puts("Frames to get = "); Console::puti(_n_frames); 

    assert(_n_frames > 0);
    assert(_n_frames < this->nframes);

    
    int found = -1; // -1: finding, 0: not found, 1: found

    // outer loop i
    for(unsigned long  i = 0; i+_n_frames < this->nframes; ){
        
        //Console::puts("   i= "); Console::puti(i); 

        found = -1; //start finding frames again

        // iterate if i busy
        if(get_bit(bitmap_work, i) == WORK_BUSY){
            //Console::puts("   -->i: index busy\n");
            i++;
            continue;
        }

        //Console::puts("   -->i: index free\n");

        // inside loop j
        for(unsigned long  j = i+1; j < i+_n_frames-1; j++){
        
            //Console::puts("             j= "); Console::puti(j);

            if( get_bit(bitmap_work, j) == WORK_BUSY){
                //Console::puts("             -->j: index free\n");

                i = j+1; 
                found = 0; // not found
                break;
            }
            //Console::puts("             --> j: index free\n");
        }


        if (found == -1){ // finding and FOUND!

            //Console::puts("      Getting => Found free seq : "); Console::puti(this->base_frame_no+i); Console::puts("->"); Console::puti(this->base_frame_no+i+_n_frames-1); Console::puts("\n");

            found = 1;

            // making head HOS and reset busy
            set_hos_status(this->bitmap_hos, i, HOS_YES);        

            // set frames gotten busy
            for(int x = i; x < i + _n_frames; x++){                
                set_work_status(bitmap_work, x, WORK_BUSY);        
            }


            //Console::puts("         Work:"); print_array(bitmap_work,2);
            //Console::puts(" HOS :"); print_array(bitmap_hos,2);    
            //Console::puts("\n");


            return (i + this->base_frame_no);
        }

    }

    if(found == -1){
        Console::puts("     ContframePool::get_frames - Frame not found in pool\n");
        return 0;
    }


    Console::puts("     ContframePool::get_frames - Should not reach here!\n");
    assert(false);
    return 0; // to remove warnings
}

/* Mark Inaccessible PUBLIC ----------------------------------------------------------*/
void ContFramePool::mark_inaccessible(unsigned long _base_frame_no, // : starting frame number to mark
                                      unsigned long _n_frames)      // : number of frames to mark
{
    // range check    
    assert((_base_frame_no >= this->base_frame_no) && (_base_frame_no+_n_frames) <= (this->base_frame_no + this->nframes));

    for(unsigned long i = _base_frame_no; i < _base_frame_no + _n_frames; i++){

        assert(get_bit(bitmap_work,i) != WORK_BUSY); // frame marking inaccessible is already busy!

    }
    set_hos_status(this->bitmap_hos, _base_frame_no - this->base_frame_no, HOS_YES); // setting frames are busy
    
    for(unsigned long i = _base_frame_no - this->base_frame_no ; i < _n_frames; i++){
    
        set_work_status(this->bitmap_work, i, WORK_BUSY); // setting frames are busy

    }
    
    //Console::puts("         Work:"); print_array(bitmap_work,2);
    //Console::puts(" HOS :"); print_array(bitmap_hos,2);    
    //Console::puts("\n");   
    
}


/* Release Frames --------------------------------------------------------------------*/
void ContFramePool::release_frames(unsigned long _first_frame_no) // : frame number that is head of the sequence to be freed
{
    
    // find which frame pool it belongs to
    bool frame_found = false;

    ContFramePool* curr = head;

    unsigned char* bitmap_work;
    unsigned char* bitmap_hos; 

    unsigned long frame_idx;

    //Console::puts("Rel #"); Console::puti(_first_frame_no); Console::puts(" in pool:");

    while(true){

        if( (_first_frame_no >= curr->base_frame_no) && (_first_frame_no < (curr->base_frame_no + curr->nframes)) ){

            frame_found = true;     

            bitmap_work = curr->bitmap_work;
            bitmap_hos  = curr->bitmap_hos;
            frame_idx = _first_frame_no - curr->base_frame_no; // finding index of first frame in bitmap

            //Console::puts("("); Console::puti(curr->base_frame_no); Console::puts(":"); Console::puti(curr->base_frame_no + curr->nframes); Console::puts(")>");

            break;
        }
        
        if(curr->next != NULL){
            curr = curr->next;
            continue;
        }
        else{
            break;
        }
    }

    //Console::puts("=>");

    if(frame_found == false){
        Console::puts("\nContframePool::release_frames- requested frame to be released NOT FOUND\n");
        return;
    }

    // check if frame is busy or NOT hos
    assert(get_bit(bitmap_work, frame_idx) == WORK_BUSY && get_bit(bitmap_hos, frame_idx) == HOS_YES);

    //Console::puts(" FrameFound #"); Console::puti(frame_idx);

    // cleaning first frame
    set_hos_status(bitmap_hos, frame_idx, HOS_NOT);
    set_work_status(bitmap_work, frame_idx, WORK_FREE);

    // cleaning rest of the frames
    unsigned int i;
    for(i = frame_idx + 1;  (i < curr->nframes) && (get_bit(bitmap_work, i) == WORK_BUSY) && (get_bit(bitmap_hos, i) == HOS_NOT) ; i++){
        set_work_status(bitmap_work, i, WORK_FREE);
        set_hos_status(bitmap_hos, i, HOS_NOT);
    }

    //Console::puts("=>Seq"); Console::puti(_first_frame_no); Console::puts(":"); Console::puti(_first_frame_no + i - frame_idx - 1); Console::puts(" ="); Console::puti(i-frame_idx); Console::puts("\n");
    //Console::puts("         Work:"); print_array(bitmap_work,2);
    //Console::puts(" HOS :"); print_array(bitmap_hos,2);    
    //Console::puts("\n");

    return;
}


/* Needed Info Frames ----------------------------------------------------------------*/
unsigned long ContFramePool::needed_info_frames(unsigned long _n_frames) // : number of frames in the pool to be managed
{
        
    if (_n_frames <= 0){
        Console::puts("\nContframePool::info_frames- Zero or less frames requested\n");
        return 0;
    }

    // implementing 2 bits for each frame 
    // frames required = (n*2)/(4096*8)

    // ceiling
    return (_n_frames*2)/(FRAME_SIZE*8) + ((_n_frames*2)%(FRAME_SIZE*8)> 0 ? 1: 0);
}


/* Bit Manipulation Functions ========================================================*/

/* Set Work Status -------------------------------------------------------------------*/
void set_work_status(unsigned char* arr, int frame_idx, int state){

    assert(state < 2 && state > -1); //asserting state values used

    if(state == WORK_FREE){
        unset_bit(arr, frame_idx);        
    }
    else{
        set_bit(arr, frame_idx);                
    }
}

/* Set HOS Status --------------------------------------------------------------------*/
void set_hos_status(unsigned char* arr, int frame_idx, int state){
    
    assert(state < 2 && state > -1); //asserting state values used

    if(state == HOS_NOT){
        unset_bit(arr, frame_idx);        
    }else{
        set_bit(arr, frame_idx);                
    }
}

void set_bit(unsigned char* arr, int idx){
    arr[idx / 8] |= 1 << (idx % 8);
}

void unset_bit(unsigned char* arr, int idx){
    unsigned char * to_change = arr + (idx / 8);
    *to_change = *to_change & ~(1 << (idx % 8));
}

unsigned int get_bit(unsigned char* arr, int idx){
    return 1 & (arr[idx / 8] >> (idx % 8));
}

void print_array(unsigned char *buf, int char_count)
{
    int i;
    unsigned char * buffer = buf;
    for (i = 0; i < char_count; i++)
    {
        for (int j = 0; j < 8; j++) 
        {
            Console::puti(!!(((*buffer) << j) & 0x80));
        }
        Console::puts("_");
        buffer = buffer + 1;
    }
    Console::puts("     ");
};
