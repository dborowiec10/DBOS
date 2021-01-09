#include <stdint.h>
#include <stddef.h>
#include <kernel/memory/memory_defs.h>
#include <kernel/memory/memory_phys.h>
#include <kernel/memory/memory_paging.h>
#include <kernel/memory/memory_heap.h>
#include <kernel/process/process.h>
#include <kernel/process/scheduler.h>
#include <kernel/interrupts/interrupts.h>
#include <kernel/interrupts/pit.h>
#include <kernel/mboot/multiboot.h>
#include <kernel/util/kstdio.h>
#include <kernel/util/logging.h>

// beginning of kernel
extern uint32_t _start;

// end of kernel
extern uint32_t _end;

// main page directory
page_dir_t * page_directory = NULL;

page_dir_t * kernel_pagedir = NULL;

int paging_status = 0;

uint32_t temp_page = 0;

uint32_t get_page_directory_address(){
  if(paging_status == 0){
    return (uint32_t) page_directory;
  } else {
    uint32_t pdir;
    __asm__ __volatile__("movl %%cr3, %%eax; \
                          movl %%eax, %0"
                         : "=r"(pdir)
                         :
                         :
    );
    return (uint32_t) pdir;
  }
}

// based on a virtual address, return index to a page directory
int get_page_directory_index(uint32_t virt_addr){
  uint32_t temp = virt_addr >> 22;
  return (int)(temp & LAST_PAGE_TABLE_INDEX);
}

// based on a virtual address, return index to a page table
int get_page_table_index(uint32_t virt_addr){
  uint32_t temp = virt_addr >> 12;
  return (int)(temp & LAST_PAGE_TABLE_INDEX);
}

// quickly map the address to the current address space to be able to read / write whilst updating
uint32_t temp_map_page(uint32_t physical){
  uint32_t temp_pte_addr = VIRT_PAGE_TAB_ADDR + ((((uint32_t)0xC0000000) >> 12) * 4);
  page_tab_en_t * temp = (page_tab_en_t *) temp_pte_addr;
  uint32_t temp_phys = physical;
  temp->bit_fields.page_addr = temp_phys >> 12;
  __asm__ __volatile__("invlpg %0"
                       :
                       : "m" (*(uint32_t *) 0xC0000000)
                       : "memory"
  );
  return (uint32_t) 0xC0000000;
}

// swap out the currently operational page directory
void swap_page_directory(uint32_t page_dir_addr){
  page_directory = (page_dir_t *) page_dir_addr;
  // by setting the new page directory to cr3
  __asm__ __volatile__("movl %0, %%cr3;"
                       :
                       : "r"(page_directory)
                       :
  );
}

// flush a specific tlb mapping based on an addr provided
void flush_tlb_specific(uint32_t addr){
  __asm__ __volatile__("invlpg %0"
                       :
                       : "m" (*(uint32_t *) addr)
                       : "memory"
  );
}

// flush the translation lookaside buffer by swapping in and out
// the address of page directory from the cr3 register
void flush_tlb(){
  __asm__ __volatile__("movl %%cr3, %%ebx;\
                        movl %%ebx, %%cr3"
                       :
                       :
                       : "ebx"
 );
}

// maps in kernel page tables onto the provided page directory
void map_kernel_tables(page_dir_t * pdir){
  memset_32((uint32_t *) temp_map_page((uint32_t) pdir), 0x00000000, 1024);

  for(int i = 768; i < 1024; i++){
    page_dir_t * tmp = kernel_pagedir;
    if(tmp->page_tables[i].val != 0x00000000 && tmp->page_tables[i].val != 0xFFFFFFFF){
      uint32_t val = tmp->page_tables[i].val;
      ((page_dir_t *) temp_map_page((uint32_t) pdir))->page_tables[i].val = val;
    }
  }

  page_dir_t * pd = (page_dir_t *) temp_map_page((uint32_t) pdir);

  pd->page_tables[1023].bit_fields.page_table_addr = ((uint32_t) pdir >> 12);
}


