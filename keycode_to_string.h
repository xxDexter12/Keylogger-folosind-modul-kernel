#ifndef KEYCODE_TO_STRING_H
#define KEYCODE_TO_STRING_H
#include <linux/kernel.h>
#include <linux/module.h>
#include <sys/types.h>


ssize_t keycode_to_string (int keycode,int shift);

#endif

