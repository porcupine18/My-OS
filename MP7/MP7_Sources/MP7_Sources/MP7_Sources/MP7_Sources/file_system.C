/*
    File        : file_system.C

    Author      : Mehul Varma

    Description : Implementation of simple File System class.
                  Has support for numerical file identifiers.
*/

/* DEFINES -----------------------------------------------------------------*/


/* INCLUDES ----------------------------------------------------------------*/
#include "assert.H"
#include "console.H"
#include "file_system.H"


/* CLASS: Inode ------------------------------------------------------------*/
    /* You may need to add a few functions, for example to help read and store inodes from and to disk. */
Inode::Inode(long _id, long _block_id, long _size, FileSystem* _fs){
    this->id = _id;
    this->block_id = _block_id;
    this->size = _size;
    this->fs = fs;
}
    


/* CLASS FileSystem --------------------------------------------------------*/

/* CONSTRUCTOR */
FileSystem::FileSystem(){ /* Just initializes local data structures . Does not connect to disk yet. */
    this->disk = NULL;
    this->size = 0;
    this-> inode_list = NULL;
    this-> free_list = NULL;

    Console::puts("++++++++++ FileSystem Constructed ++++++++++\n");
}

/* DESTRUCTOR */
FileSystem::~FileSystem() {
    Console::puts("---------- FileSystem Unmounted ----------\n");
    /* Make sure that the inode list and the free list are written back to the disk. */
    assert(false);
}


/* FILE SYSTEM FUNCTIONS*/

/*  Associates this file system with a disk. Limit to at most one file system per disk
    Returns true if operation successful (i.e. there is indeed a file system on the disk .) 
    Here you read the inode list and the free list into memory.
*/

bool FileSystem::Mount(SimpleDisk * _disk) {
    Console::puts("     -> Mount: start\n");

    // check if disk input is valid
    if(!_disk){
        Console::puts("     -> Mount: FAILED!\n");
        return false;
    }

    // assign disk
    this->disk = _disk;

    // read inode and free lists from disk
    this->disk->read(0, (unsigned char*)inode_list);
    this->disk->read(1, free_list);

    Console::puts("     -> Mount: char inode_list=");Console::puts((const char*)free_list); Console::puts("\n");

    for(int i=0; i<MAX_INODES; i++){
        Console::puts("     -> Mount: inode idx=");Console::puti(i);  Console::puts("; file_id="); Console::puti(this->inode_list[i]->block_id); Console::puts("; block_id="); Console::puti(this->inode_list[i]->block_id); Console::puts("; size="); Console::puti(this->inode_list[i]->size); Console::puts("; fs(max_inodes)="); Console::puti(this->inode_list[i]->fs->MAX_INODES); Console::puts("\n");
    }

    Console::puts("++++++++++ Mounting DONE ++++++++++\n");
    return true;
}

/*  Here you populate the disk with an initialized (probably empty) inode list
    and a free list. Make sure that blocks used for the inodes and for the free list
    are marked as used, otherwise they may get overwritten. */
/* Wipes any file system from the disk and installs an empty file system of given size . */
bool FileSystem::Format(SimpleDisk * _disk, unsigned int _size, FileSystem* _fs) {
    Console::puts("     -> Format: start\n");
    Console::puts("     -> Format: MaxInodes=");Console::puti(MAX_INODES);  Console::puts("\n");

    // make empty inode list for 1st block
    Inode* inode_buf[MAX_INODES];
    for(int i=2; i<MAX_INODES; i++){
        inode_buf[i] = new Inode((long)-1, (long)-1, (long)-1, _fs);
    }

    // set first inode busy
    inode_buf[0] = new Inode(-2, 0, 512, _fs); // inodes block marked
    inode_buf[1] = new Inode(-3, 1, 512, _fs); // freelist block marked


    for(int i=0; i<MAX_INODES; i++){
        Console::puts("     -> Format: inode idx=");Console::puti(i);  Console::puts("; file_id="); Console::puti(inode_buf[i]->id); Console::puts("; block_id="); Console::puti(inode_buf[i]->block_id); Console::puts("; size="); Console::puti(inode_buf[i]->size); Console::puts("; fs(max_inodes)="); Console::puti(inode_buf[i]->fs->MAX_INODES); Console::puts("\n");
    }

    // make empty free list for 2nd block
    unsigned char free_buf[SimpleDisk::BLOCK_SIZE];
    memset(free_buf,0,SimpleDisk::BLOCK_SIZE);

    // set first element to be busy
    free_buf[0] = 1; // 1 = BUSY, 0 = FREE

    // write empty inode list and free list to the disk
    _disk->write(0,(unsigned char*)inode_buf);
    _disk->write(1,free_buf);


    Console::puts("++++++++++ Formatting DONE ++++++++++\n");

    return true;
}

