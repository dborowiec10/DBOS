BITS 32

; *******************************multiboot_hdr***
; ***********************************************
MBOOT_P_ALIGN									equ 1 << 0
MBOOT_MEMINFO									equ 1 << 1
MBOOT_AOUT_K									equ 1 << 16
MBOOT_GFX											equ 1 << 2
MBOOT_MAGIC										equ 0x1BADB002
MBOOT_FLAGS										equ MBOOT_P_ALIGN | MBOOT_MEMINFO | MBOOT_GFX
MBOOT_CHECKSUM								equ -(MBOOT_MAGIC + MBOOT_FLAGS)
; ***********************************************
; ***********************************************


; ************initial_higher_half_hernel_pages***
; ***********************************************
;;; x86 memory map indicates that addresses from 0x80000 -> 0x90000 are guaranteed to be free to use
PAGE_DIRCTORY									equ	0x80000
PAGE_TABLE_KERNEL_HIGHER			equ 0x81000
KMAIN_PARAMS 									equ 0x82000
MAGIC_SAVE 										equ 0x82000
MBOOT_SAVE										equ 0x82004
MODS_SIZE											equ 0x82008
FIRST_MOD_START								equ 0x8200C
LAST_MOD_END									equ 0x82010
PDIR_SAVE											equ 0x82014
STACK_SAVE										equ 0x82018
STACK_BASE_SAVE								equ 0x8201C

PAGE_TABLES_MAPPED_IDENT			equ 0x82020
PAGE_DIR_ENTRIES_IDENT				equ 0x82024
PAGE_TABLE_IDENTITY_LOW				equ 0x83000

KERN_HIGHER_START							equ 0xC0001000
; ***********************************************
; ***********************************************


GLOBAL _init
GLOBAL _start_higher_half
GLOBAL _start
EXTERN _start
EXTERN _end
EXTERN bss


; ************************grub's header output***
; ***********************************************
SECTION .mboot
ALIGN 4
_mboot:
	dd MBOOT_MAGIC
	dd MBOOT_FLAGS
	dd MBOOT_CHECKSUM
	dd 0x00000000
	dd 0x00000000
	dd 0x00000000
	dd 0x00000000
	dd 0x00000000
	dd 0
	dd 0
	dd 0
; ***********************************************
; ***********************************************


; *********************************loader_code***
; ***********************************************
SECTION .init
_init:
	; save grub magic and multiboot info
	mov dword[MAGIC_SAVE], eax
	mov dword[MBOOT_SAVE], ebx

	; mov flags to eax
	mov eax, dword[ebx]

	; check if modules flag is set
	and eax, 0x4
	cmp eax, 0x4
	je .flag_present

.panic: ; no flag set, something definitely went wrong
 	hlt
	jmp .panic

.flag_present: ; we have a flag
	mov edx, dword[ebx + 24] 	; grab modules address
	mov ecx, dword[ebx + 20] 	; grab modules count
	cmp ecx, 0 ; check again if we have modules
	je no_modules

	mov ebx, 0
	push edx
	call last_mod_end ; count our modules
	mov dword[LAST_MOD_END], eax ; eax = last_mod_end
	pop edx
	mov edx, dword[edx] ; edx = first_mod_start
	mov dword[FIRST_MOD_START], edx
	sub eax, edx ; size of modules
	mov dword[MODS_SIZE], eax ; eax = mods size total
	;  modules counted here ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

	mov eax, dword[LAST_MOD_END]
	add eax, 0x00400000
	xor edx, edx ; clear edx
	mov ebx, 4096 ; divisor
	div ebx ; divide by page size
	mov ecx, eax
	mov dword[PAGE_TABLES_MAPPED_IDENT], ecx
	mov eax, PAGE_TABLE_IDENTITY_LOW
	mov ebx, 0x00000000
	or ebx, 3
	call map_tabs

	mov eax, dword[PAGE_TABLES_MAPPED_IDENT]
	xor edx, edx
	mov ebx, 1024
	div ebx
	add eax, 1
	mov dword[PAGE_DIR_ENTRIES_IDENT], eax
	jmp continue

	;; at this point we know how much space modules will take
	;; we can identity map space from 0 -> first_mod_start and space from last_mod_end + 3mb
no_modules:
	mov ecx, 1024
	mov eax, PAGE_TABLE_IDENTITY_LOW
	mov ebx, 0x00000000
	or ebx, 3
	call map_tabs

continue:
	mov ecx, dword[PAGE_DIR_ENTRIES_IDENT]
	mov eax, PAGE_DIRCTORY
	mov ebx, PAGE_TABLE_IDENTITY_LOW
	or ebx, 3
	call map_tabs

	mov ecx, 1024 ; prepare counter for inserting page table entries
	mov eax, PAGE_TABLE_KERNEL_HIGHER ; start inserting at
	mov ebx, 0x00100000 ; physical address to start mapping from
	or ebx, 3 ; set as present, read_write
	call map_tabs

	;;; at this point we have all page tables set, now we need to establish indexes in page directory as to where to insert them
	;;; first, lets establish low - we already know it starts at 0x00000000
	;;; we also know it probably won't be more than a single page table because of how grub loads modules 0x0 -> max 4MB

	mov eax, PAGE_TABLE_KERNEL_HIGHER ; map kernel page table as higher half
	or eax, 3
	mov dword[PAGE_DIRCTORY + 768*4], eax

	mov eax, PAGE_DIRCTORY ; self map page directory
	or eax, 3
	mov dword[PAGE_DIRCTORY + 1023*4], eax


	mov dword[PDIR_SAVE], PAGE_DIRCTORY
	mov eax, PAGE_DIRCTORY ; set cr3 to page dir address
	mov cr3, eax
	mov eax, cr0 ; set paging bit
	or eax, 0x80000000
	mov cr0, eax
	lea eax, [KERN_HIGHER_START] ; determine address (NO VALID GDT!)
	jmp eax ; long jump to higher kernel

last_mod_end: ; loop through modules and stop at last one - end address in eax
	mov eax, dword[edx + ebx + 4]
	add ebx, 16
	loop last_mod_end
	ret

map_tabs:
	mov dword[eax], ebx
	add eax, 4
	add ebx, 4096
	loop map_tabs
	ret

; **************************higher_half_kernel***
; ***********************************************
SECTION .text
EXTERN kmain
_start_higher_half:
    mov esp, stack_top ; setup stack
		mov dword[STACK_SAVE], esp
		mov dword[STACK_BASE_SAVE], ebp
		push KMAIN_PARAMS
    call kmain ; call C code

bad_boy: ; in case kmain returns, loop forever
		hlt
		jmp bad_boy

; ***********************************************
; ***********************************************

; **************allocateable_data - stack etc.***
; ***********************************************
SECTION	.bss
ALIGN	16
stack_bottom:
	resb 8192
stack_top:
; ***********************************************
; ***********************************************
