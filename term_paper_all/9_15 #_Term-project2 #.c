#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>

#define BLOCK_SIZE_VAR 4096
#define TOTAL_BLOCKS_VAR 64
#define INODE_SIZE_VAR 256
#define INODE_COUNT_VAR (5 * BLOCK_SIZE_VAR / INODE_SIZE_VAR) 
#define MAGIC_NUMBER_VAR 0xD34D
#define SUPERBLOCK_BLOCK_VAR 0
#define INODE_BITMAP_BLOCK_VAR 1
#define DATA_BITMAP_BLOCK_VAR 2
#define INODE_TABLE_START_BLOCK_VAR 3
#define DATA_BLOCK_START_VAR 8
#define DATA_BLOCK_COUNT_VAR (TOTAL_BLOCKS_VAR - DATA_BLOCK_START_VAR)

typedef struct {
    uint16_t magic_var;
    uint32_t block_size_var;
    uint32_t block_count_var;
    uint32_t inode_bitmap_block_var;
    uint32_t data_bitmap_block_var;
    uint32_t inode_table_start_var;
    uint32_t first_data_block_var;
    uint32_t inode_size_var;
    uint32_t inode_count_var;
    uint8_t reserved_var[4058];
} Superblock;

typedef struct {
    uint32_t mode_var;
    uint32_t uid_var;
    uint32_t gid_var;
    uint32_t size_var;
    uint32_t atime_var;
    uint32_t ctime_var;
    uint32_t mtime_var;
    uint32_t dtime_var;
    uint32_t links_count_var;
    uint32_t blocks_count_var;
    uint32_t direct_var[1];
    uint32_t single_indirect_var;
    uint32_t double_indirect_var;
    uint32_t triple_indirect_var;
    uint8_t reserved_var[156];
} Inode;

uint8_t *fs_image_var;
size_t fs_size_var;
Superblock *superblock_var;
uint8_t *inode_bitmap_var;
uint8_t *data_bitmap_var;
Inode *inode_table_var;
int errors_detected_var = 0;
int errors_fixed_var = 0;

int get_bitmap_bit_fun(uint8_t *bitmap_var, uint32_t index_var) {
    return (bitmap_var[index_var / 8] >> (index_var % 8)) & 1;
}

void set_bitmap_bit_fun(uint8_t *bitmap_var, uint32_t index_var, int value_var) {
    if (value_var)
        bitmap_var[index_var / 8] |= (1 << (index_var % 8));
    else
        bitmap_var[index_var / 8] &= ~(1 << (index_var % 8));
}

int is_valid_inode_fun(Inode *inode_var) {
    return inode_var->links_count_var > 0 && inode_var->dtime_var == 0;
}

