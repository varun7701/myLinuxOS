#include "filesys.h"
#include "types.h"
#include "lib.h"
#include "x86_desc.h"
#include "syscall.h"



/* initialise_filesys()
 * Inputs: fs - File system starting address
 * Outputs: N/A
 * Side Effects: initializes filesystem and its pointers
*/
void initialise_filesys(uint32_t fs) {
    filesys = fs;
    boot = (boot_blk_t*)filesys;
    inode_ptr = &((inode_t*)filesys)[1];
    den_ptr = &((dentry_t*)boot)[1];

    num_dir = *((int32_t *)fs);
    
}

/* file_open()
 * Inputs: None
 * Outputs: N/A
 * Side Effects: unused function, no effect
*/
int32_t file_open() {
    return 0;
}


/* file_close()
 * Inputs: None
 * Outputs: N/A
 * Side Effects: unused function, no effect
*/
int32_t file_close() {
    return 0;
}

/* file_write()
 * Inputs: None
 * Outputs: N/A
 * Side Effects: READ-ONLY FILESYSTEM
*/
int32_t file_write() {
    return -1;
}

/* file_read()
 * Inputs: fname - file name
 *         buf - pointer to buffer with data to write to    
 *         nbytes - number of bytes to write
 * Outputs: Number of bytes read
 * Side Effects: read the data for respective dentry, return total # of bytes read
*/
int32_t file_read(int32_t fname, void *buf, int32_t nbytes) {

    int numread;
    uint32_t length = (uint32_t)nbytes;

    // validate args
    if (buf == NULL || nbytes < 0 || fname < 0 || fname > MAX_NUM_FD) {
        return -1;
    }

    pcb_t* pcb = (pcb_t*)(KPAGE_SIZE - (KSTACK_SIZE * (pid + 1)));

    // set numread equal to output of read_data
    numread = read_data(pcb->fd_array[fname].inode, pcb->fd_array[fname].file_pos, (uint8_t*)buf, length);

    if(numread == FILE_FAIL)
        return -1;
    else{

        // add the number of bytes read
        pcb->fd_array[fname].file_pos += numread;

        //return numread
        return numread;
    }

}

/* dir_read()
 * Inputs: buf - pointer to buffer with data to write to       
 *         nbytes - number of bytes to write
 * Outputs: PASS if no errors
 * Side Effects: retrieves filename and populates buffer
*/
int32_t dir_read(int32_t fd, void* buf, int32_t nbytes) {
    dentry_t dentry;

    pcb_t* pcb = (pcb_t*)(KPAGE_SIZE - (KSTACK_SIZE * (pid + 1)));
    
    // fill dentry using the file_pos (init. to 0)
    int32_t check_val = read_dentry_by_index(pcb->fd_array[fd].file_pos, &dentry);

    if(check_val == -1)
        return -1;

    // copy filename into buffer
    memcpy(buf, dentry.file_name, FILENAME_MAX_LENGTH);

    // increase file_pos for keeping variable accurate
    pcb->fd_array[fd].file_pos += 1;

    int32_t retval = strlen((int8_t*)(dentry.file_name));

    // cut filename down to max if it is too long
    if(retval > FILENAME_MAX_LENGTH)
        retval = FILENAME_MAX_LENGTH;

    return retval;
}

/* dir_open()
 * Inputs: None
 * Outputs: N/A
 * Side Effects: unused function, no effect
*/
int32_t dir_open() {
    return 0;
}

/* dir_close()
 * Inputs: None
 * Outputs: N/A
 * Side Effects: unused function, no effect
*/
int32_t dir_close() {
    return 0;
}



/* dir_write()
 * Inputs: None
 * Outputs: N/A
 * Side Effects: READ-ONLY FILESYSTEM
*/
int32_t dir_write() {
    return -1;
}


/* read_dentry_by_name()
 * Inputs: fname - file name
 *         dentry - the current dentry to be accessed and filled
 * Outputs: PASS/FAIL
 * Side Effects: populates the dentry that is passed in the argument
*/
int32_t read_dentry_by_name(const uint8_t *fname, dentry_t *dentry) {
    int i;
    
    // validate arguments
    if(fname == NULL || dentry == NULL || strlen((int8_t*)(fname)) > FILENAME_MAX_LENGTH) {
        return -1;
    }

    // find file matching the given name and copy data into arg
    for(i=0; i < BOOT_SIZE; i++) {
        if(strncmp((int8_t*)fname, (int8_t*)den_ptr[i].file_name, FILENAME_MAX_LENGTH) == 0){
            memcpy(dentry->file_name, den_ptr[i].file_name, sizeof(den_ptr[i].file_name));
            dentry->type = den_ptr[i].type;
            dentry->inode = den_ptr[i].inode;
            memcpy(dentry->reserved, den_ptr[i].reserved, sizeof(den_ptr[i].reserved));
            return 0;
       }
    }
   return -1;
}


