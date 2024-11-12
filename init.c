#include "struct_keylogger.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Autorul Tău");
MODULE_DESCRIPTION("Un simplu modul pentru logarea tastelor");

static int __init keyboard_logger_init(void) {
    printk(KERN_INFO "Keyboard logger loaded\n");
    return 0;
}

static void __exit keyboard_logger_exit(void) {
    printk(KERN_INFO "Keyboard logger unloaded\n");
}

module_init(keyboard_logger_init);
module_exit(keyboard_logger_exit);