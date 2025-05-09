#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

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

int main() {
    FILE *img = fopen("vsfs.img", "rb");
    if (!img) fatal("Cannot open vsfs.img");

    //super block system
    Superblock sb;
    read_block(img, SUPERBLOCK_BLOCK, &sb);
    check_superblock(&sb);

    fclose(img);
    return 0;
}
