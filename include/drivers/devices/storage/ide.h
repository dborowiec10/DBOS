#ifndef IDE_H
#define IDE_H
#include <stddef.h>
#include <stdint.h>
#include <kernel/vfs/vfs.h>

// used to get the device from the pci driver
// identifies IDE ATA mass storage devices
#define IDE_ATA_PCI_CLASS_CODE              0x01
#define IDE_ATA_PCI_SUBCLASS_CODE           0x01

#define IDE_ATA_PARTITION_BOOTABLE          0x80

// used for initialisation - > confirmed using pci device
#define IDE_ATA_BAR0                        0x1F0
#define IDE_ATA_BAR1                        0x3F6
#define IDE_ATA_BAR2                        0x170
#define IDE_ATA_BAR3                        0x376
#define IDE_ATA_BAR4                        0x000

// channel status
#define IDE_ATA_STATUS_BUSY                 0x80
#define IDE_ATA_STATUS_DRIVE_READY          0x40
#define IDE_ATA_STATUS_DRIVE_WRITE_FAULT    0x20
#define IDE_ATA_STATUS_DRIVE_SEEK_COMPLETE  0x10
#define IDE_ATA_STATUS_DATA_REQUEST_READY   0x08
#define IDE_ATA_STATUS_CORRECTED_DATA       0x04
#define IDE_ATA_STATUS_INLEX                0x02
#define IDE_ATA_STATUS_ERROR                0x01

// read error bitmask
#define IDE_ATA_ERROR_BAD_SECTOR            0x80
#define IDE_ATA_ERROR_UNCORRECTABLE_DATA    0x40
#define IDE_ATA_ERROR_NO_MEDIA              0x20
#define IDE_ATA_ERROR_ID_NOT_FOUND          0x10
#define IDE_ATA_ERROR_NO_MEDIA_R            0x08
#define IDE_ATA_ERROR_COMMAND_ABORT         0x04
#define IDE_ATA_ERROR_TRACK0_NOT_FOUND      0x02
#define IDE_ATA_ERROR_NO_ADDRESS_MARK       0x01

// commands
#define IDE_ATA_COMMAND_READ_PIO            0x20
#define IDE_ATA_COMMAND_READ_PIO_EXT        0x24
#define IDE_ATA_COMMAND_READ_DMA            0xC8
#define IDE_ATA_COMMAND_READ_DMA_EXT        0x25
#define IDE_ATA_COMMAND_WRITE_PIO           0x30
#define IDE_ATA_COMMAND_WRITE_PIO_EXT       0x34
#define IDE_ATA_COMMAND_WRITE_DMA           0xCA
#define IDE_ATA_COMMAND_WRITE_DMA_EXT       0x35
#define IDE_ATA_COMMAND_CACHE_FLUSH         0xE7
#define IDE_ATA_COMMAND_CACHE_FLUSH_EXT     0xEA
#define IDE_ATA_COMMAND_PACKET              0xA0
#define IDE_ATA_COMMAND_IDENTIFY_PACKET     0xA1
#define IDE_ATA_COMMAND_IDENTIFY            0xEC

// identification space
#define IDE_ATA_IDENTIFY_DEVICETYPE         0
#define IDE_ATA_IDENTIFY_CYLINDERS          2
#define IDE_ATA_IDENTIFY_HEADS              6
#define IDE_ATA_IDENTIFY_SECTORS            12
#define IDE_ATA_IDENTIFY_SERIAL             20
#define IDE_ATA_IDENTIFY_MODEL              54
#define IDE_ATA_IDENTIFY_LBA28_0            60 // must be taken as uint32_t
#define IDE_ATA_IDENTIFY_LBA28_1            61 // provide number of lba28 addressable sectors
#define IDE_ATA_IDENTIFY_LBA48_SUPPORT      83
#define IDE_ATA_IDENTIFY_CAPABILITIES       98
#define IDE_ATA_IDENTIFY_LBA48_0            100
#define IDE_ATA_IDENTIFY_LBA48_1            101
#define IDE_ATA_IDENTIFY_LBA48_2            102
#define IDE_ATA_IDENTIFY_LBA48_3            103
#define IDE_ATA_IDENTIFY_FIELDVALID         106
#define IDE_ATA_IDENTIFY_MAX_LBA            120
#define IDE_ATA_IDENTIFY_COMMANDSETS        164
#define IDE_ATA_IDENTIFY_MAX_LBA_EXT        200

