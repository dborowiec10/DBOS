#include <stddef.h>
#include <stdint.h>
#include <kernel/util/linked_list.h>
#include <drivers/devices/pci.h>
#include <drivers/devices/pci_database_devices.h>
#include <kernel/util/kstdio.h>
#include <kernel/util/logging.h>
#include <kernel/memory/memory_heap.h>

uint16_t pci_read_base(uint8_t reg_no);
uint16_t pci_read(uint8_t bus_no, uint8_t device_no, uint8_t function_no, uint8_t reg_no);
void pci_write(uint8_t bus_no, uint8_t device_no, uint8_t function_no, uint8_t reg_no, int size, uint32_t data);
void pci_write_base(uint32_t base, int size, uint32_t data);
uint32_t pci_make_config_space(uint32_t bus_no, uint32_t device_no, uint32_t function_no, uint8_t reg_no);
void pci_build_device(uint8_t bus, uint8_t dev, uint16_t ven, uint8_t bc, uint8_t sc, uint8_t hdr);
void pci_scan_bus(uint8_t bus_no);
void pci_set_names(pci_dev_t * device);
void pci_set_header(pci_dev_t * pci_device);
void pci_set_standard_header(pci_dev_t * pci_device);

lnk_lst_t * pci_devices = NULL;

void pci_initialise(){
  // list for our devices
  pci_devices = linked_list_create();

  // lets first figure out the header type for the first [bus, dev, fun] triplet
  uint8_t header = (uint8_t) pci_read(0, 0, 0, PCI_HEADER_TYPE);

  // this will tell us if there is a single PCI host controller
  if((header & PCI_MULTI_FUNC) == 0){
    pci_scan_bus(0);
    return;
  }

  // loop over all functions of first device -> they should be pci host controllers
  for(int i=0; i < PCI_FUN_PER_DEV; i++){
    if(pci_read(0, 0, i, PCI_VEN_ID) != PCI_FAIL)
      break;
    pci_scan_bus(i);
  }
}

// retrieves pci device with the vendor id and device id or NULL if they are not present.
pci_dev_t * pci_get_device(uint16_t vendor_id, uint16_t device_id){
  pci_dev_t * temp = NULL;
  uint8_t found = 0x00;
  linked_list_each(device, pci_devices){
    temp = (pci_dev_t *) device->data;
    if(temp->ven_id == vendor_id && temp->dev_id == device_id){
      found = 0x01;
      break;
    }
  }
  if(!found){
    temp = NULL;
  }
  return temp;
}

// returns a linked list of devices which satisfy conditions:
// class code and sub class code
lnk_lst_t * pci_get_devices(uint8_t class_code, uint8_t sub_class_code){
  lnk_lst_t * temp = NULL;
  linked_list_each(device, pci_devices){
    pci_dev_t * dev = (pci_dev_t *) device->data;
    if(dev->class_code == class_code && dev->sub_class == sub_class_code){
      if(temp == NULL){
        temp = linked_list_create();
      }
      linked_list_append(temp, (uint32_t *) dev);
    }
  }
  return temp;
}

// scans a single pci bus to look for devices
void pci_scan_bus(uint8_t bus_no){
  for(uint8_t d = 0; d < PCI_DEV_PER_BUS; d++){
    uint16_t ven_id = (uint16_t) pci_read(bus_no, d, 0, PCI_VEN_ID);
    uint16_t dev_id = 0x0000;
    uint8_t base_class = 0x00;
    uint8_t sub_class = 0x00;
    uint8_t header_type = 0x00;
    if(ven_id != PCI_FAIL){
      base_class = (uint8_t) pci_read(bus_no, d, 0, PCI_CLASS_CODE);
      sub_class = (uint8_t) pci_read(bus_no, d, 0, PCI_SUB_CLASS);
      header_type = (uint8_t) pci_read(bus_no, d, 0, PCI_HEADER_TYPE);
      if(base_class == PCI_BASE_CLASS_BRIDGE && sub_class == PCI_SUB_CLASS_BRIDGE){
        pci_scan_bus((uint8_t) pci_read(bus_no, d, 0, PCI_SECONDARY_BUS));
      }
      if((header_type & PCI_MULTI_FUNC) != 0){
        for(int f = 0; f < PCI_FUN_PER_DEV; f++){
          if(pci_read(bus_no, d, f, PCI_VEN_ID) != PCI_FAIL){
            uint8_t bc = (uint8_t) pci_read(bus_no, d, f, PCI_CLASS_CODE);
            uint8_t sc = (uint8_t) pci_read(bus_no, d, f, PCI_SUB_CLASS);
            if(bc == PCI_BASE_CLASS_BRIDGE && sc == PCI_SUB_CLASS_BRIDGE){
              pci_scan_bus((uint8_t) pci_read(bus_no, d, f, PCI_SECONDARY_BUS));
            }
          }
        }
      }
      pci_build_device(bus_no, d, ven_id, base_class, sub_class, header_type);
    }
  }
}

