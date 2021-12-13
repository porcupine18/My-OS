/*
     File        : file.C

     Author      : Riccardo Bettati
     Modified    : 2021/11/28

     Description : Implementation of simple File class, with support for
                   sequential read/write operations.
*/

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "assert.H"
#include "console.H"
#include "file.H"

/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR/DESTRUCTOR */
/*--------------------------------------------------------------------------*/

File::File(FileSystem* _fs, int _id){
    Console::puts("File  -> Constructor: start- opening file\n");
    
    this->name_id = _id;                                    // get file's name
    this->filesystem = _fs;                                 // get file's related filesystem
    this->file_inode = this->filesystem->LookupFile(_id);   // get file's related inode
    assert(this->file_inode);

    short block_id = this->file_inode->block_id;               // get block_id of current file

    unsigned char tmp_cache[512];
    this->filesystem->disk->read(block_id, tmp_cache); // load data into cache

    memset(this->block_cache, '\0', SimpleDisk::BLOCK_SIZE);   // clean cache before writing to it
    memcpy(this->block_cache, tmp_cache, this->file_inode->file_size);

    if(this->file_inode->file_size == 0){
        this->seek_position = 0;
    }

    Console::puts("File  -> Constructor: intial Cache = \""); Console::puts((const char*)this->block_cache); Console::puts("\"\n");
    Console::puts("File  -> Constructor: seek = "); Console::puti(this->seek_position); Console::puts("\n");
    Console::puts("File  -> Constructor: file opened!\n");
}

File::~File() {
    // commit changes to file
    this->filesystem->disk->write(this->file_inode->block_id, this->block_cache);

    // clean cache
    memset(this->block_cache, '\0', SimpleDisk::BLOCK_SIZE);

    Console::puts("File  -> Destructor: Cleared cache, closing file\n");
    /* Make sure that you write any cached data to disk. */
    /* Also make sure that the inode in the inode list is updated. */
}

/*--------------------------------------------------------------------------*/
/* FILE FUNCTIONS */
/*--------------------------------------------------------------------------*/

int File::Read(unsigned int _n, char *_buf) {
    Console::puts("File  -> Read: start\n");

    // input checking
    if(_n < 1 || _n > 512){
        Console::puts("File  -> Read: invalid number of bytes asked to read\n");
        return 0;
    }

    // check how many bytes to read without overshooting
    int to_read;
    int max_read = this->file_inode->file_size - this->seek_position;
    if(_n > max_read){
        to_read = max_read;
    } else{
        to_read = _n;
    }

    // read from the file cache, update seek position
    memcpy(_buf, this->block_cache+this->seek_position, to_read);
    _buf[to_read]='\0';
    this->seek_position += to_read;

    Console::puts("File  -> Read: size=");  Console::puti(this->file_inode->file_size);
    Console::puts(";  seek=");              Console::puti(this->seek_position);
    Console::puts(";  max_read=");          Console::puti(max_read);
    Console::puts(";  can_read=");          Console::puti(to_read);
    Console::puts(";  asked_to_read=");     Console::puti(_n);                          
    Console::puts("\n");

    Console::puts("File  -> Read: read from cache= \""); Console::puts(_buf); Console::puts("\"\n");

    Console::puts("File  -> Read: DONE\n");

    return to_read;
}

int File::Write(unsigned int _n, const char *_buf) {
    Console::puts("File  -> Write: start\n");
    
    // input checking
    if(_n < 1 || _n > 512){
        Console::puts("File  -> Write: invalid number of bytes asked to write\n");
        return 0;
    }
    if(! *_buf){
        Console::puts("File  -> Write: NULL pointer passed\n");
        return 0;
    }

    Console::puts("File  -> Write: to write=\""); Console::puts(_buf); Console::puts("\"\n");

    // check how many bytes to write without buffer overflow
    int to_write;
    int max_write = SimpleDisk::BLOCK_SIZE - this->file_inode->file_size;
    if(_n > max_write){
        to_write = max_write;
    } else{
        to_write = _n;
    }

    Console::puts("File  -> Write: init Cache = \""); Console::puts((const char*)this->block_cache); Console::puts("\"\n");

    Console::puts("File  -> Write: size=");  Console::puti(this->file_inode->file_size);
    Console::puts(";  seek=");               Console::puti(this->seek_position); 
    Console::puts(";  max_write=");          Console::puti(max_write);
    Console::puts(";  will_write=");         Console::puti(to_write);
    Console::puts(";  told to write=");      Console::puti(_n);Console::puts("\n");

    // write to file , update size in inode, update seek position
    memcpy(this->block_cache+this->seek_position, _buf, to_write);
    if(seek_position + to_write < 512){
        this->block_cache[seek_position+to_write+1] = '\0';
    }

    this->seek_position += to_write;
    this->file_inode->file_size += to_write;


    Console::puts("File  -> Write: new  Cache = \""); Console::puts((const char*)this->block_cache); Console::puts("\"\n");

    Console::puts("File  -> Write: DONE\n");
    return to_write;    
}

void File::Reset() {
    this->seek_position = 0;
    Console::puts("File  -> Reset: Done\n");
}

bool File::EoF() {
    
    if(this->seek_position == this->file_inode->file_size){
        Console::puts("File  -> EOF: reached EOF\n");
        return true;
    } else{
        Console::puts("File  -> EOF: not reached EOF\n");
        return false;
    }
}