// interface type
#define IDE_ATA                             0x00
#define IDE_ATAPI                           0x01

// master or slave
#define IDE_ATA_MASTER                      0x00
#define IDE_ATA_SLAVE                       0x01

// channels
#define IDE_ATA_PRIMARY                     0x00
#define IDE_ATA_SECONDARY                   0x01

// directions
#define IDE_ATA_READ                        0x00
#define IDE_ATA_WRITE                       0x01

// channel registers
#define IDE_ATA_REG_DATA                    0x00

// BAR0
#define IDE_ATA_REG_ERROR                   0x01 // read only
#define IDE_ATA_REG_FEATURES                0x01 // write only
#define IDE_ATA_REG_SECCOUNT0               0x02
#define IDE_ATA_REG_LBA0                    0x03
#define IDE_ATA_REG_LBA1                    0x04
#define IDE_ATA_REG_LBA2                    0x05
#define IDE_ATA_REG_HDDEVSEL                0x06
#define IDE_ATA_REG_COMMAND                 0x07 // write only
#define IDE_ATA_REG_STATUS                  0x07 // read only
#define IDE_ATA_REG_SECCOUNT1               0x08
#define IDE_ATA_REG_LBA3                    0x09
#define IDE_ATA_REG_LBA4                    0x0A
#define IDE_ATA_REG_LBA5                    0x0B
#define IDE_ATA_REG_CONTROL                 0x0C
#define IDE_ATA_REG_ALTSTATUS               0x0C
#define IDE_ATA_REG_DEVADDRESS              0x0D


typedef struct IDE_ATA_partition_entry {
  uint8_t bootable;
  uint8_t start_head;
  uint16_t start_sec_cyl;
  uint8_t system_id;
  uint8_t end_head;
  uint16_t end_sec_cyl;
  uint32_t lba_start;
  uint32_t seccount;
} ide_ata_part_en_t;

typedef struct IDE_ATA_device {
  uint8_t present;
  uint8_t channel;
  uint8_t drive;
  uint8_t type;
  uint8_t device_type;
  uint8_t capabilities;
  uint32_t command_sets;
  uint32_t size;
  char model[41];
  uint16_t base;
  uint16_t control;
  uint16_t bmide;
  uint32_t nint;
  int is_slave;
  uint8_t addressing; //chs = 0x01, lba28 = 0x02, lba48 = 0x04
  uint64_t lba48_seccount;
  uint32_t lba28_seccount;
  ide_ata_part_en_t partition_table[4];
} ide_ata_device_t;

int ide_ata_init();

int open_node_ide_ata(vfs_gen_node_t * vfs_node, uint32_t flags);
int close_node_ide_ata(vfs_gen_node_t * vfs_node, uint32_t flags);
int write_node_ide_ata(vfs_gen_node_t * vfs_node, uint32_t start_at, uint32_t size, uint8_t * buffer);
int read_node_ide_ata(vfs_gen_node_t * vfs_node, uint32_t start_at, uint32_t size, uint8_t * buffer);
vfs_gen_node_t * read_directory_ide_ata(vfs_gen_node_t * vfs_node, int entry_no);
vfs_gen_node_t * find_in_directory_ide_ata(vfs_gen_node_t * vfs_node, char * name);
vfs_gen_node_t ** list_directory_ide_ata(vfs_gen_node_t * vfs_node);

void ide_ata_mount_devices();

#endif