// read device fields into a structure
void pci_build_device(uint8_t bus, uint8_t dev, uint16_t ven, uint8_t bc, uint8_t sc, uint8_t hdr){
  pci_dev_t * device = (pci_dev_t *) kern_malloc(sizeof(pci_dev_t));
  device->bus = bus;
  device->dev = dev;
  device->ven_id = ven;
  device->dev_id = (uint16_t) pci_read(bus, dev, 0, PCI_DEV_ID);
  device->rev_id = (uint8_t) pci_read(bus, dev, 0, PCI_REVISION_ID);
  device->prog_if = (uint8_t) pci_read(bus, dev, 0, PCI_PROG_IF);
  device->sub_class = sc;
  device->class_code = bc;
  device->cache_line_size = (uint8_t) pci_read(bus, dev, 0, PCI_CACHE_LINE);
  device->latency_timer = (uint8_t) pci_read(bus, dev, 0, PCI_LATENCY_TIMER);
  device->header_type = hdr;
  device->bist = (uint8_t) pci_read(bus, dev, 0, PCI_BIST);
  pci_set_names(device);
  pci_set_header(device);
  linked_list_append(pci_devices, (uint32_t *) device);
}

// checks the header and determines what additional information to read from the device
void pci_set_header(pci_dev_t * pci_device){
  uint8_t header = pci_device->header_type;
  switch (header) {
    case 0x00:
      pci_set_standard_header(pci_device);
      break;
    case 0x01:
      kprintf("Header type 0x01");
      break;
    case 0x02:
      kprintf("Header type 0x02");
      break;
    case 0x80:
      // multifunction device
      pci_set_standard_header(pci_device);
      break;
  }
}

// reads standard header data from the pci device
void pci_set_standard_header(pci_dev_t * pci_device){
  pci_dev_std_hdr_t * std_hdr = (pci_dev_std_hdr_t *) kern_malloc(sizeof(pci_dev_std_hdr_t));
  uint8_t bus = pci_device->bus;
  uint8_t dev = pci_device->dev;

  std_hdr->bar0 = pci_read(bus, dev, 0, PCI_BAR0);
  std_hdr->bar0 |= ((uint32_t) (pci_read(bus, dev, 0, PCI_BAR0 + 2) << 16));
  std_hdr->bar1 = pci_read(bus, dev, 0, PCI_BAR1);
  std_hdr->bar1 |= ((uint32_t) (pci_read(bus, dev, 0, PCI_BAR1 + 2) << 16));
  std_hdr->bar2 = pci_read(bus, dev, 0, PCI_BAR2);
  std_hdr->bar2 |= ((uint32_t) (pci_read(bus, dev, 0, PCI_BAR2 + 2) << 16));
  std_hdr->bar3 = pci_read(bus, dev, 0, PCI_BAR3);
  std_hdr->bar3 |= ((uint32_t) (pci_read(bus, dev, 0, PCI_BAR3 + 2) << 16));
  std_hdr->bar4 = pci_read(bus, dev, 0, PCI_BAR4);
  std_hdr->bar4 |= ((uint32_t) (pci_read(bus, dev, 0, PCI_BAR4 + 2) << 16));
  std_hdr->bar5 = pci_read(bus, dev, 0, PCI_BAR5);
  std_hdr->bar5 |= ((uint32_t) (pci_read(bus, dev, 0, PCI_BAR5 + 2) << 16));

  std_hdr->cardbus_cis = pci_read(bus, dev, 0, PCI_CARDBUS_CIS);
  std_hdr->cardbus_cis |= ((uint32_t) (pci_read(bus, dev, 0, PCI_CARDBUS_CIS + 2) << 16));
  std_hdr->subsystem_vendor_id = pci_read(bus, dev, 0, PCI_SUBSYS_VENDOR);
  std_hdr->subsystem_id = pci_read(bus, dev, 0, PCI_SUBSYS);
  std_hdr->exp_rom_base = pci_read(bus, dev, 0, PCI_EXPANSION_ROM);
  std_hdr->exp_rom_base |= ((uint32_t) (pci_read(bus, dev, 0, PCI_EXPANSION_ROM + 2) << 16));

  std_hdr->capabilities = pci_read(bus, dev, 0, PCI_CAPABILITIES);
  std_hdr->int_line = (uint8_t) pci_read(bus, dev, 0, PCI_IRQ_LINE);
  std_hdr->int_pin = (uint8_t) pci_read(bus, dev, 0, PCI_IRQ_PIN);
  std_hdr->min_grant = (uint8_t) pci_read(bus, dev, 0, PCI_MIN_GRANT);
  std_hdr->max_latency = (uint8_t) pci_read(bus, dev, 0, PCI_MAX_LATENCY);
  pci_device->additional_info = (uint32_t *) std_hdr;
}

