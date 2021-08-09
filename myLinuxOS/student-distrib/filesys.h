#include "lib.h"
#include "types.h"
#include "x86_desc.h"



#define BLOCK_4KB 4096                  // absolute block num (4KB per block)
#define BOOT_SIZE 64                    // Boot Block has 64B for directory entries


#define FILENAME_MAX_LENGTH 32          // the max length of our filename is 32 (zero-padded)
#define DENTRY_NUM 63                   // directories have 63 files (actually 62 due to "." being the first entry)
#define RESERVE_BYTES 24                // allocate 24 bytes for our dentries page reserve value
#define ONEKB_PAGE_SIZE 1023            // 4kb page / 4B = 1024 - 1
#define BOOTBLK_RESERVE 52              // 52B reserved for our boot block in file system

#define FILE_FAIL -1                    // return value for failure
#define FILE_PASS 0                     // return value for pass


/* directory entry structure */
typedef struct dentry{
    char file_name[FILENAME_MAX_LENGTH];
    uint32_t type;          
    uint32_t inode;
    uint8_t reserved[RESERVE_BYTES];
} dentry_t;

typedef struct inode{
    uint32_t length;
    uint32_t data_block[ONEKB_PAGE_SIZE];
} inode_t;

typedef struct boot_blk {
    uint32_t total_dentries;
    uint32_t total_inodes;
    uint32_t total_datablock;
    uint8_t boot_blk_reserved[BOOTBLK_RESERVE];
    dentry_t dentries[DENTRY_NUM];
} boot_blk_t;


/* init filesys func */
void initialise_filesys();


/* file operations */
int32_t file_open();
int32_t file_close();
int32_t file_write();
int32_t file_read(int32_t fname, void *buf, int32_t nbytes);

/* directory operations */
int32_t dir_read(int32_t fd, void* buf, int32_t nbytes);
int32_t dir_open();
int32_t dir_close();
int32_t dir_write(); 

/* Read helper functions */
int32_t read_dentry_by_name(const uint8_t *fname, dentry_t *dentry);
int32_t read_dentry_by_index(uint32_t index, dentry_t *dentry);
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t *buf, uint32_t length);

/* Our created test functions */
int directory_test();
int file_test(int32_t fname);

/* Create variables for .c file to use */
uint32_t num_dir;
dentry_t den;
dentry_t *den_ptr;
inode_t *inode_ptr;
struct boot_blk *boot;
uint32_t filesys;
