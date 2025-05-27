#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>

#define BLOCK_SIZE 4096
#define TOTAL_BLOCKS 64
#define INODE_SIZE 256
#define INODE_COUNT (5 * BLOCK_SIZE / INODE_SIZE) 
#define MAGIC_NUMBER 0xD34D
#define SUPERBLOCK_BLOCK 0
#define INODE_BITMAP_BLOCK 1
#define DATA_BITMAP_BLOCK 2
#define INODE_TABLE_START_BLOCK 3
#define DATA_BLOCK_START 8
#define DATA_BLOCK_COUNT (TOTAL_BLOCKS - DATA_BLOCK_START)

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
    uint8_t reserved[4058];
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
    uint32_t blocks_count;
    uint32_t direct[1];
    uint32_t single_indirect;
    uint32_t double_indirect;
    uint32_t triple_indirect;
    uint8_t reserved[156];
} Inode;


uint8_t *fs_image;
size_t fs_size;
Superblock *superblock;
uint8_t *inode_bitmap;
uint8_t *data_bitmap;
Inode *inode_table;
int errors_detected = 0;
int errors_fixed = 0;

int get_bitmap_bit(uint8_t *bitmap, uint32_t index) {
    return (bitmap[index / 8] >> (index % 8)) & 1;
}

void set_bitmap_bit(uint8_t *bitmap, uint32_t index, int value) {
    if (value)
        bitmap[index / 8] |= (1 << (index % 8));
    else
        bitmap[index / 8] &= ~(1 << (index % 8));
}

int is_valid_inode(Inode *inode) {
    return inode->links_count > 0 && inode->dtime == 0;
}

void superblock_validation_checker() {
    printf("Welcome to A Consistency Checker for Very Simple File System (VSFS)\n");
    printf("### Superblock Validator ###\n");
    printf("Checking superblock...\n");
    if (superblock->magic != MAGIC_NUMBER) {
        printf("Error: Invalid magic number (0x%04x, expected 0x%04x)\n", superblock->magic, MAGIC_NUMBER);
        superblock->magic = MAGIC_NUMBER;
        errors_detected++;
        errors_fixed++;
        printf("Fixed: Set magic number to 0x%04x\n", MAGIC_NUMBER);
    }
    if (superblock->block_size != BLOCK_SIZE) {
        printf("Error: Invalid block size (%u, expected %u)\n", superblock->block_size, BLOCK_SIZE);
        superblock->block_size = BLOCK_SIZE;
        errors_detected++;
        errors_fixed++;
        printf("Fixed: Set block size to %u\n", BLOCK_SIZE);
    }
    if (superblock->total_blocks != TOTAL_BLOCKS) {
        printf("Error: Invalid total blocks (%u, expected %u)\n", superblock->total_blocks, TOTAL_BLOCKS);
        superblock->total_blocks = TOTAL_BLOCKS;
        errors_detected++;
        errors_fixed++;
        printf("Fixed: Set total blocks to %u\n", TOTAL_BLOCKS);
    }
    if (superblock->inode_bitmap_block != INODE_BITMAP_BLOCK) {
        printf("Error: Invalid inode bitmap block (%u, expected %u)\n", superblock->inode_bitmap_block, INODE_BITMAP_BLOCK);
        superblock->inode_bitmap_block = INODE_BITMAP_BLOCK;
        errors_detected++;
        errors_fixed++;
        printf("Fixed: Set inode bitmap block to %u\n", INODE_BITMAP_BLOCK);
    }
    if (superblock->data_bitmap_block != DATA_BITMAP_BLOCK) {
        printf("Error: Invalid data bitmap block (%u, expected %u)\n", superblock->data_bitmap_block, DATA_BITMAP_BLOCK);
        superblock->data_bitmap_block = DATA_BITMAP_BLOCK;
        errors_detected++;
        errors_fixed++;
        printf("Fixed: Set data bitmap block to %u\n", DATA_BITMAP_BLOCK);
    }
    if (superblock->inode_table_start != INODE_TABLE_START_BLOCK) {
        printf("Error: Invalid inode table start (%u, expected %u)\n", superblock->inode_table_start, INODE_TABLE_START_BLOCK);
        superblock->inode_table_start = INODE_TABLE_START_BLOCK;
        errors_detected++;
        errors_fixed++;
        printf("Fixed: Set inode table start to %u\n", INODE_TABLE_START_BLOCK);
    }
    if (superblock->first_data_block != DATA_BLOCK_START) {
        printf("Error: Invalid first data block (%u, expected %u)\n", superblock->first_data_block, DATA_BLOCK_START);
        superblock->first_data_block = DATA_BLOCK_START;
        errors_detected++;
        errors_fixed++;
        printf("Fixed: Set first data block to %u\n", DATA_BLOCK_START);
    }
    if (superblock->inode_size != INODE_SIZE) {
        printf("Error: Invalid inode size (%u, expected %u)\n", superblock->inode_size, INODE_SIZE);
        superblock->inode_size = INODE_SIZE;
        errors_detected++;
        errors_fixed++;
        printf("Fixed: Set inode size to %u\n", INODE_SIZE);
    }
    if (superblock->inode_count != INODE_COUNT) {
        printf("Error: Invalid inode count (%u, expected %u)\n", superblock->inode_count, INODE_COUNT);
        superblock->inode_count = INODE_COUNT;
        errors_detected++;
        errors_fixed++;
        printf("Fixed: Set inode count to %u\n", INODE_COUNT);
    }
    printf("Superblock validation complete.\n\n");
}

