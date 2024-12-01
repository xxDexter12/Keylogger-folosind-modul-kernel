#ifndef CLIPBOARD_CALLBACK_H
#define CLIPBOARD_CALLBACK_H
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/workqueue.h>

#define CLIPBOARD_BUFFER_SIZE 1024

//static char clipboard_buffer[CLIPBOARD_BUFFER_SIZE];
static struct workqueue_struct *clipboard_wq; // structura worqueue este o coada de structuri work


//handler pt scriere in /proc/keyboard_clipboard
ssize_t clipboard_write_procfs(struct file *file, const char __user *user_buffer,size_t count,loff_t *pos);

static const struct proc_ops clipboard_fops= {
.proc_write=clipboard_write_procfs,
};

#endif
