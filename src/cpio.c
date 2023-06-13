#include "cpio.h"
#include "string.h"


static char buf[RECV_LEN] = {0};
extern void from_el1_to_el0(unsigned int instr_addr, unsigned int stack_addr);

// unsigned int _8byte_hexadecimal_to_int(char* msb){
//   unsigned int ret = 0;
//   for(unsigned int i = 0; i < 8; i++){
//     unsigned int temp = *(msb + i);
//     if(temp >= 'a'){
//       temp = temp - 'a' + 10;
//     }
//     else{
//       temp -= '0';
//     }
//     ret *= 16;
//     ret += temp;
//   }
//   return ret;
// }

unsigned int hexstr_to_uint(char *s, unsigned int len) {

    unsigned int n = 0;

    for (int i=0; i<len; i++) {
        n *= 16;
        if (s[i] >= '0' && s[i] <= '9') {
            n += s[i] - '0';
        } else if (s[i] >= 'A' && s[i] <= 'F') {
            n += s[i] - 'A' + 10;
        }
    }

    return n;

}

void ls_cmd(){

  struct cpio_newc_header *pos = (struct cpio_noewc_header*) CPIO_BASE;
  char* current = (char*) CPIO_BASE;
  int filesize = 0;
  while(1){
    pos = (struct cpio_newc_header*)current;
    int namesize = hexstr_to_uint(pos->c_namesize, 8);
    filesize = hexstr_to_uint(pos->c_filesize, 8);
    current += sizeof(struct cpio_newc_header);
    printf("%s\n", current);
    if(strcmp(current, "TRAILER!!!") == 0) {
      uart_puts("\r");
      uart_puts("File Not Found\n\r");
      break;
    }
    int load_flag = 0;
    // if(strcmp(current, name) == 0){
    //   load_flag = 1;
    // }
    current += namesize;
    int total_size = current - (char*) pos;
    int reminder = total_size % 4;
    if(reminder != 0){
      current  += (4 - reminder);
    }
    if(load_flag == 1){
      break;
    }
    current += filesize;
    total_size = current - (char*) pos;
    reminder = total_size % 4;
    if(reminder != 0){
      current += (4 - reminder);
    }
  }
  // struct cpio_newc_header *header = (struct cpio_newc_header*)CPIO_BASE;
  // unsigned int filesize = 0;
  // unsigned int namesize = 0;
  // unsigned int offset;
  // char* filename;
  // while(1){
  //   filename = ((void*)header) + sizeof(struct cpio_newc_header);
  //   uart_puts(filename);
  //   uart_puts("\r\n");
  //   if(strcmp(filename, "TRAILER!!!") == 0) break;
    
  //   namesize =  hexstr_to_uint(header->c_namesize, 8);
  //   filesize =  hexstr_to_uint(header->c_filesize, 8);
  //   printf("%x, %x\n", namesize, filesize);
  //   offset = sizeof(struct cpio_newc_header) + namesize;
  //   if (offset % 4 != 0) 
  //           offset = ((offset/4) + 1) * 4;
        
  //   if (filesize % 4 != 0)
  //       filesize = ((filesize/4) + 1) * 4;

  //       offset = offset + filesize;

  //       header = ((void*)header) + offset;
  // }
  // while(1){
  //   pos = (struct cpio_newc_header*)current;
  //   unsigned long namesize = _8byte_hexadecimal_to_int(pos->c_namesize);
  //   unsigned long filesize = _8byte_hexadecimal_to_int(pos->c_filesize);
  //   current += sizeof(struct cpio_newc_header);
  //   printf("%d, %d \n", namesize, filesize);
  //   uart_puts(current);
  //   uart_puts("\r\n");
  //   // if(strcmp(current, "TRAILER!!!") == 0) break;
    
  //   // total size of the fixed header plus pathname is a multiple	of four
  //   current += namesize;
  //   unsigned int total_size = current - (char*) pos;
  //   unsigned int reminder = total_size % 4;
  //   if(reminder != 0){
  //     current  += (4 - reminder);
  //   }
  //   current += filesize;
  //   total_size = current - (char*) pos;
  //   reminder = total_size % 4;
  //   if(reminder != 0){
  //     current += (4 - reminder);
  //   }
  // }
  printf("END\n");
}

// void cat_cmd(){
//   uart_puts("Filename: ");
//   uart_getline(buf, RECV_LEN);
//   struct cpio_newc_header *pos = (struct cpio_newc_header*)CPIO_BASE;
//   char *current = (char*) CPIO_BASE;
//   while(1){
//     pos = (struct cpio_newc_header*)current;
//     int namesize = _8byte_hexadecimal_to_int(pos->c_namesize);
//     int filesize = _8byte_hexadecimal_to_int(pos->c_filesize);
//     current += sizeof(struct cpio_newc_header);
//     if(strcmp(current, "TRAILER!!!") == 0) {
//       uart_puts("\r");
//       uart_puts("File Not Found\n\r");
//       break;
//     }
//     int output_flag = 0;
//     if(strcmp(current, buf) == 0){
//       output_flag = 1;
//     }
//     current += namesize;
//     int total_size = current - (char*) pos;
//     int reminder = total_size % 4;
//     if(reminder != 0){
//       current  += (4 - reminder);
//     }
//     if(output_flag == 1){
//       uart_puts("\r");
//       uart_puts(current);
//       break;
//     }
//     current += filesize;
//     total_size = current - (char*) pos;
//     reminder = total_size % 4;
//     if(reminder != 0){
//       current += (4 - reminder);
//     }
//   }
// }

// void exec_cmd(){
//   uart_puts("Filename: ");
//   uart_getline(buf, RECV_LEN);
//   struct cpio_newc_header *pos = (struct cpio_newc_header*)CPIO_BASE;
//   char *current = (char*) CPIO_BASE;

//   while(1){
//     pos = (struct cpio_newc_header*)current;
//     int namesize = _8byte_hexadecimal_to_int(pos->c_namesize);
//     int filesize = _8byte_hexadecimal_to_int(pos->c_filesize);
//     current += sizeof(struct cpio_newc_header);
//     if(strcmp(current, "TRAILER!!!") == 0) {
//       uart_puts("\r");
//       uart_puts("File Not Found\n\r");
//       break;
//     }
//     int load_flag = 0;
//     if(strcmp(current, buf) == 0){
//       load_flag = 1;
//     }
//     current += namesize;
//     int total_size = current - (char*) pos;
//     int reminder = total_size % 4;
//     if(reminder != 0){
//       current  += (4 - reminder);
//     }
//     if(load_flag == 1){
//       uart_puts("\r");
//       char *program_start = (void*)current;
//       uart_puts("LOAD USER PROG\n\r");
//       from_el1_to_el0((unsigned int)program_start, (unsigned int)USER_STACK_TOP);
//       break;
//     }
//     current += filesize;
//     total_size = current - (char*) pos;
//     reminder = total_size % 4;
//     if(reminder != 0){
//       current += (4 - reminder);
//     }
//   }
// }