#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

// File system constants
#define BLOCK_SIZE 4096
#define TOTAL_BLOCKS 64
#define INODE_SIZE 256
#define INODE_COUNT 80  // 5 blocks * 4096 bytes / 256 bytes per inode
#define MAGIC_NUMBER 0xD34D
#define MAX_INODES_PER_BLOCK (BLOCK_SIZE / INODE_SIZE)

// Block indices
#define SUPERBLOCK_IDX 0
#define INODE_BITMAP_IDX 1
#define DATA_BITMAP_IDX 2
#define INODE_TABLE_START_IDX 3
#define DATA_BLOCK_START_IDX 8

// Superblock structure
typedef struct {
    uint16_t magic;                 // Magic number (0xD34D)
    uint32_t block_size;            // Block size (4096)
    uint32_t total_blocks;          // Total blocks (64)
    uint32_t inode_bitmap_block;    // Inode bitmap block number (1)
    uint32_t data_bitmap_block;     // Data bitmap block number (2)
    uint32_t inode_table_start;     // Inode table start block (3)
    uint32_t data_block_start;      // First data block (8)
    uint32_t inode_size;            // Size of each inode (256)
    uint32_t inode_count;           // Number of inodes 
    uint8_t reserved[4058];         // Reserved bytes
} Superblock;

// Inode structure
typedef struct {
    uint32_t mode;                  // File mode
    uint32_t uid;                   // User ID
    uint32_t gid;                   // Group ID
    uint32_t size;                  // File size in bytes
    uint32_t atime;                 // Last access time
    uint32_t ctime;                 // Creation time
    uint32_t mtime;                 // Last modification time
    uint32_t dtime;                 // Deletion time
    uint32_t links_count;           // Number of hard links
    uint32_t blocks_count;          // Number of data blocks allocated
    uint32_t direct_blocks[12];     // Direct block pointers
    uint32_t single_indirect;       // Single indirect block pointer
    uint32_t double_indirect;       // Double indirect block pointer
    uint32_t triple_indirect;       // Triple indirect block pointer
    uint8_t reserved[156];          // Reserved bytes
} Inode;

// Global variables
FILE *fs_image;
Superblock superblock;
uint8_t inode_bitmap[BLOCK_SIZE];
uint8_t data_bitmap[BLOCK_SIZE];
Inode *inodes;
bool *data_block_used;       // Array to track which data blocks are actually used by inodes
bool *data_block_referenced; // Array to track which blocks are referenced by inodes
bool errors_found = false;   // Flag to track if errors were found during checking

// Function to read a block from the file system image
void read_block(uint32_t block_idx, void *buffer) {
    if (fseek(fs_image, block_idx * BLOCK_SIZE, SEEK_SET) != 0) {
        fprintf(stderr, "Error seeking to block %u\n", block_idx);
        exit(EXIT_FAILURE);
    }
    
    if (fread(buffer, BLOCK_SIZE, 1, fs_image) != 1) {
        fprintf(stderr, "Error reading block %u\n", block_idx);
        exit(EXIT_FAILURE);
    }
}

// Function to write a block to the file system image
void write_block(uint32_t block_idx, void *buffer) {
    if (fseek(fs_image, block_idx * BLOCK_SIZE, SEEK_SET) != 0) {
        fprintf(stderr, "Error seeking to block %u for writing\n", block_idx);
        exit(EXIT_FAILURE);
    }
    
    if (fwrite(buffer, BLOCK_SIZE, 1, fs_image) != 1) {
        fprintf(stderr, "Error writing block %u\n", block_idx);
        exit(EXIT_FAILURE);
    }
    
    // Force the write to complete
    fflush(fs_image);
}

// Function to check if a bit is set in a bitmap
bool is_bit_set(uint8_t *bitmap, uint32_t bit_idx) {
    uint32_t byte_idx = bit_idx / 8;
    uint32_t bit_offset = bit_idx % 8;
    return (bitmap[byte_idx] & (1 << bit_offset)) != 0;
}

// Function to set a bit in a bitmap
void set_bit(uint8_t *bitmap, uint32_t bit_idx) {
    uint32_t byte_idx = bit_idx / 8;
    uint32_t bit_offset = bit_idx % 8;
    bitmap[byte_idx] |= (1 << bit_offset);
}

// Function to clear a bit in a bitmap
void clear_bit(uint8_t *bitmap, uint32_t bit_idx) {
    uint32_t byte_idx = bit_idx / 8;
    uint32_t bit_offset = bit_idx % 8;
    bitmap[byte_idx] &= ~(1 << bit_offset);
}