// clones page structures
page_dir_t * clone_page_structures(page_dir_t * src_dir){
  // allocate space for new page directory
  page_dir_t * new_page_dir = (page_dir_t *) alloc_phys_page();
  // clear it out
  memset_32((uint32_t *) temp_map_page((uint32_t) new_page_dir), 0x00000000, 1024);
  // loop through every page directory entry
  for(int i = 0; i < 1024; i++){
    // temporarily map in the source directory
    page_dir_t * sdir = (page_dir_t *) temp_map_page((uint32_t) src_dir);
    // if the entry is present and there is a valid non zero address in the entry
    if(sdir->page_tables[i].bit_fields.present_bit != 0x0){
      // if we are looking at a kernel page directory entry
      if(i >= 768){
        // extract the value
        page_dir_en_t entry = sdir->page_tables[i];
        // map in new directory
        page_dir_t * ndir = (page_dir_t *) temp_map_page((uint32_t) new_page_dir);
        // copy over the value
        ndir->page_tables[i] = entry;
        // if it is the last page directory entry
        if(i == 1023){
          // retrieve page table
          ndir->page_tables[i].bit_fields.page_table_addr = ((uint32_t) new_page_dir >> 12);
        }
      } else {
        // retrieve address of the src page table
        page_tab_t * src_page_table = (page_tab_t *) (sdir->page_tables[i].bit_fields.page_table_addr << 12);
        // fetch the entry
        page_dir_en_t src_entry = sdir->page_tables[i];
        // allocate space for new page table
        page_tab_t * new_page_table = (page_tab_t *) alloc_phys_page();
        // clear it out
        memset_32((uint32_t *) temp_map_page((uint32_t) new_page_table), 0x00000000, 1024);
        // for every entry in src table
        for(int j = 0; j < 1024; j++){
          page_tab_t * src_ptab = (page_tab_t *) temp_map_page((uint32_t) src_page_table);
          // if the entry is present
          if(src_ptab->page_entries[j].bit_fields.present_bit != 0x0){
            // make it readonly
            src_ptab->page_entries[j].bit_fields.read_write = 0;
            // fetch entry
            page_tab_en_t entry = src_ptab->page_entries[j];
            // fetch addr
            uint32_t addr = (uint32_t) (src_ptab->page_entries[j].bit_fields.page_addr << 12);
            // map new page table accessible
            page_tab_t * new_ptab = (page_tab_t *) temp_map_page((uint32_t) new_page_table);
            // copy over the entry
            new_ptab->page_entries[j] = entry;
            // increase reference count of the physical frame
            alloc_phys_increase_refcount(addr);
          }
        }
        // map in new directory
        page_dir_t * ndir = (page_dir_t *) temp_map_page((uint32_t) new_page_dir);
        // copy over the entry
        ndir->page_tables[i] = src_entry;
        // set page table address
        ndir->page_tables[i].bit_fields.page_table_addr = (uint32_t)((uint32_t) new_page_table >> 12);
      }
    }
  }
  // parent process needs to reupdate its caches to reflect the r/w being readonly
  flush_tlb();
  // return fresh new page directory
  return new_page_dir;
}

// removes address space of a process from the given page directory
// any address mappings below 0xC0000000 (higher kernel) will be removed
void remove_address_space(page_dir_t * page_dir){
  page_dir_t * pdir = NULL;
  // we loop only for the lower 3GB of virtual memory, kernel is merely mapped in
  // no need to free its pages
  for(int i = 0; i < 768; i++){
    // fetch page directory
    pdir = (page_dir_t *) temp_map_page((uint32_t) page_dir);
    // if entry has something in it
    if(pdir->page_tables[i].bit_fields.present_bit != 0x0){
      // fetch physical address of the page table
      uint32_t ptab_phys = (uint32_t)(pdir->page_tables[i].bit_fields.page_table_addr << 12);
      // we can now safely wipe the entry
      pdir->page_tables[i].val = 0x00000000;
      // temporarily map in the page table
      page_tab_t * ptab = (page_tab_t *) temp_map_page(ptab_phys);
      // for all entries in this page table
      for(int j = 0; j < 1024; j++){
        // if entry has something in it
        if(ptab->page_entries[j].bit_fields.present_bit != 0x0){
          // fetch physical frame address
          uint32_t phys_frame_addr = (uint32_t)(ptab->page_entries[j].bit_fields.page_addr << 12);
          // get its reference count
          int refcount = alloc_phys_get_refcount(phys_frame_addr);
          if(refcount == 1){
            // if only this page table points to this physical frame
            alloc_phys_free(phys_frame_addr);
          } else {
            // if some other page tables or even processes point to this physical frame
            // simply decrease its reference count
            alloc_phys_decrease_refcount(phys_frame_addr);
          }
          // wipe entry
          ptab->page_entries[j].val = 0x00000000;
        }
      }
      // free physical frame allocated for the page table
      alloc_phys_free(ptab_phys);
    }
  }
  flush_tlb();
}

