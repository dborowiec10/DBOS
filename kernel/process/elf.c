#include <stddef.h>
#include <stdint.h>
#include <kernel/process/elf.h>
#include <kernel/util/kstdio.h>
#include <kernel/memory/memory_heap.h>
#include <kernel/memory/memory_paging.h>
#include <kernel/memory/memory_phys.h>
#include <kernel/util/logging.h>

int validate_elf(Elf32_Ehdr hdr);

// validates that the provided elf header is a valid header
// ready to be used to load the executable into the process image
int validate_elf(Elf32_Ehdr hdr){
  if(hdr.e_ident[EI_MAG0] != ELFMAG0){ goto fail; }
  if(hdr.e_ident[EI_MAG1] != ELFMAG1){ goto fail; }
  if(hdr.e_ident[EI_MAG2] != ELFMAG2){ goto fail; }
  if(hdr.e_ident[EI_MAG3] != ELFMAG3){ goto fail; }
  if(hdr.e_ident[EI_CLASS] != ELF_CLASS_32){ goto fail; }
  if(hdr.e_ident[EI_DATA] != ELF_DATA_LITTLE_ENDIAN){ goto fail; }
  if(hdr.e_ident[EI_VERSION] != ELF_CURRENT_VERSION){ goto fail; }
  if(hdr.e_machine != ELF_MACHINE_X86){ goto fail; }
  if(hdr.e_type != ET_REL && hdr.e_type != ET_EXEC){ goto fail; }
  return 0;
fail:
    return -1;
}

// loads elf executable into the process image
Elf32_Ehdr elf_load_executable(proc_t * process, vfs_gen_node_t * file){
  // reserve space for header
  Elf32_Ehdr e_hdr;
  read_from_vfs_node(file, 0, sizeof(Elf32_Ehdr), (uint8_t *) &e_hdr);
  // validate elf header
  if(validate_elf(e_hdr) == -1){
    return e_hdr;
  }
  // for every program header present in the elf file structure
  for(uint32_t addr = 0; addr < ((uint32_t) e_hdr.e_phentsize * e_hdr.e_phnum); addr += ((uint32_t) e_hdr.e_phentsize)){
    Elf32_Phdr p_hdr;
    // read in the header
    read_from_vfs_node(file, ((uint32_t) e_hdr.e_phoff) + ((uint32_t) addr), sizeof(Elf32_Phdr), (uint8_t *) &p_hdr);

    // ensure virtual address to be loaded is not 0 and the program header is loadable
    if(p_hdr.p_vaddr != 0x00000000 && p_hdr.p_type == PT_LOAD){
      // start and end of this section is dictated by the start address provided
      // by the header and a size of section in memory (this might be different from the size in file)
      uint32_t start = (uint32_t) p_hdr.p_vaddr;
      uint32_t end = (uint32_t) (p_hdr.p_vaddr + p_hdr.p_memsz);

      int size_in_pages = ((end - start) / 4096);
      if(((end - start) % 4096) != 0){
        size_in_pages++;
      }

      uint32_t physicals[size_in_pages];
      // for the number of pages required for this program header
      for(int i = 0; i < size_in_pages; i++){
        // allocate a physical address
        physicals[i] = alloc_phys_page();
        // clear it out
        memset_32((uint32_t *) temp_map_page(physicals[i]), 0x00000000, 1024);
        // map it to virtual address required by the header
        set_page_table_entry(process->proc_pdir, start + (i * 0x1000), physicals[i], 1, 1);
      }

      uint32_t file_offset = p_hdr.p_offset;
      uint32_t file_total_size = p_hdr.p_filesz;

      uint32_t file_start = file_offset;
      uint32_t file_end = file_start + file_total_size;

      uint32_t file_add_cur = file_start;

      int page_index = 0;
      // read in the data from the file into the allocated and mapped memory areas
      while(file_add_cur < file_end){
        // if size left to copy is higher than a page, load a page
        if((file_end - file_add_cur) > 0x1000){
          read_from_vfs_node(file, file_add_cur, 4096, (uint8_t *) temp_map_page(physicals[page_index]));
        } else {
          // read in the lefover data
          read_from_vfs_node(file, file_add_cur, file_end - file_add_cur, (uint8_t *) temp_map_page(physicals[page_index]));
        }

        page_index++;

        file_add_cur += 0x1000;

      }
    }
  }

  return e_hdr;
}
