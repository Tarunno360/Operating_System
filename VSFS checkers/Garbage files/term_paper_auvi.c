

1 of 2,113
(no subject)
External
Inbox

Ferdous Auvi
Attachments
8:55 PM (2 hours ago)
to me


 One attachment
  •  Scanned by Gmail
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

// Constants and macros
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
    uint16_t magic_block;
    uint32_t block_size;
    uint32_t total_no_blocks;
    uint32_t inode_bitmap_block;
    uint32_t data_bitmap_block;
    uint32_t inode_table_start;
    uint32_t first_data_block;
    uint32_t inode_size;
    uint32_t inode_count;
    uint8_t reserved[4058];
} __attribute__((packed)) vsfs_super_block;

typedef struct {
    uint32_t mode;
    uint32_t uid;
    uint32_t gid;
    uint32_t size;
    uint32_t atime;
    uint32_t ctime;
    uint32_t mtime;
    uint32_t dtime;
    uint32_t nlink;
    uint32_t blocks;
    uint32_t direct_ptr;
    uint32_t single_indirect;
    uint32_t double_indirect;
    uint32_t triple_indirect;
    uint8_t reserved[156];
} __attribute__((packed)) vsfs_inode;

// Global variables
char *fs_image = NULL;
vsfs_super_block *superblock_checker = NULL;
uint8_t *inode_bitmap_checker = NULL;
uint8_t *data_bitmap_checker = NULL;
vsfs_inode *inodes = NULL;
uint8_t *computed_inode_bitmap = NULL;
uint8_t *computed_data_bitmap = NULL;
int *block_refs = NULL;
bool *visited_blocks = NULL;

// Function declarations
bool load_fs_image(const char *filename);
void superblock_checker();
void inode_bitmap_checker();
void data_bitmap_checker();
void duplicate_blocks_checker();
void bad_blocks_checker();
void error_fixing();
void write_fs_image(const char *filename);
void cleanup();

bool is_bit_set(uint8_t *bitmap, int bit);
void set_bit(uint8_t *bitmap, int bit);
void clear_bit(uint8_t *bitmap, int bit);
bool is_valid_inode(int inode_idx);
void block_pointers_validation(uint32_t block_ptr, int inode_idx);
void indirect_blockhandling(uint32_t block_ptr, int level, int inode_idx);

// Main function
int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <input_image> <output_image>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *input_file = argv[1];
    const char *output_file = argv[2];

    printf("VSFS Consistency Checker\n");
    printf("========================\n\n");

    if (!load_fs_image(input_file)) {
        fprintf(stderr, "Failed to load file system image: %s\n", input_file);
        return EXIT_FAILURE;
    }

    printf("Checking file system integrity...\n\n");

    superblock_checker();
    inode_bitmap_checker();
    data_bitmap_checker();
    duplicate_blocks_checker();
    bad_blocks_checker();

    error_fixing();
    write_fs_image(output_file);

    printf("\nConsistency check complete. Corrected image saved to: %s\n", output_file);
    cleanup();

    return EXIT_SUCCESS;
}

// Load file system image
bool load_fs_image(const char *filename) {
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

    fs_image = (char *)malloc(file_size);
    if (!fs_image) {
        perror("Memory allocation failed");
        fclose(file);
        return false;
    }

    if (fread(fs_image, 1, file_size, file) != file_size) {
        perror("Error reading file");
        free(fs_image);
        fs_image = NULL;
        fclose(file);
        return false;
    }

    fclose(file);

    superblock_checker = (vsfs_super_block *)(fs_image + SUPERBLOCK_IDX * BLOCK_SIZE);
    inode_bitmap_checker = (uint8_t *)(fs_image + INODE_BITMAP_IDX * BLOCK_SIZE);
    data_bitmap_checker = (uint8_t *)(fs_image + DATA_BITMAP_IDX * BLOCK_SIZE);
    inodes = (vsfs_inode *)(fs_image + INODE_TABLE_START_IDX * BLOCK_SIZE);

    computed_inode_bitmap = (uint8_t *)calloc(BLOCK_SIZE, 1);
    computed_data_bitmap = (uint8_t *)calloc(BLOCK_SIZE, 1);
    block_refs = (int *)calloc(TOTAL_BLOCKS, sizeof(int));
    visited_blocks = (bool *)calloc(TOTAL_BLOCKS, sizeof(bool));

    if (!computed_inode_bitmap || !computed_data_bitmap || !block_refs || !visited_blocks) {
        perror("Memory allocation failed");
        cleanup();
        return false;
    }

    return true;
}

