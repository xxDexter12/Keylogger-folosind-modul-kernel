
#include "struct_keylogger.h"

static int __init keylogger_initialization(void)
{
    printk(KERN_INFO "hello");
    return 0;
}

static void __exit keylogger_exit(void)
{
    
}
module_init(keylogger_initialization);