/* read_dentry_by_index()
 * Inputs: index - dentry index
 *         dentry - the current dentry to be accessed and filled
 * Outputs: PASS/FAIL
 * Side Effects: populates the dentry that is passed in the argument
*/
int32_t read_dentry_by_index(uint32_t index, dentry_t *dentry) {
    dentry_t *cur_dentry;

    // validate arguments
    if(dentry == NULL || index > boot->total_dentries || index < 0) {
        return -1;
    }

    // populate cur_dentry with current index value and copy to arg passed in
    cur_dentry = &(boot->dentries[index]);
    *dentry = *cur_dentry;


    return 0;
}


/* read_data()
 * Inputs: inode - the inode value for its specific block
 *         offset - the offset value from beginning
 *         buf - pointer to buffer where we will copy data into
 *         length - number of bytes to write
 * Outputs: num_bytes_read if successful, -1 if fails.
 * Side Effects: retrieve the data from a specific file given by the inode
*/
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t *buf, uint32_t length) {

    // create variables
    int i;
    uint32_t total_filelength, num_bytes_read, total_dblocks, curr_byte_loc;

    // get the start values for where we are going to be accessing in our datablocks
    uint32_t dblock_location = offset / BLOCK_4KB;
    uint32_t dblock_data_location = offset % BLOCK_4KB;
    total_dblocks = boot->total_datablock;

    // a 0-length argument should return 0 rather than -1 indicating fail  (could even printf here to note)
    if(length == 0)
        return 0;

    // argument validity check on all four args passed in 
    if(inode == NULL || buf == NULL || inode >= boot->total_inodes || offset < 0 || length < 0 || inode < 0) {
        return -1;
    }

    // initial locations of our inode
    inode_t* curr_inode = (inode_t*)((uint32_t)boot + (inode + 1) * BLOCK_4KB);
    
    // init num_bytes_read and get total_filelength
    num_bytes_read = 0;
    total_filelength = curr_inode->length;

    // get where the initial location where data blocks start
    uint8_t* data_start = (uint8_t*)((uint32_t)boot + (boot->total_inodes + 1) * BLOCK_4KB);

    // grab the starting data node that we are reading from, curr_data is redundant as we set it inside our for loop below
    uint32_t data_index_val = curr_inode->data_block[dblock_location];
    uint8_t* curr_data_index = (uint8_t*)(data_start + data_index_val * BLOCK_4KB);

    for(i = 0; i < length; i++){

        uint32_t loop_file_length = (offset + i);

        // if we are over the total file length then we return the # of bytes read up to this point
        if(loop_file_length >= total_filelength)        
            return num_bytes_read;

        // if we go over the byte index then we update the data_block index to move to the next data block
        if(dblock_data_location >= BLOCK_4KB){      
            dblock_data_location = 0;
            dblock_location++;
        }

        // same as getting data_index_val, but needs to consider if dblock_location gets incremented
        curr_byte_loc = curr_inode->data_block[dblock_location];        
        curr_data_index = (uint8_t*)(data_start + curr_byte_loc * BLOCK_4KB);

        // fill the buffer with the data from our dblock_data locaiton and the current index
        buf[i] = *(curr_data_index + dblock_data_location);    

        // increment the values of dblock_data location if we read output correctly, inc num_bytes
        dblock_data_location++; 
        num_bytes_read++;

    }

    
    // return total # of bytes read overall
    return num_bytes_read;

}




/* directory_test
 *
 * Asserts that the directory file names and sizes can be read
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: displays file list and sizes of the directory
 * Coverage: dir_read, read_dentry_by_index()
 * Files: filesys.c/.h, tests.c/.h
*/
int directory_test() {
     clear();
    int i, j;

    uint8_t buffer[32];
    dentry_t* current_dentry;

     for(i=0; i<num_dir; i++)  {

        current_dentry = (dentry_t*)&(boot->dentries[i]);

        printf("file_name: ");

        dir_read(0, buffer, 0);

        for(j = 0; j < FILENAME_MAX_LENGTH; j++){

            putc(buffer[j]);

        }

        printf("      file_size: %d \n", inode_ptr[den_ptr[i].inode].length);
     }
     return 1;
}



/* file_test
 *
 * Asserts that a given file data can be read and output to terminal
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: displays file content to the terminal
 * Coverage: file_read(), read_dentry_by_name(), read_data()
 * Files: filesys.c/.h, tests.c/.h
*/
int file_test(int32_t fname) {
    clear();

    uint8_t buffer[5500];


    int num_bytes, i;

    printf("FILE_NAME = %s \n", fname);


    num_bytes = file_read(fname, buffer, 5500);
        
    for(i = 0; i < num_bytes; i++){

        if(buffer[i] != '\0')
            putc(buffer[i]);

    }

return 1;

}