// combines multiple attributes to create a single config space addr for device function
uint32_t pci_make_config_space(uint32_t bus_no, uint32_t device_no, uint32_t function_no, uint8_t reg_no){
  uint32_t config_space = 0x80000000; // 8 because we set the enable bit automatically
  config_space |= bus_no << 16;
  config_space |= device_no << 11;
  config_space |= function_no << 8;
  config_space |= (reg_no & 0xFC);
  return config_space;
}

// read from [bus_no] from [device_no] on [function_no] to [reg_no] of [size]
uint16_t pci_read(uint8_t bus_no, uint8_t device_no, uint8_t function_no, uint8_t reg_no){
  uint32_t temp;
  uint32_t config_space = pci_make_config_space((uint32_t)bus_no, (uint32_t)device_no, (uint32_t)function_no, reg_no);
  write_port_32(PCI_CONFIG_PORT, config_space);
  return pci_read_base(reg_no);
}

// read from base config space
uint16_t pci_read_base(uint8_t reg_no){
  uint32_t read = read_port_32(PCI_DATA_PORT);
  uint16_t temp = (uint16_t) ( (read >> ((reg_no & 2) * 8) ) & PCI_FAIL);
  return temp;
}

// write [data] of [size] on [bus_no] to [device_no -> function_no]'s [reg_no]
void pci_write(uint8_t bus_no, uint8_t device_no, uint8_t function_no, uint8_t reg_no, int size, uint32_t data){
  uint32_t temp;
  uint32_t config_space = pci_make_config_space((uint32_t)bus_no, (uint32_t)device_no, (uint32_t)function_no, reg_no);
  write_port_32(PCI_CONFIG_PORT, config_space);
  temp = PCI_DATA_PORT + (reg_no & 0x03);
  pci_write_base(temp, size, data);
}



// write to base config space
void pci_write_base(uint32_t base, int size, uint32_t data){
  if(size == 4){
    write_port_32(base, data);
  } else if(size == 2){
    write_port_16(base, (uint16_t) data);
  } else if(size == 1){
    write_port_8(base, (uint8_t) data);
  }
}

// lookup pci device string info from the huge table and set it on the specific device
void pci_set_names(pci_dev_t * device){

  for(int i=0; i < ((int) PCI_DEVTABLE_LEN); i++){
    if(PciDevTable[i].VenId == device->ven_id && PciDevTable[i].DevId == device->dev_id){
      device->dev_name = PciDevTable[i].Chip;
      device->dev_desc = PciDevTable[i].ChipDesc;
      break;
    }
  }
  for(int j=0; j < ((int) PCI_VENTABLE_LEN); j++){
    if(PciVenTable[j].VenId == device->ven_id){
      device->ven_name = PciVenTable[j].VenShort;
      device->ven_desc = PciVenTable[j].VenFull;
      break;
    }
  }
  for(int k=0; k < ((int) PCI_CLASSCODETABLE_LEN); k++){
    PCI_CLASSCODETABLE temp = PciClassCodeTable[k];
    if(temp.BaseClass == device->class_code && temp.SubClass == device->sub_class && temp.ProgIf == device->prog_if){
      device->base_class_desc = temp.BaseDesc;
      device->sub_class_desc = temp.SubDesc;
      device->prog_if_desc = temp.ProgDesc;
    }
  }
}