// destroys page directory and physical frames associated with the memory
// if their reference counts have reached a specific threshold
void nuke_page_directory(page_dir_t * page_dir){
  page_dir_t * pdir = NULL;
  // we loop only for the lower 3GB of virtual memory, kernel is merely mapped in
  // no need to free its pages
  for(int i = 0; i < 768; i++){
    // fetch page directory
    pdir = (page_dir_t *) temp_map_page((uint32_t) page_dir);
    // if entry has something in it
    if(pdir->page_tables[i].bit_fields.present_bit != 0x0){
      // fetch physical address of the page table
      uint32_t ptab_phys = (uint32_t)(pdir->page_tables[i].bit_fields.page_table_addr << 12);
      // temporarily map in the page table
      page_tab_t * ptab = (page_tab_t *) temp_map_page(ptab_phys);
      // for all entries in this page table
      for(int j = 0; j < 1024; j++){
        // if entry has something in it
        if(ptab->page_entries[j].bit_fields.present_bit != 0x0){
          // fetch physical frame address
          uint32_t phys_frame_addr = (uint32_t)(ptab->page_entries[j].bit_fields.page_addr << 12);
          // get its reference count
          int refcount = alloc_phys_get_refcount(phys_frame_addr);
          if(refcount == 1){
            // if only this page table points to this physical frame
            alloc_phys_free(phys_frame_addr);
          } else {
            // if some other page tables or even processes point to this physical frame
            // simply decrease its reference count
            alloc_phys_decrease_refcount(phys_frame_addr);
          }
        }
      }
      // free physical frame allocated for the page table
      alloc_phys_free(ptab_phys);
    }
  }
  // free physical frame allocated for the page directory itself
  alloc_phys_free((uint32_t) page_dir);
  flush_tlb();
}


// initialise paging and map initial memory areas
void init_paging(uint32_t phys_e_addr){
  // set up some structures
  page_dir_t * page_dir;

  // allocate physical address for page_directory
  page_directory = (page_dir_t *) alloc_phys_page();

  // temporarily map page directory to an usable address space
  page_dir = (page_dir_t *) temp_map_page((uint32_t) page_directory);

  // clear out the page directory
  memset_8((void *) page_dir, 0x00, (int) sizeof(page_dir_t));

  // loop through the page dir and make everything non present
  page_dir_en_t * pde;
  int i = 0;
  for(; i < NO_OF_TABLE_ENTRIES; i++){
    pde = &(page_dir->page_tables[i]);
    pde->bit_fields.present_bit = 0x0;
    pde->bit_fields.read_write = 0x0;
  }

  // then self-map page directory
  pde = &(page_dir->page_tables[(int)LAST_PAGE_TABLE_INDEX]);
  pde->bit_fields.present_bit = 0x1;
  pde->bit_fields.read_write = 0x1;
  pde->bit_fields.page_table_addr = ((uint32_t) page_directory) >> 12;

  // beginnings and ends...
  uint32_t virt_s_addr_k = (uint32_t) &_start;
  uint32_t virt_e_addr_k = (uint32_t)( physical_to_virtual(phys_e_addr) );
  uint32_t phys_s_addr_k = virtual_to_physical(virt_s_addr_k);
  uint32_t phys_e_addr_k = virtual_to_physical(virt_e_addr_k);
  uint32_t addr_p;
  uint32_t addr_v;

  // Map the kernel at 3GB
  for(addr_v = virt_s_addr_k, addr_p = phys_s_addr_k;
      (addr_v < virt_e_addr_k) && (addr_p < phys_e_addr_k);
      addr_v += PAGE_SIZE, addr_p += PAGE_SIZE){
    set_page_table_entry(page_directory, addr_v, addr_p, 1, 0);
  }

  // set_page_table_entry(page_directory, (uint32_t) page_dir, 0x00000000, 1, 0);
  set_page_table_entry(page_directory, (uint32_t) 0xC0000000, 0x00000000, 1, 0);

  // install page_fault_handler
  install_interrupt_handler(ISR14, (isr_handler_t) page_fault_handler, DPL_KERNEL);

  kernel_pagedir = (page_dir_t *) 0xFFFFF000;
}

// resets page directory in cr3
void reset_paging(){
  swap_page_directory((uint32_t) page_directory);
  temp_page = alloc_phys_page();
  set_page_table_entry(page_directory, (uint32_t) 0xCFFFD000, temp_page, 1, 0);
  temp_page = (uint32_t) 0xCFFFD000;
  paging_status = 1;
}

