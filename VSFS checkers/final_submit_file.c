#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define BLOCK_SIZE 4096
#define TOTAL_BLOCKS 64
#define INODE_SIZE 256
#define INODE_COUNT 80
#define DATA_BLOCK_START 8
#define DATA_BLOCK_COUNT 56
#define MAGIC_NUMBER 0xD34D

struct Superblock {
    uint16_t magic; // 0xD34D
    uint32_t block_size; // 4096
    uint32_t total_blocks; // 64
    uint32_t inode_bitmap_block; // 1
    uint32_t data_bitmap_block; // 2
    uint32_t inode_table_start; // 3
    uint32_t first_data_block; // 8
    uint32_t inode_size; // 256
    uint32_t inode_count; // 80
    uint8_t reserved[4058];
};

struct Inode {
    uint32_t mode;
    uint32_t uid;
    uint32_t gid;
    uint32_t size;
    uint32_t atime;
    uint32_t ctime;
    uint32_t mtime;
    uint32_t dtime;
    uint32_t links;
    uint32_t blocks;
    uint32_t direct[4];
    uint32_t single_indirect;
    uint32_t double_indirect;
    uint32_t triple_indirect;
    uint8_t reserved[156];
};

void read_inode(uint8_t *image, int inode_num, struct Inode *inode) {
    memcpy(inode, image + (3 * BLOCK_SIZE) + (inode_num * INODE_SIZE), INODE_SIZE);
}

void write_inode(uint8_t *image, int inode_num, struct Inode *inode) {
    memcpy(image + (3 * BLOCK_SIZE) + (inode_num * INODE_SIZE), inode, INODE_SIZE);
}

bool is_valid_inode(struct Inode *inode) {
    return inode->links > 0 && inode->dtime == 0;
}

bool validate_superblock(struct Superblock *sb, uint8_t *image) {
    bool errors = false;
    if (sb->magic != MAGIC_NUMBER) {
        printf("Error: Invalid magic number (0x%X), fixing to 0x%X\n", sb->magic, MAGIC_NUMBER);
        sb->magic = MAGIC_NUMBER;
        errors = true;
    }
    if (sb->block_size != BLOCK_SIZE) {
        printf("Error: Invalid block size (%u), fixing to %u\n", sb->block_size, BLOCK_SIZE);
        sb->block_size = BLOCK_SIZE;
        errors = true;
    }
    if (sb->total_blocks != TOTAL_BLOCKS) {
        printf("Error: Invalid total blocks (%u), fixing to %u\n", sb->total_blocks, TOTAL_BLOCKS);
        sb->total_blocks = TOTAL_BLOCKS;
        errors = true;
    }
    if (sb->inode_bitmap_block != 1) {
        printf("Error: Invalid inode bitmap block (%u), fixing to 1\n", sb->inode_bitmap_block);
        sb->inode_bitmap_block = 1;
        errors = true;
    }
    if (sb->data_bitmap_block != 2) {
        printf("Error: Invalid data bitmap block (%u), fixing to 2\n", sb->data_bitmap_block);
        sb->data_bitmap_block = 2;
        errors = true;
    }
    if (sb->inode_table_start != 3) {
        printf("Error: Invalid inode table start (%u), fixing to 3\n", sb->inode_table_start);
        sb->inode_table_start = 3;
        errors = true;
    }
    if (sb->first_data_block != DATA_BLOCK_START) {
        printf("Error: Invalid first data block (%u), fixing to %u\n", sb->first_data_block, DATA_BLOCK_START);
        sb->first_data_block = DATA_BLOCK_START;
        errors = true;
    }
    if (sb->inode_size != INODE_SIZE) {
        printf("Error: Invalid inode size (%u), fixing to %u\n", sb->inode_size, INODE_SIZE);
        sb->inode_size = INODE_SIZE;
        errors = true;
    }
    if (sb->inode_count != INODE_COUNT) {
        printf("Error: Invalid inode count (%u), fixing to %u\n", sb->inode_count, INODE_COUNT);
        sb->inode_count = INODE_COUNT;
        errors = true;
    }
    if (errors) {
        memcpy(image, sb, sizeof(struct Superblock)); // Write back fixes
    } else {
        printf(".img files Superblock is valid\n");
    }
    return !errors;
}

