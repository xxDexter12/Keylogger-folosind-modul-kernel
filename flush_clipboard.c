#include "flush_clipboard.h"

void flush_clipboard(struct CLIPBOARD *clipboard)
{
    char *aux=clipboard->clipboard_read_buffer;
    clipboard->clipboard_read_buffer=clipboard->clipboard_write_buffer;
    clipboard->clipboard_write_buffer=aux;

    schedule_work(&clipboard->work_clipboard); //aceasta functie programeaza o sarcina asincrona pt a putea scrie in fisierul de log fara sa intrerupa executia programului.

    memset(clipboard->clipboard_read_buffer,0,CLIPBOARD_BUFFER_SIZE);
    clipboard->clipboard_read_buffer_offset=0;
}