void superblock_validation_checker_fun() {
    printf("Welcome to A Consistency Checker for Very Simple File System (VSFS)\n");
    printf("### Superblock Validator ###\n");
    printf("Checking superblock...\n");
    if (superblock_var->magic_var != MAGIC_NUMBER_VAR) {
        printf("Error: Invalid magic number (0x%04x, expected 0x%04x)\n", superblock_var->magic_var, MAGIC_NUMBER_VAR);
        superblock_var->magic_var = MAGIC_NUMBER_VAR;
        errors_detected_var++;
        errors_fixed_var++;
        printf("Fixed: Set magic number to 0x%04x\n", MAGIC_NUMBER_VAR);
    }
    if (superblock_var->block_size_var != BLOCK_SIZE_VAR) {
        printf("Error: Invalid block size (%u, expected %u)\n", superblock_var->block_size_var, BLOCK_SIZE_VAR);
        superblock_var->block_size_var = BLOCK_SIZE_VAR;
        errors_detected_var++;
        errors_fixed_var++;
        printf("Fixed: Set block size to %u\n", BLOCK_SIZE_VAR);
    }
    if (superblock_var->block_count_var != TOTAL_BLOCKS_VAR) {
        printf("Error: Invalid total blocks (%u, expected %u)\n", superblock_var->block_count_var, TOTAL_BLOCKS_VAR);
        superblock_var->block_count_var = TOTAL_BLOCKS_VAR;
        errors_detected_var++;
        errors_fixed_var++;
        printf("Fixed: Set total blocks to %u\n", TOTAL_BLOCKS_VAR);
    }
    if (superblock_var->inode_bitmap_block_var != INODE_BITMAP_BLOCK_VAR) {
        printf("Error: Invalid inode bitmap block (%u, expected %u)\n", superblock_var->inode_bitmap_block_var, INODE_BITMAP_BLOCK_VAR);
        superblock_var->inode_bitmap_block_var = INODE_BITMAP_BLOCK_VAR;
        errors_detected_var++;
        errors_fixed_var++;
        printf("Fixed: Set inode bitmap block to %u\n", INODE_BITMAP_BLOCK_VAR);
    }
    if (superblock_var->data_bitmap_block_var != DATA_BITMAP_BLOCK_VAR) {
        printf("Error: Invalid data bitmap block (%u, expected %u)\n", superblock_var->data_bitmap_block_var, DATA_BITMAP_BLOCK_VAR);
        superblock_var->data_bitmap_block_var = DATA_BITMAP_BLOCK_VAR;
        errors_detected_var++;
        errors_fixed_var++;
        printf("Fixed: Set data bitmap block to %u\n", DATA_BITMAP_BLOCK_VAR);
    }
    if (superblock_var->inode_table_start_var != INODE_TABLE_START_BLOCK_VAR) {
        printf("Error: Invalid inode table start (%u, expected %u)\n", superblock_var->inode_table_start_var, INODE_TABLE_START_BLOCK_VAR);
        superblock_var->inode_table_start_var = INODE_TABLE_START_BLOCK_VAR;
        errors_detected_var++;
        errors_fixed_var++;
        printf("Fixed: Set inode table start to %u\n", INODE_TABLE_START_BLOCK_VAR);
    }
    if (superblock_var->first_data_block_var != DATA_BLOCK_START_VAR) {
        printf("Error: Invalid first data block (%u, expected %u)\n", superblock_var->first_data_block_var, DATA_BLOCK_START_VAR);
        superblock_var->first_data_block_var = DATA_BLOCK_START_VAR;
        errors_detected_var++;
        errors_fixed_var++;
        printf("Fixed: Set first data block to %u\n", DATA_BLOCK_START_VAR);
    }
    if (superblock_var->inode_size_var != INODE_SIZE_VAR) {
        printf("Error: Invalid inode size (%u, expected %u)\n", superblock_var->inode_size_var, INODE_SIZE_VAR);
        superblock_var->inode_size_var = INODE_SIZE_VAR;
        errors_detected_var++;
        errors_fixed_var++;
        printf("Fixed: Set inode size to %u\n", INODE_SIZE_VAR);
    }
    if (superblock_var->inode_count_var != INODE_COUNT_VAR) {
        printf("Error: Invalid inode count (%u, expected %u)\n", superblock_var->inode_count_var, INODE_COUNT_VAR);
        superblock_var->inode_count_var = INODE_COUNT_VAR;
        errors_detected_var++;
        errors_fixed_var++;
        printf("Fixed: Set inode count to %u\n", INODE_COUNT_VAR);
    }
    printf("Superblock validation checking has been completed.\n\n");
}

void inode_bitmap_checker_fun() {
    printf("### Inode Bitmap Consistency Checker ###\n");
    printf("Checking inode bitmap...\n");
    for (uint32_t i = 0; i < INODE_COUNT_VAR; i++) {
        int bitmap_bit_var = get_bitmap_bit_fun(inode_bitmap_var, i);
        int valid_inode_var = is_valid_inode_fun(&inode_table_var[i]);
        if (bitmap_bit_var && !valid_inode_var) {
            printf("Error Detected: Inode %u marked used in bitmap but invalid (links=%u, dtime=%u)\n", 
                   i, inode_table_var[i].links_count_var, inode_table_var[i].dtime_var);
            set_bitmap_bit_fun(inode_bitmap_var, i, 0);
            errors_detected_var++;
            errors_fixed_var++;
            printf("Fixed: Cleared inode %u in bitmap\n", i);
        } else if (!bitmap_bit_var && valid_inode_var) {
            printf("Error Detected: Inode %u is valid but not marked in bitmap\n", i);
            set_bitmap_bit_fun(inode_bitmap_var, i, 1);
            errors_detected_var++;
            errors_fixed_var++;
            printf("Fixed: Set inode %u in bitmap\n", i);
        }
    }
    printf("Inode bitmap validation checking has been completed.\n\n");
}

