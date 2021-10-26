/*
 * File: ContFramePool.H 
 * Author: Mehul Varma
 * Date  : 9/24/2021 
 */

#ifndef _CONT_FRAME_POOL_H_                   // include file only once
#define _CONT_FRAME_POOL_H_

/* DEFINES ---------------------------------------------------------------------------*/
#define WORK_FREE 0
#define WORK_BUSY 1

#define HOA_NOT 0
#define HOA_YES 1

/* INCLUDES --------------------------------------------------------------------------*/
#include "machine.H"

/* DATA STRUCTURES--------------------------------------------------------------------*/

/* CLASS:   Bit Manipulation Functions ===============================================*/
void set_bit(unsigned char* arr, int idx);
void unset_bit(unsigned char* arr, int idx);
unsigned int get_bit(unsigned char* arr, int idx);

void set_work_status(unsigned char* arr, int frame_idx, int state);
void set_hos_status(unsigned char* arr, int frame_idx, int state);
void print_array(unsigned char *buf, int char_count);

/* ===================================================================================*/
/* CLASS:   ContFramePool ============================================================*/

class ContFramePool {
    
private:

    // linked list node next pointer
    ContFramePool* next;

    // bitmaps
    unsigned char * bitmap_work;      // bitmap_work pointer 
    unsigned char * bitmap_hos;       // bitmap_hos pointer 

    // arguments taken by the constructor
    unsigned long   base_frame_no;
    unsigned long   nframes;
    unsigned long   info_frame_no;
    unsigned long   _n_info_frames;


public:

    static const unsigned int FRAME_SIZE = Machine::PAGE_SIZE; // PAGE_SIZE = Frame Size

    ContFramePool(unsigned long _base_frame_no,
                  unsigned long _n_frames,
                  unsigned long _info_frame_no,
                  unsigned long _n_info_frames);
    
    unsigned long get_frames(unsigned int _n_frames);
    
    void mark_inaccessible(unsigned long _base_frame_no,
                           unsigned long _n_frames);

    static void release_frames(unsigned long _first_frame_no);
    
    static unsigned long needed_info_frames(unsigned long _n_frames);

    void linked_list_trav_check();

};
#endif
