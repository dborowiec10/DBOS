#ifndef TARFS_H
#define TARFS_H

#include <stddef.h>
#include <stdint.h>
#include <kernel/vfs/vfs.h>

#define REGTYPE  '0'
#define AREGTYPE '\0'
#define LNKTYPE  '1'
#define SYMTYPE  '2'
#define CHRTYPE  '3'
#define BLKTYPE  '4'
#define DIRTYPE  '5'
#define FIFOTYPE '6'
#define CONTTYPE '7'

#define TUREAD   00400
#define TUWRITE  00200
#define TUEXEC   00100
#define TGREAD   00040
#define TGWRITE  00020
#define TGEXEC   00010
#define TOREAD   00004
#define TOWRITE  00002
#define TOEXEC   00001

#define TARFS_HDR_SIZE 512

typedef struct tarfs_file_header {
  char name[100];
  char mode[8];
  char uid[8];
  char gid[8];
  char size[12];
  char mtime[12];
  char chksum[8];
  char typeflag;
  char linkname[100];
  char magic[6];
  char version[2];
  char uname[32];
  char gname[32];
  char devmajor[8];
  char devminor[8];
  char prefix[155];
} tarfs_hdr_t;

typedef struct tarfs_file {
  tarfs_hdr_t * header;
  uint8_t * file_contents;
} tarfs_file_t;

typedef int (* finder_t)(tarfs_hdr_t *, void *, int);

vfs_gen_node_t * create_tarfs_filesystem(char * device, char * mountpoint);

int open_node_tarfs(vfs_gen_node_t * vfs_node, uint32_t flags);

int close_node_tarfs(vfs_gen_node_t * vfs_node, uint32_t flags);

int write_node_tarfs(vfs_gen_node_t * vfs_node, uint32_t start_at, uint32_t size, uint8_t * buffer);

int read_node_tarfs(vfs_gen_node_t * vfs_node, uint32_t start_at, uint32_t size, uint8_t * buffer);

vfs_gen_node_t * read_directory_tarfs(vfs_gen_node_t * vfs_node, int entry_no);

vfs_gen_node_t * find_in_directory_tarfs(vfs_gen_node_t * vfs_node, char * path);

vfs_gen_node_t ** list_directory_tarfs(vfs_gen_node_t * vfs_node);

#endif
