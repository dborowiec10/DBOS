#ifndef EXT2_H
#define EXT2_H
#include <stdint.h>
#include <stddef.h>

#define EXT2_MAGIC 0xEF53
#define EXT2_BLOCKSIZE 1024

#define EXT2_ISOCK 0xC000
#define EXT2_ISLNK 0xA000
#define EXT2_IREGF 0x8000
#define EXT2_IBLKD 0x6000
#define EXT2_ICHRD 0x2000
#define EXT2_IDIRE 0x4000
#define EXT2_IFIFO 0x1000

#define EXT2_IRUSR 0x0100
#define EXT2_IWUSR 0x0080
#define EXT2_IXUSR 0x0040

#define EXT2_IRGRP 0x0020
#define EXT2_IWGRP 0x0010
#define EXT2_IXGRP 0x0008

#define EXT2_IROTH 0x0004
#define EXT2_IWOTH 0x0002
#define EXT2_IXOTH 0x0001

typedef struct ext2_superblock {
	uint32_t inodes_count;
	uint32_t blocks_count;
	uint32_t r_blocks_count;
	uint32_t free_blocks_count;
	uint32_t free_inodes_count;
	uint32_t first_data_block;
	uint32_t log_block_size;
	uint32_t log_frag_size;
	uint32_t blocks_per_group;
	uint32_t frags_per_group;
	uint32_t inodes_per_group;
	uint32_t mtime;
	uint32_t wtime;
	uint16_t mnt_count;
	uint16_t max_mnt_count;
	uint16_t magic;
	uint16_t state;
	uint16_t errors;
	uint16_t minor_rev_level;
	uint32_t lastcheck;
	uint32_t checkinterval;
	uint32_t creator_os;
	uint32_t rev_level;
	uint16_t def_resuid;
	uint16_t def_resgid;

	uint32_t first_ino;
	uint16_t inode_size;
	uint16_t block_group_nr;
	uint32_t feature_compat;
	uint32_t feature_incompat;
	uint32_t feature_ro_compat;

	uint8_t  uuid[16];
	uint8_t  volume_name[16];
	uint8_t  last_mounted[16];
	uint32_t algo_bitmap;

	uint8_t  prealloc_blocks;
	uint8_t  prealloc_dir_blocks;
	uint16_t alignment;

	uint8_t  journal_uuid[16];
	uint32_t journal_inum;
	uint32_t jounral_dev;
	uint32_t last_orphan;

	uint32_t hash_seed[4];
	uint8_t  def_hash_version;
	uint8_t  padding[3];

	uint32_t default_mount_options;
	uint32_t first_meta_bg;
	uint8_t  reserved[760];

} __attribute__((packed)) ext2_superblock_t;

typedef struct ext2_block_group_descriptor {
  uint32_t block_bitmap;
  uint32_t inode_bitmap;
  uint32_t inode_table;
  uint16_t free_blocks_count;
  uint16_t free_inodes_count;
  uint16_t used_dirs_count;
  uint16_t padding;
  uint8_t  reserved[12];
} __attribute__((packed)) ext2_block_group_descriptor_t;

typedef struct ext2_inode {
  uint16_t mode;
  uint16_t user_id;
  uint32_t size;
  uint32_t last_accessed_time;
  uint32_t created_time;
  uint32_t last_modified_time;
  uint32_t deleted_time;
  uint16_t group_id;
  uint16_t links_count;
  uint32_t reserved_blocks_count;
  uint32_t flags;
  uint32_t os_dependent;
  uint32_t direct_blocks[12];
  uint32_t single_indirect;
  uint32_t double_indirect;
  uint32_t triple_indirect;
  uint32_t generation;
  uint32_t file_ac1;
  uint32_t dir_ac1;
  uint32_t fragment_address;
  uint8_t  os_dependent2[12];
} __attribute__((packed)) ext2_inode_t;

typedef struct ext2_directory_entry {
	uint32_t inode_no;
	uint16_t record_length;
	uint8_t name_length;
	uint8_t file_type;
	char * name; // not sure about this
} ext2_dir_entry_t;


#endif
