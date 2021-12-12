/* 
    File: file_system.H

    Author: R. Bettati
            Department of Computer Science
            Texas A&M University
    Date  : 21/11/28

    Description: Simple File System.
    

*/

#ifndef _FILE_SYSTEM_H_ // include file only once
#define _FILE_SYSTEM_H_

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "simple_disk.H"

/*--------------------------------------------------------------------------*/
/* FORWARDS */
/*--------------------------------------------------------------------------*/

class FileSystem;

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */
/*--------------------------------------------------------------------------*/

class Inode
{
  friend class FileSystem; // The inode is in an uncomfortable position between
  friend class File;       // File System and File. We give both full access
                           // to the Inode.

private:
  long id;        // File "name"
  long block_id;  // block in which file is stored
  long size;      // size of the file in bytes
  FileSystem* fs; // pointer to the file system

  /* You will need additional information in the inode, such as allocation 
     information. */

  // It may be handy to have a pointer to the File system.
  // For example when you need a new block or when you want
  // to load or save the inode list. (Depends on your
  // implementation.)

  /* You may need a few additional functions to help read and store the 
     inodes from and to disk. */
     
  Inode(long _id, long _block_id, long _size, FileSystem* _fs); //constuctor


};

/*--------------------------------------------------------------------------*/
/* FORWARD DECLARATIONS */
/*--------------------------------------------------------------------------*/

/* (none) */

/*--------------------------------------------------------------------------*/
/* F i l e S y s t e m  */
/*--------------------------------------------------------------------------*/

class FileSystem
{
  friend class File;
  friend class Inode;

private:

  unsigned int size; // size of the disk handled by the file system
  SimpleDisk *disk;  // disk the file system is related to

  // MAX_INODES in a single INODES block
  static constexpr unsigned int MAX_INODES        = (SimpleDisk::BLOCK_SIZE / sizeof(Inode)) -1;
  static constexpr unsigned int MAX_MAPPED_BLOCKS = SimpleDisk::BLOCK_SIZE / sizeof(unsigned char);

  Inode** inode_list;         // inode list
  unsigned char *free_list;   // free list, 1:BUSY, 0:FREE
  
  short GetFreeInode();       // returns free inode from inode list
  int   GetFreeBlock();       // returns the block id for a free block

  /* -- DEFINE YOUR FILE SYSTEM DATA STRUCTURES HERE. */

  /* The free-block list. You may want to implement the "list" as a bitmap. 
     Feel free to use an unsigned char to represent whether a block is free or not; 
     no need to go to bits if you don't want to. 
     If you reserve one block to store the "free list", you can handle a file system up to 
     256kB. (Large enough as a proof of concept.) */

  /* It may be helpful to two functions to hand out free inodes in the inode list and free
     blocks. These functions also come useful to class Inode and File. */



public:

  FileSystem();
  /* Just initializes local data structures. Does not connect to disk yet. */

  ~FileSystem();
  /* Unmount file system if it has been mounted. */

  bool Mount(SimpleDisk *_disk);
  /* Associates this file system with a disk. Limit to at most one file system per disk.
     Returns true if operation successful (i.e. there is indeed a file system on the disk.) */

  static bool Format(SimpleDisk *_disk, unsigned int _size, FileSystem* _fs);
  /* Wipes any file system from the disk and installs an empty file system of given size. */

  Inode* LookupFile(int _file_id);
  /* Find file with given id in file system. If found, return its inode. 
       Otherwise, return null. */

  bool CreateFile(int _file_id);
  /* Create file with given id in the file system. If file exists already,
     abort and return false. Otherwise, return true. */

  bool DeleteFile(int _file_id);
  /* Delete file with given id in the file system; free any disk block occupied by the file. */
};
#endif