void data_bitmap_and_bad_block_checker_fun(uint32_t *block_refs_var, uint32_t *dup_blocks_var) {
    printf("### Data Bitmap and Bad Block Checker ###\n");
    printf("Checking data bitmap and bad blocks...\n");
    memset(block_refs_var, 0, TOTAL_BLOCKS_VAR * sizeof(uint32_t));

    for (uint32_t i = 0; i < INODE_COUNT_VAR; i++) {
        if (!is_valid_inode_fun(&inode_table_var[i])) continue;
        Inode *inode_var = &inode_table_var[i];
        if (inode_var->direct_var[0] != 0) {
            if (inode_var->direct_var[0] < DATA_BLOCK_START_VAR || inode_var->direct_var[0] >= TOTAL_BLOCKS_VAR) {
                printf("Error: Inode %u has invalid direct block %u (out of range)\n", i, inode_var->direct_var[0]);
                inode_var->direct_var[0] = 0;
                errors_detected_var++;
                errors_fixed_var++;
                printf("Fixed: Cleared invalid direct block in inode %u\n", i);
            } else {
                block_refs_var[inode_var->direct_var[0]]++;
            }
        }
    }

    for (uint32_t i = DATA_BLOCK_START_VAR; i < TOTAL_BLOCKS_VAR; i++) {
        int bitmap_bit_var = get_bitmap_bit_fun(data_bitmap_var, i - DATA_BLOCK_START_VAR);
        int referenced_var = block_refs_var[i] > 0;
        if (bitmap_bit_var && !referenced_var) {
            printf("Error: Data block %u marked used in bitmap but not referenced\n", i);
            set_bitmap_bit_fun(data_bitmap_var, i - DATA_BLOCK_START_VAR, 0);
            errors_detected_var++;
            errors_fixed_var++;
            printf("Fixed: Cleared data block %u in bitmap\n", i);
        } else if (!bitmap_bit_var && referenced_var) {
            printf("Error: Data block %u referenced but not marked in bitmap\n", i);
            set_bitmap_bit_fun(data_bitmap_var, i - DATA_BLOCK_START_VAR, 1);
            errors_detected_var++;
            errors_fixed_var++;
            printf("Fixed: Set data block %u in bitmap\n", i);
        }
    }
    printf("Data bitmap and bad block validation checking has been completed.\n\n");
}

void duplicate_checker_fun(uint32_t *block_refs_var, uint32_t *dup_blocks_var) {
    printf("### Duplicate Block Checker ###\n");
    printf("Checking for duplicate blocks...\n");
    memset(dup_blocks_var, 0, TOTAL_BLOCKS_VAR * sizeof(uint32_t));
    for (uint32_t i = DATA_BLOCK_START_VAR; i < TOTAL_BLOCKS_VAR; i++) {
        if (block_refs_var[i] > 1) {
            printf("Error: Data block %u referenced by %u inodes\n", i, block_refs_var[i]);
            dup_blocks_var[i] = 1;
            errors_detected_var++;
            printf("Warning: Duplicate block %u not fixed (requires complex reallocation)\n", i);
        }
    }
    printf("Duplicate block checking has been completed.\n\n");
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <fs_image>\n", argv[0]);
        exit(1);
    }

    int fd_var = open(argv[1], O_RDWR);
    if (fd_var < 0) {
        perror("Failed to open file system image");
        exit(1);
    }

    struct stat st_var;
    if (fstat(fd_var, &st_var) < 0) {
        perror("Failed to stat file system image");
        close(fd_var);
        exit(1);
    }
    fs_size_var = st_var.st_size;
    if (fs_size_var < TOTAL_BLOCKS_VAR * BLOCK_SIZE_VAR) {
        fprintf(stderr, "File system image too small\n");
        close(fd_var);
        exit(1);
    }

    fs_image_var = mmap(NULL, fs_size_var, PROT_READ | PROT_WRITE, MAP_SHARED, fd_var, 0);
    if (fs_image_var == MAP_FAILED) {
        perror("Failed to map file system image");
        close(fd_var);
        exit(1);
    }

    superblock_var = (Superblock *)(fs_image_var + SUPERBLOCK_BLOCK_VAR * BLOCK_SIZE_VAR);
    inode_bitmap_var = fs_image_var + INODE_BITMAP_BLOCK_VAR * BLOCK_SIZE_VAR;
    data_bitmap_var = fs_image_var + DATA_BITMAP_BLOCK_VAR * BLOCK_SIZE_VAR;
    inode_table_var = (Inode *)(fs_image_var + INODE_TABLE_START_BLOCK_VAR * BLOCK_SIZE_VAR);

    uint32_t block_refs_var[TOTAL_BLOCKS_VAR];
    uint32_t dup_blocks_var[TOTAL_BLOCKS_VAR];
    superblock_validation_checker_fun();
    inode_bitmap_checker_fun();
    data_bitmap_and_bad_block_checker_fun(block_refs_var, dup_blocks_var);
    duplicate_checker_fun(block_refs_var, dup_blocks_var);

    if (msync(fs_image_var, fs_size_var, MS_SYNC) < 0) {
        perror("Failed to sync file system image");
    }

    munmap(fs_image_var, fs_size_var);
    close(fd_var);

    printf("Your provided .img file has:\n");
    printf("Total errors: %d\n", errors_detected_var);
    printf("Total errors fixed by our Consistancy checker: %d\n", errors_fixed_var);
    if (errors_detected_var == errors_fixed_var) {
        printf("All the errors in your file system is fixed by our Consistancy checker.\n");
        printf("Your file system is now consistent.\n");
    } else {
        printf("Sorry. Some of your errors could not be fixed But we tried our best.\n");
    }

    return errors_detected_var == errors_fixed_var ? 0 : 1;
}