void inode_bitmap_checker() {
    printf("### Inode Bitmap Consistency Checker ###\n");
    printf("Checking inode bitmap...\n");
    for (uint32_t i = 0; i < INODE_COUNT; i++) {
        int bitmap_bit = get_bitmap_bit(inode_bitmap, i);
        int valid_inode = is_valid_inode(&inode_table[i]);
        if (bitmap_bit && !valid_inode) {
            printf("Error: Inode %u marked used in bitmap but is invalid (links=%u, dtime=%u)\n", 
                   i, inode_table[i].links_count, inode_table[i].dtime);
            set_bitmap_bit(inode_bitmap, i, 0);
            errors_detected++;
            errors_fixed++;
            printf("Fixed: Cleared inode %u in bitmap\n", i);
        } else if (!bitmap_bit && valid_inode) {
            printf("Error: Inode %u is valid but not marked in bitmap\n", i);
            set_bitmap_bit(inode_bitmap, i, 1);
            errors_detected++;
            errors_fixed++;
            printf("Fixed: Set inode %u in bitmap\n", i);
        }
    }
    printf("Inode bitmap validation complete.\n\n");
}

void data_bitmap_and_bad_block_checker(uint32_t *block_refs, uint32_t *dup_blocks) {
    printf("### Data Bitmap and Bad Block Checker ###\n");
    printf("Checking data bitmap and bad blocks...\n");
    memset(block_refs, 0, TOTAL_BLOCKS * sizeof(uint32_t));

    for (uint32_t i = 0; i < INODE_COUNT; i++) {
        if (!is_valid_inode(&inode_table[i])) continue;
        Inode *inode = &inode_table[i];
        if (inode->direct[0] != 0) {
            if (inode->direct[0] < DATA_BLOCK_START || inode->direct[0] >= TOTAL_BLOCKS) {
                printf("Error: Inode %u has invalid direct block %u (out of range)\n", i, inode->direct[0]);
                inode->direct[0] = 0;
                errors_detected++;
                errors_fixed++;
                printf("Fixed: Cleared invalid direct block in inode %u\n", i);
            } else {
                block_refs[inode->direct[0]]++;
            }
        }
    }

    for (uint32_t i = DATA_BLOCK_START; i < TOTAL_BLOCKS; i++) {
        int bitmap_bit = get_bitmap_bit(data_bitmap, i - DATA_BLOCK_START);
        int referenced = block_refs[i] > 0;
        if (bitmap_bit && !referenced) {
            printf("Error: Data block %u marked used in bitmap but not referenced\n", i);
            set_bitmap_bit(data_bitmap, i - DATA_BLOCK_START, 0);
            errors_detected++;
            errors_fixed++;
            printf("Fixed: Cleared data block %u in bitmap\n", i);
        } else if (!bitmap_bit && referenced) {
            printf("Error: Data block %u referenced but not marked in bitmap\n", i);
            set_bitmap_bit(data_bitmap, i - DATA_BLOCK_START, 1);
            errors_detected++;
            errors_fixed++;
            printf("Fixed: Set data block %u in bitmap\n", i);
        }
    }
    printf("Data bitmap and bad block validation complete.\n\n");
}


void duplicate_checker(uint32_t *block_refs, uint32_t *dup_blocks) {
    printf("### Duplicate Block Checker ###\n");
    printf("Checking for duplicate blocks...\n");
    memset(dup_blocks, 0, TOTAL_BLOCKS * sizeof(uint32_t));
    for (uint32_t i = DATA_BLOCK_START; i < TOTAL_BLOCKS; i++) {
        if (block_refs[i] > 1) {
            printf("Error: Data block %u referenced by %u inodes\n", i, block_refs[i]);
            dup_blocks[i] = 1;
            errors_detected++;
            printf("Warning: Duplicate block %u not fixed (requires complex reallocation)\n", i);
        }
    }
    printf("Duplicate block check complete.\n\n");
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <fs_image>\n", argv[0]);
        exit(1);
    }

    int fd = open(argv[1], O_RDWR);
    if (fd < 0) {
        perror("Failed to open file system image");
        exit(1);
    }

    struct stat st;
    if (fstat(fd, &st) < 0) {
        perror("Failed to stat file system image");
        close(fd);
        exit(1);
    }
    fs_size = st.st_size;
    if (fs_size < TOTAL_BLOCKS * BLOCK_SIZE) {
        fprintf(stderr, "File system image too small\n");
        close(fd);
        exit(1);
    }

    fs_image = mmap(NULL, fs_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (fs_image == MAP_FAILED) {
        perror("Failed to map file system image");
        close(fd);
        exit(1);
    }

    superblock = (Superblock *)(fs_image + SUPERBLOCK_BLOCK * BLOCK_SIZE);
    inode_bitmap = fs_image + INODE_BITMAP_BLOCK * BLOCK_SIZE;
    data_bitmap = fs_image + DATA_BITMAP_BLOCK * BLOCK_SIZE;
    inode_table = (Inode *)(fs_image + INODE_TABLE_START_BLOCK * BLOCK_SIZE);

    uint32_t block_refs[TOTAL_BLOCKS];
    uint32_t dup_blocks[TOTAL_BLOCKS];
    superblock_validation_checker();
    inode_bitmap_checker();
    data_bitmap_and_bad_block_checker(block_refs, dup_blocks);
    duplicate_checker(block_refs, dup_blocks);

    if (msync(fs_image, fs_size, MS_SYNC) < 0) {
        perror("Failed to sync file system image");
    }

    munmap(fs_image, fs_size);
    close(fd);

    printf("Your provided .img file has:\n");
    printf("Total errors: %d\n", errors_detected);
    printf("Total errors fixed by our checker: %d\n", errors_fixed);
    if (errors_detected == errors_fixed) {
        printf("File system is now consistent.\n");
    } else {
        printf("Some errors could not be fixed.\n");
    }

    return errors_detected == errors_fixed ? 0 : 1;
}