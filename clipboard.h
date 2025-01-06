#ifndef _CLIPBOARD_H_
#define _CLIPBOARD_H_

#include <linux/workqueue.h>
#include <linux/slab.h>

void init_clipboard(void);
void cleanup_clipboard(void);
void queue_clipboard_capture(void);

#endif