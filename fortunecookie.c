#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/random.h>
#include <linux/string.h>
#include <linux/kobject.h>
#include <linux/slab.h>

#define MAX_MSGS 10
#define MAX_MSG_LEN 300

static char *messages[MAX_MSGS];
static int num_msgs = 10; // Now matches the 10 default messages
static bool defaults_loaded = false; // Track if defaults are used

static const char *default_messages[] = {
    "You look like you’ve barely slept. Are you okay?",
    "Why does your code always work the second time but never the first?",
    "Did you just reboot the Wi-Fi to fix it again?",
    "Wait, why is your keyboard so loud?",
    "I can’t believe you memorized your IP address.",
    "You drink coffee like it’s water, don’t you?",
    "How do you even *understand* that terminal stuff?",
    "Why is everything in dark mode? Even your life?",
    "Do you just sit in front of your computer all day?",
    "You’re probably the kind of person who reads Linux man pages for fun."
};

module_param_array_named(fortunes, messages, charp, &num_msgs, 0644);
MODULE_PARM_DESC(fortunes, "Custom fortune messages (max 10)");

static struct kobject *fortune_kobj;

static ssize_t message_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf) {
    unsigned int rand_idx;
    get_random_bytes(&rand_idx, sizeof(rand_idx));
    rand_idx %= num_msgs;
    return scnprintf(buf, PAGE_SIZE, "%s\n", messages[rand_idx]);
}

static struct kobj_attribute message_attr = __ATTR_RO(message);

static int __init fortune_init(void) {
    int i;

    // Load defaults only if no parameters provided
    if (num_msgs == 10) {
        for (i = 0; i < num_msgs; i++) {
            messages[i] = kstrdup(default_messages[i], GFP_KERNEL);
            if (!messages[i]) {
                while (--i >= 0) kfree(messages[i]);
                return -ENOMEM;
            }
        }
        defaults_loaded = true; // Mark defaults as loaded
    }

    // Create sysfs entry
    fortune_kobj = kobject_create_and_add("fortunecookie", kernel_kobj);
    if (!fortune_kobj) {
        if (defaults_loaded) {
            for (i = 0; i < num_msgs; i++) kfree(messages[i]);
        }
        return -ENOMEM;
    }

    if (sysfs_create_file(fortune_kobj, &message_attr.attr)) {
        kobject_put(fortune_kobj);
        if (defaults_loaded) {
            for (i = 0; i < num_msgs; i++) kfree(messages[i]);
        }
        return -ENOMEM;
    }

    printk(KERN_INFO "Fortune cookie module loaded!\n");
    return 0;
}

static void __exit fortune_exit(void) {
    int i;

    // Free only if defaults were loaded
    if (defaults_loaded) {
        for (i = 0; i < num_msgs; i++) {
            kfree(messages[i]);
        }
    }

    kobject_put(fortune_kobj);
    printk(KERN_INFO "Fortune cookie crumbled. Goodbye!\n");
}

module_init(fortune_init);
module_exit(fortune_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Grass");
MODULE_DESCRIPTION("Dispenses kernel-themed fortunes");