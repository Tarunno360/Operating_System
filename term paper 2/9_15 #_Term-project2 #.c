#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#define VSFS_MAGIC 0xD34D
#define BLOCK_SIZE 4096
#define TOTAL_BLOCKS 64
#define INODE_SIZE 256
#define MAX_INODES (5 * BLOCK_SIZE / INODE_SIZE) 
#define SUPERBLOCK_IDX 0
#define INODE_BITMAP_IDX 1
#define DATA_BITMAP_IDX 2
#define INODE_TABLE_START_IDX 3
#define DATA_BLOCK_START_IDX 8

typedef struct {
    uint16_t magic_value;          
    uint32_t block_dimension;       
    uint32_t fs_blocks_count;       
    uint32_t inode_bitmap_location; 
    uint32_t data_bitmap_location;  
    uint32_t inode_table_location;  
    uint32_t initial_data_block;    
    uint32_t inode_dimension;      
    uint32_t total_inodes;         
    uint8_t padding[4058];         
} __attribute__((packed)) vsfs_super_block;

typedef struct {
    uint32_t permissions;  
    uint32_t owner_id;         
    uint32_t group_id;        
    uint32_t file_size;       
    uint32_t access_time;       
    uint32_t creation_time;     
    uint32_t modification_time; 
    uint32_t deletion_time;      
    uint32_t hard_links;           
    uint32_t allocated_blocks;    
    uint32_t direct_block;        
    uint32_t single_indirect_ref; 
    uint32_t double_indirect_ref;  
    uint32_t triple_indirect_ref;  
    uint8_t unused[156];           
} __attribute__((packed)) vsfs_inode;

char *disk_image = NULL;
vsfs_super_block *sb_info = NULL;
uint8_t *inode_bmap = NULL;
uint8_t *data_bmap = NULL;
vsfs_inode *inode_table = NULL;
uint8_t *calculated_inode_bmap = NULL;
uint8_t *calculated_data_bmap = NULL;
int *block_references = NULL;       
bool *processed_blocks = NULL;   

bool mount_fs(const char *filename);
void verify_superblock();
void scan_inode_bitmap();
void scan_data_bitmap();
void detect_duplicate_blocks();
void detect_bad_blocks();
void repair_errors();
void save_fs_image(const char *filename);
void release_resources();

bool check_bit(uint8_t *bitmap, int bit);
void mark_bit(uint8_t *bitmap, int bit);
void unmark_bit(uint8_t *bitmap, int bit);
bool valid_inode(int inode_num);
void verify_block_references(uint32_t block_ref, int inode_num);
void process_indirect_blocks(uint32_t block_ref, int level, int inode_num);

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <input_image> <output_image>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *input_file = argv[1];
    const char *output_file = argv[2];
    
    printf("Consistency Checker for Very Simple File System\n");
    printf("========================\n\n");
    
    if (!mount_fs(input_file)) {
        fprintf(stderr, "Failed to load file system image: %s\n", input_file);
        return EXIT_FAILURE;
    }
    
    printf("Checking file system integrity...\n\n");
    
    verify_superblock();
    scan_inode_bitmap();
    scan_data_bitmap();
    detect_duplicate_blocks();
    detect_bad_blocks();

    repair_errors();
    
    save_fs_image(output_file);
    
    printf("\nConsistency check complete. Corrected image saved to: %s\n", output_file);

    release_resources();
    
    return EXIT_SUCCESS;
}

bool mount_fs(const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("Error opening file");
        return false;
    }
    
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    if (file_size != BLOCK_SIZE * TOTAL_BLOCKS) {
        fprintf(stderr, "Invalid file size: expected %d bytes, got %ld bytes\n", 
                BLOCK_SIZE * TOTAL_BLOCKS, file_size);
        fclose(file);
        return false;
    }

    disk_image = (char*)malloc(file_size);
    if (!disk_image) {
        perror("Memory allocation failed");
        fclose(file);
        return false;
    }
    
    if (fread(disk_image, 1, file_size, file) != file_size) {
        perror("Error reading file");
        free(disk_image);
        disk_image = NULL;
        fclose(file);
        return false;
    }
    
    fclose(file);
    
    sb_info = (vsfs_super_block*)(disk_image + SUPERBLOCK_IDX * BLOCK_SIZE);
    inode_bmap = (uint8_t*)(disk_image + INODE_BITMAP_IDX * BLOCK_SIZE);
    data_bmap = (uint8_t*)(disk_image + DATA_BITMAP_IDX * BLOCK_SIZE);
    inode_table = (vsfs_inode*)(disk_image + INODE_TABLE_START_IDX * BLOCK_SIZE);
    
    calculated_inode_bmap = (uint8_t*)calloc(BLOCK_SIZE, 1);
    calculated_data_bmap = (uint8_t*)calloc(BLOCK_SIZE, 1);
    block_references = (int*)calloc(TOTAL_BLOCKS, sizeof(int));
    processed_blocks = (bool*)calloc(TOTAL_BLOCKS, sizeof(bool));
    
    if (!calculated_inode_bmap || !calculated_data_bmap || !block_references || !processed_blocks) {
        perror("Memory allocation failed");
        release_resources();
        return false;
    }
    
    return true;
}

