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
            snprintf(buffer,buff_len,key);
            return strlen(buffer);
    }
    return 0;
}