// retrieves mapped physical address of virt_addr from directory
uint32_t get_mapped_address(page_dir_t * directory, uint32_t virt_addr){
  uint32_t address = 0x00000000;
  // align address to page size
  virt_addr = align_page(virt_addr);
  // temporarily map in the directory
  page_dir_t * page_dir = (page_dir_t *) temp_map_page((uint32_t) directory);
  // retrieve page directory index for this virtual address
  int PDE_index = get_page_directory_index(virt_addr);
  // get page directory entry
  page_dir_en_t * page_dir_entry = (page_dir_en_t *) &page_dir->page_tables[PDE_index];
  // get the page table
  page_tab_t * page_tab = (page_tab_t *)(page_dir_entry->bit_fields.page_table_addr << 12);
  // if it is not null
  if(page_tab != NULL){
    // temporarily map it
    page_tab = (page_tab_t *) temp_map_page((uint32_t) page_tab);
    // calculate page table index for this virtual address
    int PTE_index = get_page_table_index(virt_addr);
    // retrieve entry for this address
    page_tab_en_t * page_tab_entry = (page_tab_en_t *) &page_tab->page_entries[PTE_index];
    // retrieve mapping
    address = (uint32_t)(page_tab_entry->bit_fields.page_addr << 12);
  }
  return address;
}

// clears out any mapping in the given page directory for the virt_addr
void unset_page_table_entry(page_dir_t * directory, uint32_t virt_addr){
  // align address to page size
  virt_addr = align_page(virt_addr);
  // temporarily map in the directory
  page_dir_t * page_dir = (page_dir_t *) temp_map_page((uint32_t) directory);
  // retrieve page directory index for this virtual address
  int PDE_index = get_page_directory_index(virt_addr);
  // get page directory entry
  page_dir_en_t * page_dir_entry = (page_dir_en_t *) &page_dir->page_tables[PDE_index];
  // get the page table
  page_tab_t * page_tab = (page_tab_t *)(page_dir_entry->bit_fields.page_table_addr << 12);
  // if it is not null
  if(page_tab != NULL){
    // temporarily map it
    page_tab = (page_tab_t *) temp_map_page((uint32_t) page_tab);
    // calculate page table index for this virtual address
    int PTE_index = get_page_table_index(virt_addr);
    // retrieve entry for this address
    page_tab_en_t * page_tab_entry = (page_tab_en_t *) &page_tab->page_entries[PTE_index];
    // zero it out
    page_tab_entry->val = 0x00000000;
  }
}

// set an entry in a page table
// by looking at the virtual address, this function determines the index
// of both page directory and page table and then sets this entry to point to phys_addr
// also marks the entry as present
void set_page_table_entry(page_dir_t * directory, uint32_t virt_addr, uint32_t phys_addr, int present, int user_supervisor){
  // align addresses to page boundaries
  virt_addr = align_page(virt_addr);
  phys_addr = align_page(phys_addr);
  // declare some structures
  page_dir_t * page_dir;
  page_dir_en_t * page_dir_entry;
  page_tab_t * page_tab;
  page_tab_en_t * page_tab_entry;
  // map PD into usable address space
  page_dir = (page_dir_t *) temp_map_page((uint32_t) directory);
  // retrieve PDE
  int PDE_index = get_page_directory_index(virt_addr);
  page_dir_entry = (page_dir_en_t *) &page_dir->page_tables[PDE_index];
  // retrieve PT
  page_tab = (page_tab_t *)(page_dir_entry->bit_fields.page_table_addr << 12);
  // if PT non_present -> create it
  if(page_tab == 0x0){
    // allocate a physical page for the page table
    page_tab = (page_tab_t *) alloc_phys_page();
    // clear PDE
    memset_8((void *) page_dir_entry, 0x0, (int) sizeof(page_dir_en_t));
    // make PDE valid
    page_dir_entry->bit_fields.present_bit = 0x1;
    page_dir_entry->bit_fields.read_write = 0x1;
    page_dir_entry->bit_fields.user_supervisor = user_supervisor;
    page_dir_entry->bit_fields.page_table_addr = (uint32_t)page_tab >> 12;
    // set all addresses of this PT to be non_present
    int i;
    memset_32((uint32_t *) temp_map_page((uint32_t) page_tab), 0x00000000, 1024);
  }
  // map PT into usable address space
  page_tab = (page_tab_t *) temp_map_page((uint32_t) page_tab);
  // retrieve PTE
  int PTE_index = get_page_table_index(virt_addr);
  page_tab_entry = (page_tab_en_t *) &page_tab->page_entries[PTE_index];
  // clear PTE
  memset_8((void *) page_tab_entry, 0x0, (int) sizeof(page_tab_en_t));
  // mark PTE as present and map our physical address to virtual
  page_tab_entry->bit_fields.present_bit = present;
  page_tab_entry->bit_fields.read_write = 0x1;
  page_tab_entry->bit_fields.user_supervisor = user_supervisor;
  page_tab_entry->bit_fields.page_addr = phys_addr >> 12;
}

