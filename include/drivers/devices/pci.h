#ifndef PCI_H
#define PCI_H
#include <stddef.h>
#include <stdint.h>
#include <kernel/util/linked_list.h>

#define PCI_CONFIG_PORT   0xCF8   //
#define PCI_DATA_PORT     0xCFC   //
#define PCI_FAIL          0xFFFF  //
#define PCI_MULTI_FUNC    0x80
// pci common header values
#define PCI_VEN_ID        0x00    //
#define PCI_DEV_ID        0x02    //
#define PCI_COMMAND       0x04    //
#define PCI_STATUS        0x06
#define PCI_REVISION_ID   0x08
#define PCI_PROG_IF       0x09    //
#define PCI_SUB_CLASS     0x0A    //
#define PCI_CLASS_CODE    0x0B    //
#define PCI_CACHE_LINE    0x0C    //
#define PCI_LATENCY_TIMER 0x0D    //
#define PCI_HEADER_TYPE   0x0E    //
#define PCI_BIST          0x0F    //

#define PCI_NO_OF_BUSES   256     //
#define PCI_FUN_PER_DEV   8       //
#define PCI_DEV_PER_BUS   32      //

// if header type == 0x00 ->
#define PCI_BAR0          0x10
#define PCI_BAR02         0x12
#define PCI_BAR1          0x14
#define PCI_BAR12         0x16
#define PCI_BAR2          0x18
#define PCI_BAR22         0x1A
#define PCI_BAR3          0x1C
#define PCI_BAR32         0x1E
#define PCI_BAR4          0x20
#define PCI_BAR42         0x22
#define PCI_BAR5          0x24
#define PCI_BAR52         0x26
#define PCI_CARDBUS_CIS   0x28
#define PCI_SUBSYS_VENDOR	0x2C
#define PCI_SUBSYS			  0x2E
#define PCI_EXPANSION_ROM 0x30
#define PCI_CAPABILITIES  0x34
#define PCI_IRQ_LINE      0x3C
#define PCI_IRQ_PIN       0x3D
#define PCI_MIN_GRANT     0x3E
#define PCI_MAX_LATENCY   0x3F

// if header type is 0x01 -> PCI->PCI bridge
// bar 0
// bar 0
#define PCI_PRIMARY_BUS   0x18
#define PCI_SECONDARY_BUS 0x19

#define PCI_BASE_CLASS_BRIDGE 0x06
#define PCI_SUB_CLASS_BRIDGE  0x04


typedef struct pci_device_std_header {
  uint32_t bar0;
  uint32_t bar1;
  uint32_t bar2;
  uint32_t bar3;
  uint32_t bar4;
  uint32_t bar5;
  uint32_t cardbus_cis;
  uint16_t subsystem_vendor_id;
  uint16_t subsystem_id;
  uint32_t exp_rom_base;
  uint16_t capabilities;
  uint8_t int_line;
  uint8_t int_pin;
  uint8_t min_grant;
  uint8_t max_latency;
} pci_dev_std_hdr_t;

typedef struct pci_device {
  uint8_t bus;
  uint8_t dev;

  uint16_t ven_id;
  uint16_t dev_id;
  uint8_t rev_id;
  uint8_t prog_if;
  uint8_t sub_class;
  uint8_t class_code;
  uint8_t cache_line_size;
  uint8_t latency_timer;
  uint8_t header_type;
  uint8_t bist;

  const char * ven_name;
  const char * ven_desc;

  const char * dev_name;
  const char * dev_desc;

  const char * base_class_desc;
  const char * sub_class_desc;
  const char * prog_if_desc;

  uint32_t * additional_info;
} pci_dev_t;


void pci_initialise();
lnk_lst_t * pci_get_devices(uint8_t class_code, uint8_t sub_class_code);
pci_dev_t * pci_get_device(uint16_t vendor_id, uint16_t device_id);


#endif