// Superblock checker
void superblock_checker() {
    printf("Checking Superblock...\n");
    bool has_errors = false;

    if (superblock_checker->magic_block != VSFS_MAGIC) {
        printf("ERROR: Invalid magic number: 0x%04X (expected 0x%04X)\n",
               superblock_checker->magic_block, VSFS_MAGIC);
        superblock_checker->magic_block = VSFS_MAGIC;
        has_errors = true;
    }

    if (superblock_checker->block_size != BLOCK_SIZE) {
        printf("ERROR: Invalid block size: %u (expected %u)\n",
               superblock_checker->block_size, BLOCK_SIZE);
        superblock_checker->block_size = BLOCK_SIZE;
        has_errors = true;
    }

    if (superblock_checker->total_no_blocks != TOTAL_BLOCKS) {
        printf("ERROR: Invalid total blocks: %u (expected %u)\n",
               superblock_checker->total_no_blocks, TOTAL_BLOCKS);
        superblock_checker->total_no_blocks = TOTAL_BLOCKS;
        has_errors = true;
    }

    if (!has_errors) {
        printf("Superblock is valid.\n");
    }
    printf("\n");
}

// Helper functions
bool is_bit_set(uint8_t *bitmap, int bit) {
    return (bitmap[bit / 8] & (1 << (bit % 8))) != 0;
}

void set_bit(uint8_t *bitmap, int bit) {
    bitmap[bit / 8] |= (1 << (bit % 8));
}

void clear_bit(uint8_t *bitmap, int bit) {
    bitmap[bit / 8] &= ~(1 << (bit % 8));
}

bool is_valid_inode(int inode_idx) {
    vsfs_inode *inode = &inodes[inode_idx];
    return (inode->nlink > 0 && inode->dtime == 0);
}

