#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>


#define DATA_BLOCK_COUNT (TOTAL_BLOCKS - DATA_BLOCK_START)

#define BLOCK_SIZE 4096
#define TOTAL_BLOCKS 64
#define INODE_TABLE_BLOCKS 5
#define INODE_SIZE 256
#define MAX_INODES (INODE_TABLE_BLOCKS * BLOCK_SIZE / INODE_SIZE)

#define SUPERBLOCK_MAGIC 0xd34d
#define SUPERBLOCK_BLOCK 0
#define INODE_BITMAP_BLOCK 1
#define DATA_BITMAP_BLOCK 2
#define INODE_TABLE_START 3
#define DATA_BLOCK_START 8

#pragma pack(push, 1)
typedef struct {
    uint16_t magic;
    uint32_t block_size;
    uint32_t total_blocks;
    uint32_t inode_bitmap_block;
    uint32_t data_bitmap_block;
    uint32_t inode_table_start;
    uint32_t first_data_block;
    uint32_t inode_size;
    uint32_t inode_count;
    char reserved[4058];
} Superblock;

typedef struct {
    uint32_t mode;
    uint32_t uid;
    uint32_t gid;
    uint32_t size;
    uint32_t atime;
    uint32_t ctime;
    uint32_t mtime;
    uint32_t dtime;
    uint32_t links_count;
    uint32_t blocks;
    uint32_t direct_block;
    uint32_t single_indirect;
    uint32_t double_indirect;
    uint32_t triple_indirect;
    char reserved[156];
} Inode;
#pragma pack(pop)

void fatal(const char *msg) {
    fprintf(stderr, "Error: %s\n", msg);
    exit(1);
}

void read_block(FILE *img, int block_num, void *buffer) {
    if (fseek(img, block_num * BLOCK_SIZE, SEEK_SET) != 0)
        fatal("Seek failed");
    if (fread(buffer, BLOCK_SIZE, 1, img) != 1)
        fatal("Read failed");
}

// Check the superblock
void check_superblock(Superblock *sb) {
    printf("Checking Superblock...\n");
    if (sb->magic != SUPERBLOCK_MAGIC) fatal("Invalid magic number");
    if (sb->block_size != BLOCK_SIZE) fatal("Invalid block size");
    if (sb->total_blocks != TOTAL_BLOCKS) fatal("Invalid total block count");
    if (sb->inode_bitmap_block != INODE_BITMAP_BLOCK) fatal("Invalid inode bitmap block");
    if (sb->data_bitmap_block != DATA_BITMAP_BLOCK) fatal("Invalid data bitmap block");
    if (sb->inode_table_start != INODE_TABLE_START) fatal("Invalid inode table start");
    if (sb->first_data_block != DATA_BLOCK_START) fatal("Invalid first data block");
    if (sb->inode_size != INODE_SIZE) fatal("Invalid inode size");
    if (sb->inode_count > MAX_INODES) fatal("Too many inodes");

    printf("Superblock OK ✅\n");
}
void check_data_bitmap_consistency(Superblock *sb, Inode *inodes, uint8_t *data_bitmap) {
    bool block_used_by_inode[DATA_BLOCK_COUNT] = {false};
    bool bitmap_set[DATA_BLOCK_COUNT] = {false};

    printf("\nChecking Data Bitmap Consistency...\n");

    // Step 1: Mark blocks used by valid inodes
    for (int i = 0; i < sb->inode_count; ++i) {
        Inode *inode = &inodes[i];

        // Skip invalid inodes
        if (inode->links_count == 0 || inode->dtime != 0)
            continue;

        // Check all blocks assigned to this inode (assumes contiguous from direct_block)
        for (uint32_t j = 0; j < inode->blocks; ++j) {
            uint32_t block = inode->direct_block + j;

            if (block < DATA_BLOCK_START || block >= TOTAL_BLOCKS) {
                printf("❌ Inode %d references invalid block %d\n", i, block);
                continue;
            }

            int relative_index = block - DATA_BLOCK_START;
            block_used_by_inode[relative_index] = true;
        }
    }

    // Step 2: Parse data bitmap
    for (int i = 0; i < DATA_BLOCK_COUNT; ++i) {
        int byte = i / 8;
        int bit = i % 8;
        bitmap_set[i] = (data_bitmap[byte] >> bit) & 1;
    }

    // Step 3: Compare and report mismatches
    for (int i = 0; i < DATA_BLOCK_COUNT; ++i) {
        int abs_block = DATA_BLOCK_START + i;

        if (bitmap_set[i] && !block_used_by_inode[i]) {
            printf("❌ Block %d is marked used in bitmap but not referenced by any inode\n", abs_block);
        } else if (!bitmap_set[i] && block_used_by_inode[i]) {
            printf("❌ Block %d is referenced by inode but not marked used in bitmap\n", abs_block);
        }
    }

    printf("Data Bitmap Consistency Check Complete ✅\n");
}
void check_inode_bitmap_consistency(Superblock *sb, Inode *inodes, uint8_t *inode_bitmap) {
    printf("\nChecking Inode Bitmap Consistency...\n");

    for (int i = 0; i < sb->inode_count; ++i) {
        int byte_index = i / 8;
        int bit_index = i % 8;
        bool bit_set = (inode_bitmap[byte_index] >> bit_index) & 1;

        bool valid_inode = (inodes[i].links_count > 0) && (inodes[i].dtime == 0);

        // (a) If bit is set, inode must be valid
        if (bit_set && !valid_inode) {
            printf("❌ Inode %d is marked in bitmap but is invalid (links=%d, dtime=%d)\n",
                   i, inodes[i].links_count, inodes[i].dtime);
        }

        // (b) If inode is valid, bit must be set
        if (!bit_set && valid_inode) {
            printf("❌ Inode %d is valid but not marked in bitmap\n", i);
        }
    }

    printf("Inode Bitmap Consistency Check Complete ✅\n");
}

int main() {
    FILE *img = fopen("vsfs.img", "rb");
    if (!img) fatal("Cannot open vsfs.img");

    //super block system
    Superblock sb;
    read_block(img, SUPERBLOCK_BLOCK, &sb);
    check_superblock(&sb);
    //data block system
    uint8_t data_bitmap[BLOCK_SIZE];
    read_block(img, DATA_BITMAP_BLOCK, data_bitmap);

    // Load inodes
    Inode inodes[MAX_INODES];
    for (int i = 0; i < INODE_TABLE_BLOCKS; ++i) {
        read_block(img, INODE_TABLE_START + i, ((uint8_t *)inodes) + i * BLOCK_SIZE);
    }

    // Run the checker
    check_data_bitmap_consistency(&sb, inodes, data_bitmap);
    uint8_t inode_bitmap[BLOCK_SIZE];
    read_block(img, INODE_BITMAP_BLOCK, inode_bitmap);
    check_inode_bitmap_consistency(&sb, inodes, inode_bitmap);


    fclose(img);

    return 0;
}