void collect_indirect_blocks(uint8_t *image, uint32_t block_num, bool *used_blocks) {
    if (block_num < DATA_BLOCK_START || block_num >= TOTAL_BLOCKS) return;
    uint32_t block[BLOCK_SIZE / 4];
    memcpy(block, image + block_num * BLOCK_SIZE, BLOCK_SIZE);
    for (int i = 0; i < BLOCK_SIZE / 4; i++) {
        if (block[i] >= DATA_BLOCK_START && block[i] < TOTAL_BLOCKS) {
            used_blocks[block[i] - DATA_BLOCK_START] = true;
        }
    }
}

void collect_double_indirect_blocks(uint8_t *image, uint32_t block_num, bool *used_blocks) {
    if (block_num < DATA_BLOCK_START || block_num >= TOTAL_BLOCKS) return;
    uint32_t block[BLOCK_SIZE / 4];
    memcpy(block, image + block_num * BLOCK_SIZE, BLOCK_SIZE);
    for (int i = 0; i < BLOCK_SIZE / 4; i++) {
        if (block[i] != 0) {
            collect_indirect_blocks(image, block[i], used_blocks);
        }
    }
}

void collect_triple_indirect_blocks(uint8_t *image, uint32_t block_num, bool *used_blocks) {
    if (block_num < DATA_BLOCK_START || block_num >= TOTAL_BLOCKS) return;
    uint32_t block[BLOCK_SIZE / 4];
    memcpy(block, image + block_num * BLOCK_SIZE, BLOCK_SIZE);
    for (int i = 0; i < BLOCK_SIZE / 4; i++) {
        if (block[i] != 0) {
            collect_double_indirect_blocks(image, block[i], used_blocks);
        }
    }
}


void check_data_bitmap(uint8_t *image, struct Superblock *sb) {
    uint8_t data_bitmap[BLOCK_SIZE];
    memcpy(data_bitmap, image + 2 * BLOCK_SIZE, BLOCK_SIZE);
    bool used_blocks[DATA_BLOCK_COUNT] = {0};
    bool bitmap_blocks[DATA_BLOCK_COUNT] = {0};

    for (int i = 0; i < DATA_BLOCK_COUNT; i++) {
        if (data_bitmap[i / 8] & (1 << (i % 8))) {
            bitmap_blocks[i] = true;
        }
    }


    for (int i = 0; i < INODE_COUNT; i++) {
        struct Inode inode;
        read_inode(image, i, &inode);
        if (is_valid_inode(&inode)) {
        
            for (int j = 0; j < 4; j++) {
                if (inode.direct[j] >= DATA_BLOCK_START && inode.direct[j] < TOTAL_BLOCKS) {
                    used_blocks[inode.direct[j] - DATA_BLOCK_START] = true;
                }
            }
            
            if (inode.single_indirect != 0) {
                collect_indirect_blocks(image, inode.single_indirect, used_blocks);
            }
            if (inode.double_indirect != 0) {
                collect_double_indirect_blocks(image, inode.double_indirect, used_blocks);
            }
            if (inode.triple_indirect != 0) {
                collect_triple_indirect_blocks(image, inode.triple_indirect, used_blocks);
            }
        }
    }

    
    bool errors = false;
    for (int i = 0; i < DATA_BLOCK_COUNT; i++) {
        if (bitmap_blocks[i] && !used_blocks[i]) {
            printf("Error: Block %d marked used but not referenced\n", i + DATA_BLOCK_START);
            data_bitmap[i / 8] &= ~(1 << (i % 8)); // Clear bit
            errors = true;
        }
        if (used_blocks[i] && !bitmap_blocks[i]) {
            printf("Error: Block %d referenced but not marked used\n", i + DATA_BLOCK_START);
            data_bitmap[i / 8] |= (1 << (i % 8)); // Set bit
            errors = true;
        }
    }

    if (errors) {
        memcpy(image + 2 * BLOCK_SIZE, data_bitmap, BLOCK_SIZE); // Write back
    } else {
        printf(".img files Data bitmap is consistent\n");
    }
}