// dumps regs and stack out in case of a page fault
registers_t * page_fault_handler(registers_t * regs){
  // fetch current process
  proc_t * current = proc_get_current_proc();
  // update its registers
  current->proc_regs = regs;
  // collect faulting address
  uint32_t page_fault_addr;
  __asm__ __volatile__("mov %%cr2, %0" : "=r" (page_fault_addr));
  // page fault caused by a Ring3 process attempting to write to an existing page
  // with readonly flag
  if( (regs->err_code & 0x4) && (regs->err_code & 0x2) && (regs->err_code & 0x1) ){
    // fetch mapped address in
    uint32_t frame = get_mapped_address(current->proc_pdir, page_fault_addr);
    // collect reference count of that frame to see how many processes have it mapped in
    int frame_refcount = alloc_phys_get_refcount(frame);
    // user level current
    if(frame_refcount > 1){
      // decrease reference count for the frame, one less pdir pointing to it
      alloc_phys_decrease_refcount(frame);
      // allocate a fresh page for the frame
      uint32_t new_frame = alloc_phys_page();
      // temporarily map the old frame
      uint32_t frame_v = temp_map_page(frame);
      // copy contents over to a temporary work frame
      memcpy_32((uint32_t *) temp_page, (uint32_t *) frame_v, 1024);
      // map in new frame
      uint32_t new_frame_v = temp_map_page(new_frame);
      // copy over contents from the temporary one to the new one
      memcpy_32((uint32_t *) new_frame_v, (uint32_t *) temp_page, 1024);
      // ensure page aligned address of the fault has a fresh frame with contents mapped in
      // set_page_table_entry will make it r/w rather than readonly
      set_page_table_entry(current->proc_pdir, page_fault_addr, new_frame, 1, 1);
    } else if(frame_refcount == 1){
      // reference count of faulting frame is 1, it is allocated and mapped only in this
      // page directory, we can safely assume we are the rightful owner of it, simply map it again
      // this will force it to be r/w and will fix the fault
      set_page_table_entry(current->proc_pdir, page_fault_addr, frame, 1, 1);
    }
    // return process kernel stack, rest is handled by assembly handler just like in a
    // regular context switch
    return current->proc_regs;

  } else if(regs->err_code & 0x4){
    // something happened and process violated memory protection in other
    // way than copy-on-write, lets kill it
    // make the process dead
    current->state = PROCESS_STATE_DEAD;
    log("Process caused a page fault!");
    // call scheduler handler, this will put this process
    // onto the dead queue and set some other ready process as the current one
    // it will also return fresh set of registers to return with
    return sched_preempt_handler(current->proc_regs);
  }

  // this was most likely caused by kernel, something bad happened
  kprintf(
    "\n!!!PAGE FAULT!!!\
     \n - ERR CODE: %x \
     \n - EAX: %x, EBX: %x, ECX: %x, EDX: %x \
     \n - Stack Pointer: %x \
     \n - Instruction: %x \
     \n - FAULT ADDR: %x \
     \n ---- page %s present! \
     \n ---- %s caused the fault! \
     \n ---- CPU is in %s mode! \
     %s %s",
    regs->err_code,
    regs->eax, regs->ebx, regs->ecx, regs->edx, regs->esp, regs->eip, page_fault_addr,
    (regs->err_code & 0x1) ? "was" : "was not",
    (regs->err_code & 0x2) ? "write" : "read",
    (regs->err_code & 0x4) ? "user" : "kernel",
    (regs->err_code & 0x8) ? "Reserved bits overwritten, page corrupt!" : "",
    (regs->err_code & 0x10) ? "Tried to fetch an instruction!" : ""
  );

  // diable interrupts
  disable_interrupts();
  // loop forever
  while(1){
    __asm__ __volatile__("hlt");
    __asm__ __volatile__("cli");
  }
  // this is just so the compiler does not complain
  return NULL;
}