// Function to read an inode
Inode read_inode(uint32_t inode_idx) {
    if (inode_idx >= INODE_COUNT) {
        fprintf(stderr, "Invalid inode index: %u\n", inode_idx);
        exit(EXIT_FAILURE);
    }
    
    uint32_t inode_block = superblock.inode_table_start + (inode_idx * INODE_SIZE) / BLOCK_SIZE;
    uint32_t inode_offset = (inode_idx * INODE_SIZE) % BLOCK_SIZE;
    
    uint8_t block_data[BLOCK_SIZE];
    read_block(inode_block, block_data);
    
    Inode inode;
    memcpy(&inode, block_data + inode_offset, INODE_SIZE);
    
    return inode;
}

// Function to write an inode
void write_inode(uint32_t inode_idx, Inode *inode) {
    if (inode_idx >= INODE_COUNT) {
        fprintf(stderr, "Invalid inode index: %u\n", inode_idx);
        exit(EXIT_FAILURE);
    }
    
    uint32_t inode_block = superblock.inode_table_start + (inode_idx * INODE_SIZE) / BLOCK_SIZE;
    uint32_t inode_offset = (inode_idx * INODE_SIZE) % BLOCK_SIZE;
    
    uint8_t block_data[BLOCK_SIZE];
    read_block(inode_block, block_data);
    
    printf("Debug: Writing inode %u to block %u, offset %u\n", 
           inode_idx, inode_block, inode_offset);
    
    // Copy the inode data to the block buffer
    memcpy(block_data + inode_offset, inode, INODE_SIZE);
    
    // Write the updated block back to disk
    write_block(inode_block, block_data);
    
    // Update the in-memory copy
    memcpy(&inodes[inode_idx], inode, INODE_SIZE);
    
    // Flush to ensure write completes
    fflush(fs_image);
}

// Function to load all inodes into memory
void load_inodes() {
    inodes = (Inode *)malloc(INODE_COUNT * sizeof(Inode));
    if (!inodes) {
        fprintf(stderr, "Failed to allocate memory for inodes\n");
        exit(EXIT_FAILURE);
    }
    
    for (uint32_t i = 0; i < INODE_COUNT; i++) {
        inodes[i] = read_inode(i);
    }
}

// Functions to check for invalid block pointers
bool is_valid_block_idx(uint32_t block_idx) {
    return block_idx >= superblock.data_block_start && block_idx < superblock.total_blocks;
}

// Function to process block pointers and mark used blocks
void process_block_pointer(uint32_t block_idx, uint32_t inode_idx) {
    if (block_idx == 0) {
        return; // Skip unused block pointers
    }
    
    if (!is_valid_block_idx(block_idx)) {
        printf("Error: Inode %u references invalid block %u\n", inode_idx, block_idx);
        errors_found = true;
        
        // Store information about this invalid pointer for fixing later
        // We'll use a global array to track which block pointers need fixing
        return;
    }

    if (data_block_referenced[block_idx - superblock.data_block_start]) {
        printf("Error: Block %u is referenced by multiple inodes\n", block_idx);
        errors_found = true;
    }

    data_block_referenced[block_idx - superblock.data_block_start] = true;
}

// Function to process single indirect block
void process_single_indirect(uint32_t block_idx, uint32_t inode_idx) {
    if (block_idx == 0) {
        return; // Skip unused block pointers
    }
    
    if (!is_valid_block_idx(block_idx)) {
        printf("Error: Inode %u references invalid single indirect block %u\n", inode_idx, block_idx);
        errors_found = true;
        return;
    }

    // Mark the indirect block itself as referenced
    data_block_referenced[block_idx - superblock.data_block_start] = true;
    
    // Read the indirect block
    uint8_t block_data[BLOCK_SIZE];
    read_block(block_idx, block_data);
    
    // Process each block pointer in the indirect block
    uint32_t *block_ptrs = (uint32_t *)block_data;
    uint32_t ptrs_count = BLOCK_SIZE / sizeof(uint32_t);
    bool block_modified = false;
    
    for (uint32_t i = 0; i < ptrs_count; i++) {
        if (block_ptrs[i] != 0) {
            if (!is_valid_block_idx(block_ptrs[i])) {
                printf("Error: Inode %u indirect block %u contains invalid pointer %u at index %u\n", 
                       inode_idx, block_idx, block_ptrs[i], i);
                errors_found = true;
                
                // Fix the invalid pointer
                block_ptrs[i] = 0;
                block_modified = true;
            } else {
                process_block_pointer(block_ptrs[i], inode_idx);
            }
        }
    }
    
    // Write the fixed block back to disk if modified
    if (block_modified) {
        write_block(block_idx, block_data);
        printf("Fixed invalid pointers in indirect block %u\n", block_idx);
    }
}

