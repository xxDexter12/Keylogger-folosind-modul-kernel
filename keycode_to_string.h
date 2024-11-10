#ifndef KEYCODE_TO_STRING_H
#define KEYCODE_TO_STRING_H
#include <linux/kernel.h>
#include <linux/module.h>


ssize_t keycode_to_string (int keycode,int shift);

#endif

