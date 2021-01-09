#include <stddef.h>
#include <stdint.h>
// #include <kernel/vfs/vfs.h>
// #include <kernel/memory/memory_heap.h>
// #include <kernel/interrupts/interrupts.h>
// #include <kernel/util/kstdio.h>
// #include <kernel/util/string.h>
// #include <kernel/util/linked_list.h>
// #include <kernel/util/logging.h>
// #include <kernel/interrupts/pit.h>
// #include <drivers/devices/storage/ide.h>
// #include <drivers/devices/pci.h>

// ide ata devices -> max 4
// ide_ata_device_t * devices[4];
//
// // configures channels to use correct IO ports for IDE ATA communication
// int ide_ata_config_io();
//
// uint8_t ide_ata_read_register(ide_ata_device_t * device, uint8_t reg);
//
// void ide_ata_write_register(ide_ata_device_t * device, uint8_t reg, uint8_t data);
//
// void ide_ata_init_device(int d);
//
// uint8_t ide_ata_wait_busy(ide_ata_device_t * device);
//
// // void ide_ata_read_identify(ide_ata_device_t * device, uint32_t buf);
//
// void ide_ata_build_atapi(ide_ata_device_t * device);
//
// void ide_ata_build_ata(ide_ata_device_t * device);
//
// uint32_t ide_ata_get_device_size(ide_ata_device_t * device, uint8_t * buff);
//
// void ide_ata_set_device_model(ide_ata_device_t * device, uint8_t * buff);
//
// // returns status of 0 if all good, or 1 in case of error
// int ide_ata_read_disk_sector(ide_ata_device_t * device, uint32_t logical_block_address, uint8_t * buffer);
//
// int ide_ata_write_disk_sector(ide_ata_device_t * device, uint32_t logical_block_address, uint8_t * buffer);
//
// int ide_ata_access_disk_sector(ide_ata_device_t * device, uint32_t logical_block_address);
//
// void ide_ata_read_mbr(ide_ata_device_t * device);
//
// int ide_ata_check_mbr(ide_ata_device_t * device);
//
// vfs_gen_node_t * ide_ata_create_vfs_node(ide_ata_device_t * device, uint8_t type, char * name);
//
// void ide_ata_mount_node(vfs_gen_node_t * node);
//
// int open_node_ide_ata(vfs_gen_node_t * vfs_node, uint32_t flags){
//   return 0;
// }
//
// int close_node_ide_ata(vfs_gen_node_t * vfs_node, uint32_t flags){
//   kern_free((uint32_t *) vfs_node);
//   return 0;
// }
//
// int write_node_ide_ata(vfs_gen_node_t * vfs_node, uint32_t start_at, uint32_t size, uint8_t * buffer){
//   return 0;
// }
//
// int read_node_ide_ata(vfs_gen_node_t * vfs_node, uint32_t start_at, uint32_t size, uint8_t * buffer){
//   ide_ata_device_t * device = (ide_ata_device_t *) vfs_node->base_pointer;
//   if(device->type != IDE_ATA) { return -1; };
//   uint32_t start_sector = start_at / 512;
//   uint32_t end_sector = start_at + size / 512;
//
//   return 0;
// }
//
// vfs_gen_node_t * read_directory_ide_ata(vfs_gen_node_t * vfs_node, int entry_no){
//   return NULL;
// }
//
// vfs_gen_node_t * find_in_directory_ide_ata(vfs_gen_node_t * vfs_node, char * name){
//   return NULL;
// }
//
// vfs_gen_node_t ** list_directory_ide_ata(vfs_gen_node_t * vfs_node){
//   return NULL;
// }
//
// void ide_ata_mount_devices(){
//   for(int dev = 0; dev < 4; dev++){
//     if(devices[dev] != NULL && devices[dev]->present == 1){
//       if(devices[dev]->type == IDE_ATA){
//         vfs_gen_node_t * ata_node = ide_ata_create_vfs_node(devices[dev], IDE_ATA, strdup("hdd"));
//         ide_ata_mount_node(ata_node);
//       } else if(devices[dev]->type == IDE_ATAPI){
//         vfs_gen_node_t * atapi_node = ide_ata_create_vfs_node(devices[dev], IDE_ATAPI, strdup("cdrom"));
//         ide_ata_mount_node(atapi_node);
//       }
//     }
//   }
// }
//
// vfs_gen_node_t * ide_ata_create_vfs_node(ide_ata_device_t * device, uint8_t type, char * name){
//   vfs_gen_node_t * root_node = (vfs_gen_node_t *) kern_malloc(sizeof(vfs_gen_node_t));
//   root_node->node_no = 0;
//   root_node->name = name;
//   root_node->size = (uint32_t)(device->size) * 512;
//   root_node->type_flags = VFS_NODE_TYPE_BLKD;
//   root_node->specific_type = (type == IDE_ATA) ? IDE_ATA : IDE_ATAPI;
//
//   root_node->node_open = &open_node_ide_ata;
//   root_node->node_close = &close_node_ide_ata;
//   root_node->node_read = &read_node_ide_ata;
//   root_node->node_write = &write_node_ide_ata;
//   root_node->dir_read = &read_directory_ide_ata;
//   root_node->dir_find = &find_in_directory_ide_ata;
//   root_node->base_pointer = (uint32_t *) device;
//   return root_node;
// }
//
// void ide_ata_mount_node(vfs_gen_node_t * node){
//   char * path = (char *) kern_malloc(strlen("/dev/") + strlen(node->name));
//   ksprintf(path, "/dev/%s", node->name);
//   mount_in_vfs(node, path);
// }
//
// // configures ide devices using information taken from pci subsystem
// int ide_ata_config_io(){
//   lnk_lst_t * pci_devices_list = pci_get_devices(IDE_ATA_PCI_CLASS_CODE, IDE_ATA_PCI_SUBCLASS_CODE);
//
//   pci_dev_t * device = (pci_dev_t *) pci_devices_list->list_head->data;
//
//   pci_dev_std_hdr_t * std_hdr = (pci_dev_std_hdr_t *) device->additional_info;
//
//   uint16_t bar0 = (std_hdr->bar0 == 0x00 || std_hdr->bar0 == 0x01) ? IDE_ATA_BAR0 : 999;
//   if(bar0 == 999) return 1;
//
//   uint16_t bar1 = (std_hdr->bar1 == 0x00 || std_hdr->bar1 == 0x01) ? IDE_ATA_BAR1 : 999;
//   if(bar1 == 999) return 1;
//
//   uint16_t bar2 = (std_hdr->bar2 == 0x00 || std_hdr->bar2 == 0x01) ? IDE_ATA_BAR2 : 999;
//   if(bar2 == 999) return 1;
//
//   uint16_t bar3 = (std_hdr->bar3 == 0x00 || std_hdr->bar3 == 0x01) ? IDE_ATA_BAR3 : 999;
//   if(bar3 == 999) return 1;
//
//   uint16_t bar4 = std_hdr->bar4 & 0xFFFFFFFC;
//   if(bar4 != 0x00) return 1;
//
//   devices[0]->base = devices[1]->base = bar0;
//   devices[0]->control = devices[1]->control = bar1;
//   devices[0]->bmide = devices[1]->bmide = bar4;
//   devices[0]->is_slave = 0;
//   devices[1]->is_slave = 1;
//
//   devices[2]->base = devices[3]->base = bar2;
//   devices[2]->control = devices[3]->control = bar3;
//   devices[2]->bmide = devices[3]->bmide = bar4 + 8;
//   devices[2]->is_slave = 0;
//   devices[3]->is_slave = 1;
//
//   devices[0]->nint = 0x02;
//   devices[1]->nint = 0x02;
//   devices[2]->nint = 0x02;
//   devices[3]->nint = 0x02;
//
//   return 0;
// }
//
// uint8_t ide_ata_wait_busy(ide_ata_device_t * device){
//   uint8_t s = 0x00;
//   ide_ata_read_register(device, IDE_ATA_REG_STATUS);
//   ide_ata_read_register(device, IDE_ATA_REG_STATUS);
//   ide_ata_read_register(device, IDE_ATA_REG_STATUS);
//   ide_ata_read_register(device, IDE_ATA_REG_STATUS);
//   while(1){
//     s = ide_ata_read_register(device, IDE_ATA_REG_STATUS);
//     if(!(s & IDE_ATA_STATUS_BUSY)){
//       break;
//     }
//   };
//   return s;
// }
//
// int ide_ata_access_disk_sector(ide_ata_device_t * device, uint32_t logical_block_address){
//   int access_status = 0;
//   if(device->addressing & 0x04){
//       ide_ata_write_register(device, IDE_ATA_REG_HDDEVSEL, (device->is_slave) ? 0xF0 : 0xE0 );
//       ide_ata_write_register(device, IDE_ATA_REG_SECCOUNT1, 0);
//       ide_ata_write_register(device, IDE_ATA_REG_LBA3, (logical_block_address & 0xFF000000) >> 24);
//       ide_ata_write_register(device, IDE_ATA_REG_LBA4, 0);
//       ide_ata_write_register(device, IDE_ATA_REG_LBA5, 0);
//       ide_ata_write_register(device, IDE_ATA_REG_SECCOUNT0, 1);
//       ide_ata_write_register(device, IDE_ATA_REG_LBA0, (logical_block_address & 0x000000FF) >> 0);
//       ide_ata_write_register(device, IDE_ATA_REG_LBA1, (logical_block_address & 0x0000FF00) >> 8);
//       ide_ata_write_register(device, IDE_ATA_REG_LBA2, (logical_block_address & 0x00FF0000) >> 16);
//   } else if(device->addressing & 0x02){
//     if(logical_block_address > 0x0FFFFFFF){
//       access_status = 1; // no support for lba48 && lba28 cant address past 128GB
//     } else {
//       ide_ata_write_register(device, IDE_ATA_REG_HDDEVSEL, ((device->is_slave) ? 0xF0 : 0xE0 ) | ((logical_block_address >> 24) & 0x0F));
//       ide_ata_write_register(device, IDE_ATA_REG_SECCOUNT0, 1);
//       ide_ata_write_register(device, IDE_ATA_REG_LBA0, (logical_block_address & 0x000000ff) >>  0);
//       ide_ata_write_register(device, IDE_ATA_REG_LBA1, (logical_block_address & 0x0000ff00) >>  8);
//       ide_ata_write_register(device, IDE_ATA_REG_LBA2, (logical_block_address & 0x00ff0000) >> 16);
//     }
//   } else {
//     access_status = 1; // lba28 not supported at all -> too bad, no reads
//   }
//   return access_status;
// }
//
// int ide_ata_write_disk_sector(ide_ata_device_t * device, uint32_t logical_block_address, uint8_t * buffer){
//   int write_status = 0;
//   ide_ata_wait_busy(device);
//   // disable interrupts
//   ide_ata_write_register(device, IDE_ATA_REG_CONTROL, device->nint);
//
//   ide_ata_wait_busy(device);
//   // if device supports lba48
//   write_status = ide_ata_access_disk_sector(device, logical_block_address);
//   if(write_status != 1){
//     if(device->addressing & 0x04){
//       ide_ata_write_register(device, IDE_ATA_REG_COMMAND, IDE_ATA_COMMAND_WRITE_PIO_EXT);
//     } else {
//       ide_ata_write_register(device, IDE_ATA_REG_COMMAND, IDE_ATA_COMMAND_WRITE_PIO);
//     }
//     uint8_t status = ide_ata_wait_busy(device);
//     if(
//       (status & IDE_ATA_STATUS_DRIVE_READY) &&
//       (status & IDE_ATA_STATUS_DRIVE_SEEK_COMPLETE) &&
//       (status & IDE_ATA_STATUS_DATA_REQUEST_READY)
//     ) {
//       uint16_t * buf_big = (uint16_t *) buffer;
//       for(int i = 0; i < 256; i++){
//         write_port_16(device->base + IDE_ATA_REG_DATA, buf_big[i]);
//         ide_ata_write_register(device, IDE_ATA_REG_COMMAND, IDE_ATA_COMMAND_CACHE_FLUSH);
//       }
//       ide_ata_wait_busy(device);
//       write_status = 0;
//     }
//   }
//   return write_status;
// }
//
//
// int ide_ata_read_disk_sector(ide_ata_device_t * device, uint32_t logical_block_address, uint8_t * buffer){
//   int read_status = 0;
//   ide_ata_wait_busy(device);
//   ide_ata_write_register(device, IDE_ATA_REG_CONTROL, device->nint); // disable interrupts
//   ide_ata_wait_busy(device);
//   read_status = ide_ata_access_disk_sector(device, logical_block_address);
//   if(read_status != 1){
//     if(device->addressing & 0x04){
//       ide_ata_write_register(device, IDE_ATA_REG_COMMAND, IDE_ATA_COMMAND_READ_PIO_EXT);
//     } else {
//       ide_ata_write_register(device, IDE_ATA_REG_COMMAND, IDE_ATA_COMMAND_READ_PIO);
//     }
//     uint8_t status = ide_ata_wait_busy(device);
//     if(
//       !(status & IDE_ATA_STATUS_BUSY) &&
//       (status & IDE_ATA_STATUS_DATA_REQUEST_READY) &&
//       !(status & IDE_ATA_STATUS_ERROR) &&
//       !(status & IDE_ATA_STATUS_DRIVE_WRITE_FAULT)
//     ) {
//       read_port_16_mult(device->base + IDE_ATA_REG_DATA, buffer, 256);
//       ide_ata_wait_busy(device);
//       read_status = 0;
//     } else {
//       read_status = 1;
//     }
//   }
//   return read_status;
// }
//
// // returns 1 if mbr present, 0 if no mbr, -1 if error
// int ide_ata_check_mbr(ide_ata_device_t * device){
//   uint8_t * temp = (uint8_t *) kern_malloc(512);
//   int status;
//   if(ide_ata_read_disk_sector(device, 0, temp) != 0){
//     status = -1;
//   } else {
//     uint16_t mbr_signature = *((uint16_t *) (temp + 510));
//     if(mbr_signature == 0x55AA || mbr_signature == 0xAA55){
//       status = 1;
//     } else {
//       status = 0;
//     }
//   }
//   kern_free((uint32_t *)temp);
//   return status;
// }
//
// void ide_ata_irq_handler_primary(registers_t * regs){
//   ide_ata_wait_busy(devices[0]);
// }
//
// void ide_ata_irq_handler_secondary(registers_t * regs){
//   ide_ata_wait_busy(devices[0]);
// }
//
// // initialise ide_ata interface
// int ide_ata_init(){
//   for(int p = 0; p < 4; p++){
//     devices[p] = (ide_ata_device_t *) kern_malloc(sizeof(ide_ata_device_t));
//   }
//
//   if(!ide_ata_config_io()){
//     for(int i = 0; i < 4; i++){
//       ide_ata_init_device(i);
//     }
//   } else {
//     return -1;
//   }
//
//
//   install_interrupt_handler(IRQ14, &ide_ata_irq_handler_primary, DPL_KERNEL);
//   install_interrupt_handler(IRQ15, &ide_ata_irq_handler_secondary, DPL_KERNEL);
//
//   ide_ata_mount_devices();
//
//
//   // ide_ata_device_t * device = NULL;
//   // int sts;
//   // for(int d = 0; d < 4; d++){
//   //   if(devices[d]->type == IDE_ATA){
//   //     sts = ide_ata_check_mbr(devices[d]);
//   //     if(sts == 1){
//   //       device = devices[d];
//   //       break;
//   //     } else if(sts == -1){
//   //       return -1;
//   //     } else if(sts == 0){
//   //       return -2;
//   //     }
//   //   }
//   // }
//   //
//   // uint32_t lba_start = 0x00000000;
//   // // loop through partition entries
//   // for(int o = 0; o < 4; o++){
//   //   if(device->partition_table[o].bootable == IDE_ATA_PARTITION_BOOTABLE){
//   //     if(lba_start == 0){
//   //       lba_start = device->partition_table[o].lba_start;
//   //     }
//   //   }
//   // }
//   //
//   // uint8_t * test_buffer = (uint8_t *) kern_malloc(1024);
//   // memset_8(test_buffer, 0x00, 512);
//   // ide_ata_read_disk_sector(device, lba_start + 2, test_buffer);
//   // kprintf("MAGIC: %x %x",test_buffer[56], test_buffer[57]);
//   return 0;
// }
//
// void ide_ata_read_mbr(ide_ata_device_t * device){
//   uint8_t * mbr = (uint8_t *) kern_malloc(512);
//   ide_ata_read_disk_sector(device, 0, mbr);
//   uint16_t mbr_signature = *((uint16_t *) (mbr + 510));
//   if(mbr_signature == 0x55AA || mbr_signature == 0xAA55){
//     int x;
//     uint16_t i;
//     for(i = 0x1BE, x = 0; i < 0x1FE && x < 4; i+=0x10, x++){
//       device->partition_table[x].bootable = *((uint8_t *) (mbr + i));
//       device->partition_table[x].start_head = *((uint8_t *) (mbr + i + 1));
//       device->partition_table[x].start_sec_cyl = *((uint16_t *) (mbr + i + 2));
//       device->partition_table[x].system_id = *((uint8_t *) (mbr + i + 4));
//       device->partition_table[x].end_head = *((uint8_t *) (mbr + i + 5));
//       device->partition_table[x].end_sec_cyl = *((uint16_t *) (mbr + i + 6));
//       device->partition_table[x].lba_start = *((uint32_t *) (mbr + i + 8));
//       device->partition_table[x].seccount = *((uint32_t *) (mbr + i + 12));
//     }
//   }
//   kern_free((uint32_t *) mbr);
// }
//
// uint32_t ide_ata_get_device_size(ide_ata_device_t * device, uint8_t * buff){
//   device->addressing = (device->command_sets & (1 << 26)) ? 0x02 : 0x01;
//   return *( (uint32_t *) (buff + ( (device->addressing == 0x02) ? IDE_ATA_IDENTIFY_MAX_LBA_EXT : IDE_ATA_IDENTIFY_MAX_LBA) ) );
// }
//
// void ide_ata_set_device_model(ide_ata_device_t * device, uint8_t * buff){
//   // clear out our struct element
//   for(int s = 0; s < 40; s++){
//     device->model[s] = 0;
//   }
//   for(int i = 0; i < 40; i += 2){
//     device->model[i] = buff[IDE_ATA_IDENTIFY_MODEL + i + 1];
//     device->model[i + 1] = buff[IDE_ATA_IDENTIFY_MODEL + i];
//   }
//   device->model[39] = 0;
// }
//
// void ide_ata_build_ata(ide_ata_device_t * device){
//   // allocate 512 bytes -> 256 * 16 bit values
//   uint16_t * iden_buffer = (uint16_t *) kern_malloc(512);
//   read_port_16_mult(device->base + IDE_ATA_REG_DATA, (uint8_t *) iden_buffer, 256);
//   device->present = 1;
//   device->type = IDE_ATA;
//   device->device_type = *( (uint16_t *) iden_buffer + IDE_ATA_IDENTIFY_DEVICETYPE);
//   device->capabilities = *( (uint16_t *) iden_buffer + IDE_ATA_IDENTIFY_CAPABILITIES);
//   device->command_sets = *( (uint16_t *) iden_buffer + IDE_ATA_IDENTIFY_COMMANDSETS);
//   device->size = ide_ata_get_device_size(device, (uint8_t *) iden_buffer);
//   ide_ata_set_device_model(device, (uint8_t *) iden_buffer);
//
//   // determine addressing type
//   if(*( (uint16_t *) iden_buffer + IDE_ATA_IDENTIFY_LBA48_SUPPORT) & 0x400){
//     uint16_t lba48_0 = *( (uint16_t *) iden_buffer + IDE_ATA_IDENTIFY_LBA48_0);
//     uint16_t lba48_1 = *( (uint16_t *) iden_buffer + IDE_ATA_IDENTIFY_LBA48_1);
//     uint16_t lba48_2 = *( (uint16_t *) iden_buffer + IDE_ATA_IDENTIFY_LBA48_2);
//     uint16_t lba48_3 = *( (uint16_t *) iden_buffer + IDE_ATA_IDENTIFY_LBA48_3);
//     uint64_t lba48 = (uint64_t)((uint64_t) lba48_3 << 48) | ((uint64_t) lba48_2 << 32) | ((uint64_t) lba48_1 << 16) | (uint64_t) lba48_0;
//     if(lba48 != 0){
//       device->addressing = 0x04; // lba48 supported by device
//       device->lba48_seccount = lba48;
//     }
//   }
//
//   uint16_t lba28_0 = *( (uint16_t *) iden_buffer + IDE_ATA_IDENTIFY_LBA28_0);
//   uint16_t lba28_1 = *( (uint16_t *) iden_buffer + IDE_ATA_IDENTIFY_LBA28_1);
//   uint32_t lba28 = (uint32_t)(lba28_1 << 16) | (lba28_0);
//   if(lba28 != 0){
//     device->addressing |= 0x02;
//     device->lba28_seccount = lba28;
//   }
//   kern_free((uint32_t *) iden_buffer);
//   ide_ata_read_mbr(device);
// }
//
// void ide_ata_build_atapi(ide_ata_device_t * device){
//   // allocate 512 bytes -> 256 * 16 bit values
//   uint16_t * iden_buffer = (uint16_t *) kern_malloc(512);
//   read_port_16_mult(device->base + IDE_ATA_REG_DATA, (uint8_t *) iden_buffer, 256);
//   device->present = 1;
//   device->type = IDE_ATAPI;
//   device->device_type = *( (uint16_t *) iden_buffer + IDE_ATA_IDENTIFY_DEVICETYPE);
//   device->capabilities = *( (uint16_t *) iden_buffer + IDE_ATA_IDENTIFY_CAPABILITIES);
//   device->command_sets = *( (uint16_t *) iden_buffer + IDE_ATA_IDENTIFY_COMMANDSETS);
//   device->size = ide_ata_get_device_size(device, (uint8_t *) iden_buffer);
//   ide_ata_set_device_model(device, (uint8_t *) iden_buffer);
//
//   // determine addressing type
//   if(*( (uint16_t *) iden_buffer + IDE_ATA_IDENTIFY_LBA48_SUPPORT) & 0x400){
//     uint16_t lba48_0 = *( (uint16_t *) iden_buffer + IDE_ATA_IDENTIFY_LBA48_0);
//     uint16_t lba48_1 = *( (uint16_t *) iden_buffer + IDE_ATA_IDENTIFY_LBA48_1);
//     uint16_t lba48_2 = *( (uint16_t *) iden_buffer + IDE_ATA_IDENTIFY_LBA48_2);
//     uint16_t lba48_3 = *( (uint16_t *) iden_buffer + IDE_ATA_IDENTIFY_LBA48_3);
//     uint64_t lba48 = (uint64_t)((uint64_t) lba48_3 << 48) | ((uint64_t) lba48_2 << 32) | ((uint64_t) lba48_1 << 16) | (uint64_t) lba48_0;
//     if(lba48 != 0){
//       device->addressing = 0x04; // lba48 supported by device
//       device->lba48_seccount = lba48;
//     }
//   }
//
//   uint16_t lba28_0 = *( (uint16_t *) iden_buffer + IDE_ATA_IDENTIFY_LBA28_0);
//   uint16_t lba28_1 = *( (uint16_t *) iden_buffer + IDE_ATA_IDENTIFY_LBA28_1);
//   uint32_t lba28 = (uint32_t)(lba28_1 << 16) | (lba28_0);
//   if(lba28 != 0){
//     device->addressing |= 0x02;
//     device->lba28_seccount = lba28;
//   }
//   kern_free((uint32_t *) iden_buffer);
// }
//
// void ide_ata_init_device(int d){
//   ide_ata_device_t * device = devices[d];
//   // preread status -> floating bus detection
//   // if floating bus detected on that bus, just return, there is no device
//   if(ide_ata_read_register(device, IDE_ATA_REG_STATUS) != 0xFF){
//     ide_ata_wait_busy(device);
//     ide_ata_write_register(device, IDE_ATA_REG_HDDEVSEL, (device->is_slave) ? 0xB0 : 0xA0 );
//     ide_ata_write_register(device, IDE_ATA_REG_SECCOUNT0, 0);
//     ide_ata_write_register(device, IDE_ATA_REG_LBA0, 0);
//     ide_ata_write_register(device, IDE_ATA_REG_LBA1, 0);
//     ide_ata_write_register(device, IDE_ATA_REG_LBA2, 0);
//     ide_ata_wait_busy(device);
//     ide_ata_write_register(device, IDE_ATA_REG_COMMAND, IDE_ATA_COMMAND_IDENTIFY);
//     ide_ata_wait_busy(device);
//     if(ide_ata_read_register(device, IDE_ATA_REG_STATUS) != 0x00){
//       uint8_t lba1 = ide_ata_read_register(device, IDE_ATA_REG_LBA1);
//       uint8_t lba2 = ide_ata_read_register(device, IDE_ATA_REG_LBA2);
//
//       // ata = lba1 -> 0x00, lba2 -> 0x00, sata = lba1 -> 0x3C, lba2 -> 0xC3
//       if( ((lba1 == 0x00) && (lba2 == 0x00)) || ((lba1 == 0x3C) && (lba2 == 0xC3)) ){
//         ide_ata_build_ata(device);
//       } else if( ((lba1 == 0x14) && (lba2 == 0xEB)) || ((lba1 == 0x69) && (lba2 == 0x96)) ){
//         ide_ata_build_atapi(device);
//       }
//     }
//   }
//   return;
// }
//
// // read from device register
// uint8_t ide_ata_read_register(ide_ata_device_t * device, uint8_t reg){
//   uint8_t result;
//   if(reg > IDE_ATA_REG_STATUS && reg < IDE_ATA_REG_CONTROL){
//     ide_ata_write_register(device, IDE_ATA_REG_CONTROL, 0x80 | device->nint);
//   }
//   if(reg < 0x08){
//     result = read_port_8(device->base + reg - 0x00);
//   } else if(reg < 0x0C){
//     result = read_port_8(device->base + reg - 0x06);
//   } else if(reg < 0x0E){
//     result = read_port_8(device->control + reg - 0x0A);
//   } else if(reg < 0x16){
//     result = read_port_8(device->bmide + reg - 0x0E);
//   }
//   if(reg > IDE_ATA_REG_STATUS && reg < IDE_ATA_REG_CONTROL){
//     ide_ata_write_register(device, IDE_ATA_REG_CONTROL, device->nint);
//   }
//   return result;
// }
//
// // write to device register
// void ide_ata_write_register(ide_ata_device_t * device, uint8_t reg, uint8_t data){
//   if(reg > IDE_ATA_REG_STATUS && reg < IDE_ATA_REG_CONTROL){
//     ide_ata_write_register(device, IDE_ATA_REG_CONTROL, 0x80 | device->nint);
//   }
//
//   if(reg < 0x08){
//     write_port_8(device->base + reg - 0x00, data);
//   } else if(reg < 0x0C){
//     write_port_8(device->base + reg - 0x06, data);
//   } else if(reg < 0x0E){
//     write_port_8(device->control + reg - 0x0A, data);
//   } else if(reg < 0x16){
//     write_port_8(device->bmide + reg - 0x0E, data);
//   }
//
//   if(reg > IDE_ATA_REG_STATUS && reg < IDE_ATA_REG_CONTROL){
//     ide_ata_write_register(device, IDE_ATA_REG_CONTROL, device->nint);
//   }
// }