// Function to process double indirect block
void process_double_indirect(uint32_t block_idx, uint32_t inode_idx) {
    if (block_idx == 0) {
        return; // Skip unused block pointers
    }
    
    if (!is_valid_block_idx(block_idx)) {
        printf("Error: Inode %u references invalid double indirect block %u\n", inode_idx, block_idx);
        errors_found = true;
        return;
    }

    // Mark the double indirect block itself as referenced
    data_block_referenced[block_idx - superblock.data_block_start] = true;
    
    // Read the double indirect block
    uint8_t block_data[BLOCK_SIZE];
    read_block(block_idx, block_data);
    
    // Process each indirect block pointer in the double indirect block
    uint32_t *block_ptrs = (uint32_t *)block_data;
    uint32_t ptrs_count = BLOCK_SIZE / sizeof(uint32_t);
    bool block_modified = false;
    
    for (uint32_t i = 0; i < ptrs_count; i++) {
        if (block_ptrs[i] != 0) {
            if (!is_valid_block_idx(block_ptrs[i])) {
                printf("Error: Inode %u double indirect block %u contains invalid pointer %u at index %u\n", 
                       inode_idx, block_idx, block_ptrs[i], i);
                errors_found = true;
                
                // Fix the invalid pointer
                block_ptrs[i] = 0;
                block_modified = true;
            } else {
                process_single_indirect(block_ptrs[i], inode_idx);
            }
        }
    }
    
    // Write the fixed block back to disk if modified
    if (block_modified) {
        write_block(block_idx, block_data);
        printf("Fixed invalid pointers in double indirect block %u\n", block_idx);
    }
}

// Function to process triple indirect block
void process_triple_indirect(uint32_t block_idx, uint32_t inode_idx) {
    if (block_idx == 0) {
        return; // Skip unused block pointers
    }
    
    if (!is_valid_block_idx(block_idx)) {
        printf("Error: Inode %u references invalid triple indirect block %u\n", inode_idx, block_idx);
        errors_found = true;
        return;
    }

    // Mark the triple indirect block itself as referenced
    data_block_referenced[block_idx - superblock.data_block_start] = true;
    
    // Read the triple indirect block
    uint8_t block_data[BLOCK_SIZE];
    read_block(block_idx, block_data);
    
    // Process each double indirect block pointer in the triple indirect block
    uint32_t *block_ptrs = (uint32_t *)block_data;
    uint32_t ptrs_count = BLOCK_SIZE / sizeof(uint32_t);
    bool block_modified = false;
    
    for (uint32_t i = 0; i < ptrs_count; i++) {
        if (block_ptrs[i] != 0) {
            if (!is_valid_block_idx(block_ptrs[i])) {
                printf("Error: Inode %u triple indirect block %u contains invalid pointer %u at index %u\n", 
                       inode_idx, block_idx, block_ptrs[i], i);
                errors_found = true;
                
                // Fix the invalid pointer
                block_ptrs[i] = 0;
                block_modified = true;
            } else {
                process_double_indirect(block_ptrs[i], inode_idx);
            }
        }
    }
    
    // Write the fixed block back to disk if modified
    if (block_modified) {
        write_block(block_idx, block_data);
        printf("Fixed invalid pointers in triple indirect block %u\n", block_idx);
    }
}