void inode_bitmap_checker() {
    printf("Checking Inode Bitmap...\n");
    int errors = 0;

    for (uint32_t i = 0; i < superblock_checker->inode_count; i++) {
        if (is_valid_inode(i)) {
            set_bit(computed_inode_bitmap, i);
        }
    }

    for (uint32_t i = 0; i < superblock_checker->inode_count; i++) {
        bool should_be_used = is_bit_set(computed_inode_bitmap, i);
        bool is_used = is_bit_set(inode_bitmap_checker, i);

        if (should_be_used != is_used) {
            printf("ERROR: Inode %u bitmap inconsistency\n", i);
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
void block_pointers_validation(uint32_t block_ptr, int inode_idx) {
    if (block_ptr == 0) {
        return; // Null pointer, nothing to do
    }

    // Check if the block pointer is valid (within range)
    if (block_ptr >= superblock_checker->total_no_blocks) {
        printf("ERROR: Inode %d references invalid block %u (out of range)\n",
               inode_idx, block_ptr);
        return;
    }

    // Mark the block as referenced
    set_bit(computed_data_bitmap, block_ptr - DATA_BLOCK_START_IDX);
    block_refs[block_ptr]++;
}

void indirect_blockhandling(uint32_t block_ptr, int level, int inode_idx) {
    if (block_ptr == 0) {
        return; // Null pointer
    }

    if (visited_blocks[block_ptr]) {
        printf("ERROR: Cycle detected in indirect blocks! Block %u is referenced multiple times\n", block_ptr);
        return;
    }

    if (block_ptr >= superblock_checker->total_no_blocks) {
        printf("ERROR: Inode %d references invalid indirect block %u (out of range)\n",
               inode_idx, block_ptr);
        return;
    }

    visited_blocks[block_ptr] = true;
    set_bit(computed_data_bitmap, block_ptr - DATA_BLOCK_START_IDX);
    block_refs[block_ptr]++;

    uint32_t *indirect_entries = (uint32_t *)(fs_image + block_ptr * BLOCK_SIZE);
    uint32_t entries_per_block = BLOCK_SIZE / sizeof(uint32_t);

    for (uint32_t i = 0; i < entries_per_block; i++) {
        uint32_t entry = indirect_entries[i];
        if (entry == 0) continue;

        if (level == 1) { // Single indirect
            block_pointers_validation(entry, inode_idx);
        } else if (level > 1) { // Double or triple indirect
            indirect_blockhandling(entry, level - 1, inode_idx);
        }
    }

    visited_blocks[block_ptr] = false; // Unmark for future traversals
}

void data_bitmap_checker() {
    printf("Checking Data Bitmap...\n");
    int errors = 0;

    memset(computed_data_bitmap, 0, BLOCK_SIZE);
    memset(block_refs, 0, TOTAL_BLOCKS * sizeof(int));
    memset(visited_blocks, 0, TOTAL_BLOCKS * sizeof(bool));

    for (uint32_t i = 0; i < DATA_BLOCK_START_IDX; i++) {
        block_refs[i] = 1;
    }

    for (uint32_t i = 0; i < superblock_checker->inode_count; i++) {
        if (!is_valid_inode(i)) continue;

        vsfs_inode *inode = &inodes[i];

        block_pointers_validation(inode->direct_ptr, i);

        if (inode->single_indirect != 0) {
            indirect_blockhandling(inode->single_indirect, 1, i);
        }

        if (inode->double_indirect != 0) {
            indirect_blockhandling(inode->double_indirect, 2, i);
        }

        if (inode->triple_indirect != 0) {
            indirect_blockhandling(inode->triple_indirect, 3, i);
        }
    }

    for (uint32_t i = 0; i < (superblock_checker->total_no_blocks - DATA_BLOCK_START_IDX); i++) {
        bool should_be_used = is_bit_set(computed_data_bitmap, i);
        bool is_used = is_bit_set(data_bitmap_checker, i);

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

void duplicate_blocks_checker() {
    printf("Checking for duplicate block references...\n");
    int duplicates = 0;

    for (uint32_t i = DATA_BLOCK_START_IDX; i < superblock_checker->total_no_blocks; i++) {
        if (block_refs[i] > 1) {
            printf("ERROR: Block %u is referenced by %d inodes\n", i, block_refs[i]);
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

void bad_blocks_checker() {
    printf("Checking for bad block references...\n");
    int bad_blocks = 0;

    for (uint32_t i = 0; i < superblock_checker->inode_count; i++) {
        if (!is_valid_inode(i)) continue;

        vsfs_inode *inode = &inodes[i];

        if (inode->direct_ptr != 0 && inode->direct_ptr >= superblock_checker->total_no_blocks) {
            printf("ERROR: Inode %u has invalid direct block pointer: %u\n", i, inode->direct_ptr);
            bad_blocks++;
            inode->direct_ptr = 0; // Fix the error by setting to null
        }

        if (inode->single_indirect != 0 && inode->single_indirect >= superblock_checker->total_no_blocks) {
            printf("ERROR: Inode %u has invalid single indirect block pointer: %u\n", i, inode->single_indirect);
            bad_blocks++;
            inode->single_indirect = 0;
        }

        if (inode->double_indirect != 0 && inode->double_indirect >= superblock_checker->total_no_blocks) {
            printf("ERROR: Inode %u has invalid double indirect block pointer: %u\n", i, inode->double_indirect);
            bad_blocks++;
            inode->double_indirect = 0;
        }

        if (inode->triple_indirect != 0 && inode->triple_indirect >= superblock_checker->total_no_blocks) {
            printf("ERROR: Inode %u has invalid triple indirect block pointer: %u\n", i, inode->triple_indirect);
            bad_blocks++;
            inode->triple_indirect = 0;
        }
    }

    if (bad_blocks == 0) {
        printf("No bad block references found.\n");
    } else {
        printf("Fixed %d bad block references.\n", bad_blocks);
    }
    printf("\n");
}

void error_fixing() {
    printf("Fixing file system errors...\n");

    printf("Fixing inode bitmap...\n");
    for (uint32_t i = 0; i < superblock_checker->inode_count; i++) {
        bool should_be_used = is_valid_inode(i);

        if (should_be_used) {
            set_bit(inode_bitmap_checker, i);
        } else {
            clear_bit(inode_bitmap_checker, i);
        }
    }

    printf("Fixing data bitmap...\n");
    for (uint32_t i = 0; i < (superblock_checker->total_no_blocks - DATA_BLOCK_START_IDX); i++) {
        bool should_be_used = is_bit_set(computed_data_bitmap, i);

        if (should_be_used) {
            set_bit(data_bitmap_checker, i);
        } else {
            clear_bit(data_bitmap_checker, i);
        }
    }

    printf("All errors fixed.\n\n");
}

void write_fs_image(const char *filename) {
    FILE *file = fopen(filename, "wb");
    if (!file) {
        perror("Error opening output file");
        return;
    }

    size_t fs_size = BLOCK_SIZE * TOTAL_BLOCKS;
    if (fwrite(fs_image, 1, fs_size, file) != fs_size) {
        perror("Error writing file system image");
    }

    fclose(file);
}

void cleanup() {
    if (fs_image) free(fs_image);
    if (computed_inode_bitmap) free(computed_inode_bitmap);
    if (computed_data_bitmap) free(computed_data_bitmap);
    if (block_refs) free(block_refs);
    if (visited_blocks) free(visited_blocks);

    fs_image = NULL;
    computed_inode_bitmap = NULL;
    computed_data_bitmap = NULL;
    block_refs = NULL;
    visited_blocks = NULL;
}