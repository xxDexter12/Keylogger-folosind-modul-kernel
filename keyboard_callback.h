#ifndef KEYBOARD_CALLBACK_H
#define KEYBOARD_CALLBACK_H
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/input.h>
#include <linux/keyboard.h>
#include <linux/file.h>
#include <linux/fs.h>
#include <linux/workqueue.h>
int keyboard_callback(struct notifier_block* nb,unsigned long action, void *data);
#endif