#include <stddef.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <scalls.h>

// controls a device/file specified
// op = operation
// arg = arguments (last argument should be NULL)
// returns status or value (device and op dependent)
int ioctl(int fd, unsigned long op, void * args){
  return _ioctl(fd, (uint32_t) op, (uint32_t *) args);
}
