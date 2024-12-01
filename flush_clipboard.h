#ifndef FLUSH_CLIPBOARD_H
#define FLUSH_CLIPBOARD_H
#include "struct_clipboard.h"

//aceasta functie se ocupa cu eliberarea bufferului de citire a tastelor si punerea datelor din acesta intr-un buffer de scriere
//tot aici se programeaza scrierea in fisierul de log prin schedule_work

void flush_clipboard(struct CLIPBOARD *clipboard);

#endif