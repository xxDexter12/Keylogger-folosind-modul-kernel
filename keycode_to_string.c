#include "keycode_to_string.h"

//buffer va fi locul unde se va stoca litera asociata tastei apasate care este un code(keycode)
size_t keycode_to_string(int keycode,int shift_pressed,char*buffer,size_t buff_len)
{
    memset(buffer,0,buff_len);

    if(keycode >=KEY_RESERVED && keycode <=KEY_PAUSE) // KEY_RESERVED -val minima iar KEY_PAUSE-val maxima
    {
            const char *key;
            if (shift_pressed == 1) //majuscule
            {
                key = keymap[keycode][1];
            } else {
                key = keymap[keycode][0];
            }
            if (!key || strlen(key) == 0) {
                pr_err("Keycode %d nu are mapare validă\n", keycode);
                return 0;
            }
            snprintf(buffer,buff_len,key);
            return strlen(buffer);
    }
    return 0;
}