/* Here you go through the inode list to find the file. */
Inode* FileSystem::LookupFile(int _file_id) {
    Console::puts("         -> LookupFile: start\n");
    Console::puts("         -> LookupFile: looking up file with id="); Console::puti(_file_id); Console::puts("\n");

    // validate file id
    if(_file_id < 0){
        Console::puts("         -> LookupFile: No files with negative ids exists in this File System!\n");
        return NULL;
    }

    // iterate through inode list and return inode if file_id matches
    for(int i=0; i<MAX_INODES; i++){
        if(this->inode_list[i]->id == _file_id){
            Console::puts("         -> LookupFile: FOUND FILE! inode: block_id="); Console::puti(this->inode_list[i]->block_id); Console::puts("; name_id="); Console::puti(this->inode_list[i]->id); Console::puts("; size="); Console::puti(this->inode_list[i]->size); Console::puts("; fs(max_inodes)="); Console::puti((unsigned int)this->inode_list[i]->fs->MAX_INODES); Console::puts("\n");Console::puts("\n");
            return (this->inode_list[i]);
        }
    }

    // if code gets here, no match, return NULL
    Console::puts("         -> LookupFile: No file found\n");
    return NULL;
}

/*  Here you check if the file exists already. If so, throw an error.
    Then get yourself a free inode and initialize all the data needed for the
    new file. After this function there will be a new file on disk.                     */
bool FileSystem::CreateFile(int _file_id) { //assigning a free inode to the _file_id
    Console::puts("     -> CreateFile: start\n");
    Console::puts("     -> CreateFile: creating file with id="); Console::puti(_file_id); Console::puts("\n");

    // LookupFile: iterate through inode list to see if file exists
    Inode* file_inode = this->LookupFile(_file_id);

    // return false if file inode is valid
    if(file_inode){
        Console::puts("     -> CreateFile: FAILED-File Name already exists!\n");
        return false;
    }

    // get free inode's idx from inode list
    short inode_idx = GetFreeInode();

    // get idx of a free block from free list
    int freelist_idx = GetFreeBlock();

    // validate freelist and inode idxs
    if(inode_idx == -1 || freelist_idx == -1){
        Console::puts("     -> CreateFile: FAILED!!\n");
        return false;
    }

    // set inode and bitmap for new file
    this->inode_list[inode_idx] = new Inode(_file_id, freelist_idx, 0, this);
    this->free_list[freelist_idx] = 1; // set busy

    Console::puts("     -> CreateFile: FILE CREATED\n");
    return true;
}

/*  First, check if the file exists. If not, throw an error. 
    Then free all blocks that belong to the file and delete/invalidate 
    (depending on your implementation of the inode list) the inode. */
bool FileSystem::DeleteFile(int _file_id) {
    Console::puts("     -> DeleteFile: start");
    Console::puts("     -> DeleteFile: deleting file with id:"); Console::puti(_file_id); Console::puts("\n");

    // LookupFile: iterate through inode list to see if file exists
    Inode* file_inode = this->LookupFile(_file_id);
    if(!file_inode){
        Console::puts("     -> DeleteFile: FAILED-File does not exist!\n");
        return false;
    }

    // delete inode structure
    int block_id = file_inode->block_id;
    
    short inode_idx = -1;
    for(int i=0; i<MAX_INODES; i++){
        if(this->inode_list[i] == file_inode){
            inode_idx  = i;
        }
    }
    assert(inode_idx != -1); // cannot delete a file that doesn't belong to a inode, FATAL ERROR

    // resetting inode list and freelist
    this->inode_list[inode_idx]->id       = -1;
    this->inode_list[inode_idx]->block_id = -1;
    this->inode_list[inode_idx]->size     = -1;

    this->free_list[block_id] = 0; // set free

    Console::puts("     -> CreateFile: FILE DELETED!\n");
    return true;
}


/* returns the inode index*/
short FileSystem::GetFreeInode(){
    Console::puts("         -> GetFreeInode: start\n");

    // check to find free inode
    int i;
    for(i=0; i<MAX_INODES; i++){
        Console::puts("         -> GetFreeInode: finding free idx=");Console::puti(i); Console::puts("; block_id="); Console::puti(this->inode_list[i]->block_id); Console::puts("\n");
        if(this->inode_list[i]->block_id == -1){
            Console::puts("         -> GetFreeInode: FOUND free inode idx=");Console::puti(i); Console::puts("\n");
            return i;
        }
    }

    Console::puts("         -> GetFreeInode: FAILED! Reached max file limit (ran out of inodes)\n");
    return -1;
}

/* returns the block id for a free block */
int FileSystem::GetFreeBlock(){

    int block_id = 0;

    // iterate through freelist to find empty block and return block id
    for(block_id = 0; block_id < MAX_MAPPED_BLOCKS; block_id++){
        if(this->free_list[block_id] = -1){
            Console::puts("         -> GetFreeBlock: Found free block block_id=");Console::puti(block_id); Console::puts("\n");
            return block_id;
        }
    }

    // if reaches here, no free blocks, return -1
    Console::puts("         -> GetFreeBlock: FAILED-No free blocks found!\n");
    return -1;

}