void verify_superblock() {
    printf("Loading....");
    printf("Checking Superblock for any error\n");
    bool has_errors = false;
    
    if (sb_info->magic_value != VSFS_MAGIC) {
        printf("ERROR DETECTED: Invalid magic number: 0x%04X (expected 0x%04X)\n", 
               sb_info->magic_value, VSFS_MAGIC);
        sb_info->magic_value = VSFS_MAGIC;
        has_errors = true;
    }
    
    if (sb_info->block_dimension != BLOCK_SIZE) {
        printf("ERROR DETECTED: Invalid block size: %u (expected %u)\n", 
               sb_info->block_dimension, BLOCK_SIZE);
        sb_info->block_dimension = BLOCK_SIZE;
        has_errors = true;
    }
    
    if (sb_info->fs_blocks_count != TOTAL_BLOCKS) {
        printf("ERROR DETECTED: Invalid total blocks: %u (expected %u)\n", 
               sb_info->fs_blocks_count, TOTAL_BLOCKS);
        sb_info->fs_blocks_count = TOTAL_BLOCKS;
        has_errors = true;
    }
    
    if (sb_info->inode_bitmap_location != INODE_BITMAP_IDX) {
        printf("ERROR DETECTED: Invalid inode bitmap block number: %u (expected %u)\n", 
               sb_info->inode_bitmap_location, INODE_BITMAP_IDX);
        sb_info->inode_bitmap_location = INODE_BITMAP_IDX;
        has_errors = true;
    }
    
    if (sb_info->data_bitmap_location != DATA_BITMAP_IDX) {
        printf("ERROR DETECETED: Invalid data bitmap block number: %u (expected %u)\n", 
               sb_info->data_bitmap_location, DATA_BITMAP_IDX);
        sb_info->data_bitmap_location = DATA_BITMAP_IDX;
        has_errors = true;
    }
    
    if (sb_info->inode_table_location != INODE_TABLE_START_IDX) {
        printf("ERROR DETECTED: Invalid inode table start block number: %u (expected %u)\n", 
               sb_info->inode_table_location, INODE_TABLE_START_IDX);
        sb_info->inode_table_location = INODE_TABLE_START_IDX;
        has_errors = true;
    }
    
    if (sb_info->initial_data_block != DATA_BLOCK_START_IDX) {
        printf("ERROR DETECTED: Invalid first data block number: %u (expected %u)\n", 
               sb_info->initial_data_block, DATA_BLOCK_START_IDX);
        sb_info->initial_data_block = DATA_BLOCK_START_IDX;
        has_errors = true;
    }
    
    if (sb_info->inode_dimension != INODE_SIZE) {
        printf("ERROR DETECTED: Invalid inode size: %u (expected %u)\n", 
               sb_info->inode_dimension, INODE_SIZE);
        sb_info->inode_dimension = INODE_SIZE;
        has_errors = true;
    }
    
    if (sb_info->total_inodes != MAX_INODES) {
        printf("ERROR DETECTED: Invalid inode count: %u (expected %u)\n", 
               sb_info->total_inodes, MAX_INODES);
        sb_info->total_inodes = MAX_INODES;
        has_errors = true;
    }
    
    if (!has_errors) {
        printf("Congrats.No error has been occured in superblock. So super block is valid.\n");
    }
    printf("\n");
}

bool check_bit(uint8_t *bitmap, int bit) {
    int byte_idx = bit / 8;
    int bit_idx = bit % 8;
    return (bitmap[byte_idx] & (1 << bit_idx)) != 0;
}

void mark_bit(uint8_t *bitmap, int bit) {
    int byte_idx = bit / 8;
    int bit_idx = bit % 8;
    bitmap[byte_idx] |= (1 << bit_idx);
}