void check_inode_bitmap(uint8_t *image, struct Superblock *sb) {
    uint8_t inode_bitmap[BLOCK_SIZE];
    memcpy(inode_bitmap, image + 1 * BLOCK_SIZE, BLOCK_SIZE);
    bool bitmap_inodes[INODE_COUNT] = {0};
    bool valid_inodes[INODE_COUNT] = {0};

    for (int i = 0; i < INODE_COUNT; i++) {
        if (inode_bitmap[i / 8] & (1 << (i % 8))) {
            bitmap_inodes[i] = true;
        }
    }

    for (int i = 0; i < INODE_COUNT; i++) {
        struct Inode inode;
        read_inode(image, i, &inode);
        if (is_valid_inode(&inode)) {
            valid_inodes[i] = true;
        }
    }


    bool errors = false;
    for (int i = 0; i < INODE_COUNT; i++) {
        if (bitmap_inodes[i] && !valid_inodes[i]) {
            printf("Error: Inode %d marked used but invalid\n", i);
            inode_bitmap[i / 8] &= ~(1 << (i % 8)); // Clear bit
            errors = true;
        }
        if (valid_inodes[i] && !bitmap_inodes[i]) {
            printf("Error: Inode %d valid but not marked used\n", i);
            inode_bitmap[i / 8] |= (1 << (i % 8)); // Set bit
            errors = true;
        }
    }

    if (errors) {
        memcpy(image + 1 * BLOCK_SIZE, inode_bitmap, BLOCK_SIZE); // Write back
    } else {
        printf(".img files Inode bitmap is consistent\n");
    }
}


void check_duplicates(uint8_t *image, struct Superblock *sb) {
    int block_ref_count[DATA_BLOCK_COUNT] = {0};
    int block_owner[DATA_BLOCK_COUNT] = {-1};

    for (int i = 0; i < INODE_COUNT; i++) {
        struct Inode inode;
        read_inode(image, i, &inode);
        if (is_valid_inode(&inode)) {
            // Direct blocks
            for (int j = 0; j < 4; j++) {
                if (inode.direct[j] >= DATA_BLOCK_START && inode.direct[j] < TOTAL_BLOCKS) {
                    int idx = inode.direct[j] - DATA_BLOCK_START;
                    block_ref_count[idx]++;
                    block_owner[idx] = i;
                }
            }
            // Indirect blocks
            bool used_blocks[DATA_BLOCK_COUNT] = {0};
            if (inode.single_indirect != 0) {
                collect_indirect_blocks(image, inode.single_indirect, used_blocks);
            }
            if (inode.double_indirect != 0) {
                collect_double_indirect_blocks(image, inode.double_indirect, used_blocks);
            }
            if (inode.triple_indirect != 0) {
                collect_triple_indirect_blocks(image, inode.triple_indirect, used_blocks);
            }
            for (int j = 0; j < DATA_BLOCK_COUNT; j++) {
                if (used_blocks[j]) {
                    block_ref_count[j]++;
                    block_owner[j] = i;
                }
            }
        }
    }

    bool errors = false;
    for (int i = 0; i < DATA_BLOCK_COUNT; i++) {
        if (block_ref_count[i] > 1) {
            printf("Error: Block %d referenced by %d inodes\n", i + DATA_BLOCK_START, block_ref_count[i]);
            errors = true;
            
        }
    }
    if (!errors) {
        printf("No duplicate block references found\n");
    }
}