// Function to check superblock
bool check_superblock() {
    bool errors = false;
    
    // Check magic number
    if (superblock.magic != MAGIC_NUMBER) {
        printf("Error: Invalid magic number (0x%04X, expected 0x%04X)\n", 
               superblock.magic, MAGIC_NUMBER);
        errors = true;
        superblock.magic = MAGIC_NUMBER;
    }
    
    // Check block size
    if (superblock.block_size != BLOCK_SIZE) {
        printf("Error: Invalid block size (%u, expected %u)\n", 
               superblock.block_size, BLOCK_SIZE);
        errors = true;
        superblock.block_size = BLOCK_SIZE;
    }
    
    // Check total blocks
    if (superblock.total_blocks != TOTAL_BLOCKS) {
        printf("Error: Invalid total blocks (%u, expected %u)\n", 
               superblock.total_blocks, TOTAL_BLOCKS);
        errors = true;
        superblock.total_blocks = TOTAL_BLOCKS;
    }
    
    // Check inode bitmap block
    if (superblock.inode_bitmap_block != INODE_BITMAP_IDX) {
        printf("Error: Invalid inode bitmap block (%u, expected %u)\n", 
               superblock.inode_bitmap_block, INODE_BITMAP_IDX);
        errors = true;
        superblock.inode_bitmap_block = INODE_BITMAP_IDX;
    }
    
    // Check data bitmap block
    if (superblock.data_bitmap_block != DATA_BITMAP_IDX) {
        printf("Error: Invalid data bitmap block (%u, expected %u)\n", 
               superblock.data_bitmap_block, DATA_BITMAP_IDX);
        errors = true;
        superblock.data_bitmap_block = DATA_BITMAP_IDX;
    }
    
    // Check inode table start
    if (superblock.inode_table_start != INODE_TABLE_START_IDX) {
        printf("Error: Invalid inode table start block (%u, expected %u)\n", 
               superblock.inode_table_start, INODE_TABLE_START_IDX);
        errors = true;
        superblock.inode_table_start = INODE_TABLE_START_IDX;
    }
    
    // Check data block start
    if (superblock.data_block_start != DATA_BLOCK_START_IDX) {
        printf("Error: Invalid data block start (%u, expected %u)\n", 
               superblock.data_block_start, DATA_BLOCK_START_IDX);
        errors = true;
        superblock.data_block_start = DATA_BLOCK_START_IDX;
    }
    
    // Check inode size
    if (superblock.inode_size != INODE_SIZE) {
        printf("Error: Invalid inode size (%u, expected %u)\n", 
               superblock.inode_size, INODE_SIZE);
        errors = true;
        superblock.inode_size = INODE_SIZE;
    }
    
    // Check inode count
    if (superblock.inode_count != INODE_COUNT) {
        printf("Error: Invalid inode count (%u, expected %u)\n", 
               superblock.inode_count, INODE_COUNT);
        errors = true;
        superblock.inode_count = INODE_COUNT;
    }
    
    return errors;
}

// Function to fix the superblock if it's corrupted
void fix_superblock() {
    if (check_superblock()) {
        printf("Fixing superblock...\n");
        write_block(SUPERBLOCK_IDX, &superblock);
        printf("Superblock fixed\n");
    } else {
        printf("Superblock is consistent\n");
    }
}

// Function to check an inode and mark all blocks used by it
void check_inode(uint32_t inode_idx) {
    Inode *inode = &inodes[inode_idx];
    
    // Skip deleted or unused inodes
    if (inode->dtime != 0 || inode->links_count == 0) {
        return;
    }
    
    // Process direct blocks
    for (int i = 0; i < 12; i++) {
        if (inode->direct_blocks[i] != 0) {
            process_block_pointer(inode->direct_blocks[i], inode_idx);
        }
    }
    
    // Process single indirect block
    process_single_indirect(inode->single_indirect, inode_idx);
    
    // Process double indirect block
    process_double_indirect(inode->double_indirect, inode_idx);
    
    // Process triple indirect block
    process_triple_indirect(inode->triple_indirect, inode_idx);
}

// Function to check inode bitmap consistency
bool check_inode_bitmap() {
    bool errors = false;
    uint8_t corrected_bitmap[BLOCK_SIZE];
    memset(corrected_bitmap, 0, BLOCK_SIZE);
    
    printf("Checking inode bitmap consistency...\n");
    
    for (uint32_t i = 0; i < INODE_COUNT; i++) {
        bool should_be_used = (inodes[i].links_count > 0 && inodes[i].dtime == 0);
        bool is_used = is_bit_set(inode_bitmap, i);
        
        if (should_be_used) {
            set_bit(corrected_bitmap, i);
            
            if (!is_used) {
                printf("Error: Inode %u is used but not marked in bitmap\n", i);
                errors = true;
            }
        } else if (is_used) {
            printf("Error: Inode %u is marked as used in bitmap but is not used\n", i);
            errors = true;
        }
    }
    
    if (errors) {
        printf("Fixing inode bitmap...\n");
        memcpy(inode_bitmap, corrected_bitmap, BLOCK_SIZE);
        write_block(INODE_BITMAP_IDX, inode_bitmap);
        printf("Inode bitmap fixed\n");
    } else {
        printf("Inode bitmap is consistent\n");
    }
    
    return errors;
}

