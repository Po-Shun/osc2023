#ifndef __CPIO_H__
#define __CPIO_H__
#include "fdt.h"

#define RECV_LEN 100
#define USER_STACK_TOP 0x40000

struct cpio_newc_header {
		   char	   c_magic[6];
		   char	   c_ino[8];
		   char	   c_mode[8];
		   char	   c_uid[8];
		   char	   c_gid[8];
		   char	   c_nlink[8];
		   char	   c_mtime[8];
		   char	   c_filesize[8];
		   char	   c_devmajor[8];
		   char	   c_devminor[8];
		   char	   c_rdevmajor[8];
		   char	   c_rdevminor[8];
		   char	   c_namesize[8];
		   char	   c_check[8];
	   };

unsigned int hexstr_to_uint(char *s, unsigned int len);
void ls_cmd();
void exec_cmd();
void cat_cmd();

#endif