void unmark_bit(uint8_t *bitmap, int bit) {
    int byte_idx = bit / 8;
    int bit_idx = bit % 8;
    bitmap[byte_idx] &= ~(1 << bit_idx);
}

bool valid_inode(int inode_num) {
    vsfs_inode *inode = &inode_table[inode_num];
    return (inode->hard_links > 0 && inode->deletion_time == 0);
}

void scan_inode_bitmap() {
    printf("Checking Inode Bitmap...\n");
    int errors = 0;
    
    for (uint32_t i = 0; i < sb_info->total_inodes; i++) {
        if (valid_inode(i)) {
            mark_bit(calculated_inode_bmap, i);
        }
    }
    
    for (uint32_t i = 0; i < sb_info->total_inodes; i++) {
        bool should_be_used = check_bit(calculated_inode_bmap, i);
        bool is_used = check_bit(inode_bmap, i);
        
        if (should_be_used != is_used) {
            printf("ERROR: Inode %u bitmap inconsistency: %s but should be %s\n", 
                   i, is_used ? "used" : "unused", should_be_used ? "used" : "unused");
            errors++;
        }
    }
    
    if (errors == 0) {
        printf("Inode bitmap is consistent.\n");
    } else {
        printf("Found %d inconsistencies in the inode bitmap.\n", errors);
    }
    printf("\n");
}

void verify_block_references(uint32_t block_ref, int inode_num) {
    if (block_ref == 0) {
        return;  
    }
    
    if (block_ref >= sb_info->fs_blocks_count) {
        printf("ERROR: Inode %d references invalid block %u (out of range)\n", 
               inode_num, block_ref);
        return;
    }
    
    mark_bit(calculated_data_bmap, block_ref - DATA_BLOCK_START_IDX);
    block_references[block_ref]++;
}

void process_indirect_blocks(uint32_t block_ref, int level, int inode_num) {
    if (block_ref == 0) {
        return; 
    }
    
    if (processed_blocks[block_ref]) {
        printf("ERROR DETECTED: Cycle detected in indirect blocks! Block %u is referenced multiple times\n", block_ref);
        return;
    }
    
    if (block_ref >= sb_info->fs_blocks_count) {
        printf("ERROR DETECTED: Inode %d references invalid indirect block %u (out of range)\n", 
               inode_num, block_ref);
        return;
    }
    processed_blocks[block_ref] = true;
    mark_bit(calculated_data_bmap, block_ref - DATA_BLOCK_START_IDX);
    block_references[block_ref]++;

    uint32_t *indirect_entries = (uint32_t*)(disk_image + block_ref * BLOCK_SIZE);
    uint32_t entries_per_block = BLOCK_SIZE / sizeof(uint32_t);
    
    for (uint32_t i = 0; i < entries_per_block; i++) {
        uint32_t entry = indirect_entries[i];
        if (entry == 0) continue;
        
        if (level == 1) {  
            verify_block_references(entry, inode_num);
        } else if (level > 1) {  
            process_indirect_blocks(entry, level - 1, inode_num);
        }
    }
    
    processed_blocks[block_ref] = false;  
}

void scan_data_bitmap() {
    printf("Checking Data Bitmap...\n");
    int errors = 0;
    
    memset(calculated_data_bmap, 0, BLOCK_SIZE);
    memset(block_references, 0, TOTAL_BLOCKS * sizeof(int));
    memset(processed_blocks, 0, TOTAL_BLOCKS * sizeof(bool));

    for (uint32_t i = 0; i < DATA_BLOCK_START_IDX; i++) {
        block_references[i] = 1;  
    }
   
    for (uint32_t i = 0; i < sb_info->total_inodes; i++) {
        if (!valid_inode(i)) continue;
        
        vsfs_inode *inode = &inode_table[i];
        
        verify_block_references(inode->direct_block, i);

        if (inode->single_indirect_ref != 0) {
            process_indirect_blocks(inode->single_indirect_ref, 1, i);
        }
        
        if (inode->double_indirect_ref != 0) {
            process_indirect_blocks(inode->double_indirect_ref, 2, i);
        }

        if (inode->triple_indirect_ref != 0) {
            process_indirect_blocks(inode->triple_indirect_ref, 3, i);
        }
    }

    for (uint32_t i = 0; i < (sb_info->fs_blocks_count - DATA_BLOCK_START_IDX); i++) {
        bool should_be_used = check_bit(calculated_data_bmap, i);
        bool is_used = check_bit(data_bmap, i);
        
        if (should_be_used != is_used) {
            printf("ERROR: Data block %u bitmap inconsistency: %s but should be %s\n", 
                   i + DATA_BLOCK_START_IDX, is_used ? "used" : "unused", should_be_used ? "used" : "unused");
            errors++;
        }
    }
    
    if (errors == 0) {
        printf("Data bitmap is consistent.\n");
    } else {
        printf("Found %d inconsistencies in the data bitmap.\n", errors);
    }
    printf("\n");
}