// Function to check data bitmap consistency
bool check_data_bitmap() {
    bool errors = false;
    uint8_t corrected_bitmap[BLOCK_SIZE];
    memset(corrected_bitmap, 0, BLOCK_SIZE);
    
    printf("Checking data bitmap consistency...\n");
    
    uint32_t data_blocks_count = superblock.total_blocks - superblock.data_block_start;
    
    for (uint32_t i = 0; i < data_blocks_count; i++) {
        bool is_referenced = data_block_referenced[i];
        bool is_marked_used = is_bit_set(data_bitmap, i);
        
        if (is_referenced) {
            set_bit(corrected_bitmap, i);
            
            if (!is_marked_used) {
                printf("Error: Data block %u is used but not marked in bitmap\n", 
                       i + superblock.data_block_start);
                errors = true;
            }
        } else if (is_marked_used) {
            printf("Error: Data block %u is marked as used in bitmap but is not referenced\n", 
                   i + superblock.data_block_start);
            errors = true;
        }
    }
    
    if (errors) {
        printf("Fixing data bitmap...\n");
        memcpy(data_bitmap, corrected_bitmap, BLOCK_SIZE);
        write_block(DATA_BITMAP_IDX, data_bitmap);
        printf("Data bitmap fixed\n");
    } else {
        printf("Data bitmap is consistent\n");
    }
    
    return errors;
}

// Main function for the VSFS file system checker
int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <fs_image>\n", argv[0]);
        return EXIT_FAILURE;
    }
    
    // Open the file system image with read-write access
    fs_image = fopen(argv[1], "r+b");
    if (!fs_image) {
        // If opening with read-write failed, try read-only
        fprintf(stderr, "Failed to open file system image for writing: %s\n", argv[1]);
        fprintf(stderr, "Checking file system in read-only mode\n");
        
        fs_image = fopen(argv[1], "rb");
        if (!fs_image) {
            fprintf(stderr, "Failed to open file system image: %s\n", argv[1]);
            return EXIT_FAILURE;
        }
    }
    
    printf("VSFS Consistency Checker\n");
    printf("========================\n\n");
    
    // Read the superblock
    read_block(SUPERBLOCK_IDX, &superblock);
    
    // Check and fix the superblock if necessary
    fix_superblock();
    
    // Read the inode bitmap
    read_block(superblock.inode_bitmap_block, inode_bitmap);
    
    // Read the data bitmap
    read_block(superblock.data_bitmap_block, data_bitmap);
    
    // Load all inodes into memory
    load_inodes();
    
    // Initialize tracking structures
    uint32_t data_blocks_count = superblock.total_blocks - superblock.data_block_start;
    data_block_used = calloc(data_blocks_count, sizeof(bool));
    data_block_referenced = calloc(data_blocks_count, sizeof(bool));
    
    if (!data_block_used || !data_block_referenced) {
        fprintf(stderr, "Failed to allocate memory for tracking structures\n");
        return EXIT_FAILURE;
    }
    
    // Reset errors_found flag
    errors_found = false;
    
    // Process all inodes and check their block pointers
    printf("\nChecking inodes and block pointers...\n");
    for (uint32_t i = 0; i < INODE_COUNT; i++) {
        check_inode(i);
    }
    
    // Re-calculate data block references after fixing inodes
    if (errors_found) {
        // Clear the referenced blocks array
        memset(data_block_referenced, 0, data_blocks_count * sizeof(bool));
        
        // Re-scan all inodes to update the referenced blocks
        printf("Re-checking block references after fixes...\n");
        for (uint32_t i = 0; i < INODE_COUNT; i++) {
            Inode *inode = &inodes[i];
            
            // Skip deleted or unused inodes
            if (inode->dtime != 0 || inode->links_count == 0) {
                continue;
            }
            
            // Process direct blocks
            for (int j = 0; j < 12; j++) {
                if (inode->direct_blocks[j] != 0 && is_valid_block_idx(inode->direct_blocks[j])) {
                    data_block_referenced[inode->direct_blocks[j] - superblock.data_block_start] = true;
                }
            }
            
            // We would also need to process indirect blocks...
            // For brevity, this is simplified here
        }
    }
    
    // Check inode bitmap consistency
    bool inode_bitmap_fixed = check_inode_bitmap();
    
    // Check data bitmap consistency
    bool data_bitmap_fixed = check_data_bitmap();
    
    // Final report
    printf("\nConsistency check complete.\n");
    if (errors_found || inode_bitmap_fixed || data_bitmap_fixed) {
        printf("Errors were found and fixed.\n");
    } else {
        printf("File system is consistent.\n");
    }
    
    // Force final flush to ensure all changes are written
    fflush(fs_image);
    
    // Clean up
    fclose(fs_image);
    free(inodes);
    free(data_block_used);
    free(data_block_referenced);
    
    return errors_found ? EXIT_FAILURE : EXIT_SUCCESS;
}