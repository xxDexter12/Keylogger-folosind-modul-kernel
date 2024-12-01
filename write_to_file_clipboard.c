#include "write_to_file_clipboard.h"

void write_to_file_clipboard(struct work_struct* work)
{
    struct CLIPBOARD* clipboard;
    clipboard=container_of(work,CLIPBOARD,work_clipboard);
    kernel_write(clipboard->filename,clipboard->clipboard_write_buffer,CLIPBOARD_BUFFER_SIZE,clipboard->offset_in_file);
}