void detect_duplicate_blocks() {
    printf("Checking for duplicate block references...\n");
    int duplicates = 0;
    
    for (uint32_t i = DATA_BLOCK_START_IDX; i < sb_info->fs_blocks_count; i++) {
        if (block_references[i] > 1) {
            printf("ERROR: Block %u is referenced by %d inodes\n", i, block_references[i]);
            duplicates++;
        }
    }
    
    if (duplicates == 0) {
        printf("No duplicate block references found.\n");
    } else {
        printf("Found %d blocks with multiple references.\n", duplicates);
    }
    printf("\n");
}

void detect_bad_blocks() {
    printf("Checking for bad block references...\n");
    int bad_blocks = 0;
    
    for (uint32_t i = 0; i < sb_info->total_inodes; i++) {
        if (!valid_inode(i)) continue;
        
        vsfs_inode *inode = &inode_table[i];
        
        if (inode->direct_block != 0 && inode->direct_block >= sb_info->fs_blocks_count) {
            printf("ERROR: Inode %u has invalid direct block pointer: %u\n", i, inode->direct_block);
            bad_blocks++;
            
            inode->direct_block = 0;
        }
        
        if (inode->single_indirect_ref != 0 && inode->single_indirect_ref >= sb_info->fs_blocks_count) {
            printf("ERROR: Inode %u has invalid single indirect block pointer: %u\n", i, inode->single_indirect_ref);
            bad_blocks++;
            inode->single_indirect_ref = 0;
        }
        
        if (inode->double_indirect_ref != 0 && inode->double_indirect_ref >= sb_info->fs_blocks_count) {
            printf("ERROR: Inode %u has invalid double indirect block pointer: %u\n", i, inode->double_indirect_ref);
            bad_blocks++;
            inode->double_indirect_ref = 0;
        }
        
        if (inode->triple_indirect_ref != 0 && inode->triple_indirect_ref >= sb_info->fs_blocks_count) {
            printf("ERROR: Inode %u has invalid triple indirect block pointer: %u\n", i, inode->triple_indirect_ref);
            bad_blocks++;
            inode->triple_indirect_ref = 0;
        }
    }
    
    if (bad_blocks == 0) {
        printf("No bad block references found.\n");
    } else {
        printf("Fixed %d bad block references.\n", bad_blocks);
    }
    printf("\n");
}

void repair_errors() {
    printf("Fixing file system errors(if any)...\n");

    printf("Fixing inode bitmap(if any)...\n");
    for (uint32_t i = 0; i < sb_info->total_inodes; i++) {
        bool should_be_used = valid_inode(i);
        
        if (should_be_used) {
            mark_bit(inode_bmap, i);
        } else {
            unmark_bit(inode_bmap, i);
        }
    }

    printf("Fixing data bitmap(if any)...\n");
    for (uint32_t i = 0; i < (sb_info->fs_blocks_count - DATA_BLOCK_START_IDX); i++) {
        bool should_be_used = check_bit(calculated_data_bmap, i);
        
        if (should_be_used) {
            mark_bit(data_bmap, i);
        } else {
            unmark_bit(data_bmap, i);
        }
    }
    
    printf("All errors fixed(if any).\n\n");
}

void save_fs_image(const char *filename) {
    FILE *file = fopen(filename, "wb");
    if (!file) {
        perror("Error opening output file");
        return;
    }
    
    size_t fs_size = BLOCK_SIZE * TOTAL_BLOCKS;
    if (fwrite(disk_image, 1, fs_size, file) != fs_size) {
        perror("Error writing file system image");
    }
    
    fclose(file);
}

void release_resources() {
    if (disk_image) free(disk_image);
    if (calculated_inode_bmap) free(calculated_inode_bmap);
    if (calculated_data_bmap) free(calculated_data_bmap);
    if (block_references) free(block_references);
    if (processed_blocks) free(processed_blocks);
    
    disk_image = NULL;
    calculated_inode_bmap = NULL;
    calculated_data_bmap = NULL;
    block_references = NULL;
    processed_blocks = NULL;
}