void check_bad_blocks(uint8_t *image, struct Superblock *sb) {
    bool errors = false;
    for (int i = 0; i < INODE_COUNT; i++) {
        struct Inode inode;
        read_inode(image, i, &inode);
        if (is_valid_inode(&inode)) {
            for (int j = 0; j < 4; j++) {
                if (inode.direct[j] != 0 && (inode.direct[j] < DATA_BLOCK_START || inode.direct[j] >= TOTAL_BLOCKS)) {
                    printf("Error: Inode %d has invalid direct block pointer %u\n", i, inode.direct[j]);
                    inode.direct[j] = 0;
                    errors = true;
                }
            }
            if (inode.single_indirect != 0 && (inode.single_indirect < DATA_BLOCK_START || inode.single_indirect >= TOTAL_BLOCKS)) {
                printf("Error: Inode %d has invalid single indirect pointer %u\n", i, inode.single_indirect);
                inode.single_indirect = 0;
                errors = true;
            }
            if (inode.double_indirect != 0 && (inode.double_indirect < DATA_BLOCK_START || inode.double_indirect >= TOTAL_BLOCKS)) {
                printf("Error: Inode %d has invalid double indirect pointer %u\n", i, inode.double_indirect);
                inode.double_indirect = 0;
                errors = true;
            }
            if (inode.triple_indirect != 0 && (inode.triple_indirect < DATA_BLOCK_START || inode.triple_indirect >= TOTAL_BLOCKS)) {
                printf("Error: Inode %d has invalid triple indirect pointer %u\n", i, inode.triple_indirect);
                inode.triple_indirect = 0;
                errors = true;
            }
            if (errors) {
                write_inode(image, i, &inode); // Write back fixed inode
            }
        }
    }
    if (!errors) {
        printf("No bad block pointers found\n");
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <vsfs.img>\n", argv[0]);
        return 1;
    }


    int fd = open(argv[1], O_RDWR);
    if (fd < 0) {
        perror("Failed to open image");
        return 1;
    }

    struct stat st;
    if (fstat(fd, &st) < 0) {
    perror("Failed to stat image");
    close(fd);
    return 1;
}
    if (st.st_size != TOTAL_BLOCKS * BLOCK_SIZE) {
        printf("Error: Image size (%lld) does not match expected (%d)\n",
     st.st_size, TOTAL_BLOCKS * BLOCK_SIZE);
      close(fd);
     return 1;
    }

    uint8_t *image = mmap(NULL, TOTAL_BLOCKS * BLOCK_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (image == MAP_FAILED) {
        perror("Failed to map image");
        close(fd);
        return 1;
    }

    struct Superblock sb;
    memcpy(&sb, image, sizeof(struct Superblock));
    printf("WELCOME ....\n");
    printf("=== A Consistency Checker for Very Simple File System (VSFS)===\n");
    printf("Checking Superblock...\n");
    validate_superblock(&sb, image);
    printf("Checking Inode Bitmap...\n");
    check_inode_bitmap(image, &sb);
    printf("Checking Data Bitmap...\n");
    check_data_bitmap(image, &sb);
    printf("Checking for Duplicate Blocks...\n");
    check_duplicates(image, &sb);
    printf("Checking for Bad Blocks...\n");
    check_bad_blocks(image, &sb);

    if (msync(image, TOTAL_BLOCKS * BLOCK_SIZE, MS_SYNC) < 0) {
        perror("Failed to sync image");
    }

    printf("\n===Verifying Again for any error===\n");
    memcpy(&sb, image, sizeof(struct Superblock)); // Reload superblock
    bool superblock_valid = validate_superblock(&sb, image);
    check_inode_bitmap(image, &sb);
    check_data_bitmap(image, &sb);
    check_duplicates(image, &sb);
    check_bad_blocks(image, &sb);

    munmap(image, TOTAL_BLOCKS * BLOCK_SIZE);
    close(fd);

    printf("\nFile system check has been completed\n");
    if (superblock_valid) {
        printf("All checks passed successfully\n");
    } else {
        printf("Some errors were fixed, but re-verification may have found issues\n");
    }
    return 0;
}