#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/random.h>
#include <linux/string.h>
#include <linux/kobject.h>
#include <linux/slab.h>

#define MAX_MSGS 10
#define MAX_MSG_LEN 100

static char *messages[MAX_MSGS];
static int num_msgs = 5;

// Default messages (will be copied dynamically)
static const char *default_messages[] = {
    "The kernel is wise, but /dev/null is its void.",
    "Why did the sysadmin quit? They couldn't sudo anymore!",
    "Segmentation fault (core dumped)... but where?",
    "rm -rf /bin/laden: Aladeen result.",
    "There are 10 types of people: those who understand binary and those who don't.",
    "Why do programmers prefer dark mode? Because light attracts bugs.",
    "A SQL query walks into a bar, walks up to two tables and asks, 'Can I join you?'",
    "Why do Java developers wear glasses? Because they don't see sharp.",
    "How many programmers does it take to change a light bulb? None, that's a hardware problem.",
    "Why was the developer unhappy at their job? They wanted arrays.",
    "What's a programmer's favorite hangout place? Foo Bar.",
    "Why do programmers hate nature? It has too many bugs.",
    "What's the object-oriented way to become wealthy? Inheritance.",
    "Why did the programmer go broke? Because they used up all their cache.",
    "What's a programmer's favorite type of music? Algo-rhythm.",
    "Why do Python programmers prefer snake_case? Because they can't C.",
    "How do you comfort a JavaScript bug? You console it.",
    "Why was the function feeling sad? It had too many arguments.",
    "Why don't programmers like to go outside? The sunlight causes too many glares on their screens.",
    "What do you call a programmer from Finland? Nerdic."
};


module_param_array_named(fortunes, messages, charp, &num_msgs, 0644);
MODULE_PARM_DESC(fortunes, "Custom fortune messages (max 10)");

static struct kobject *fortune_kobj;

// Sysfs read handler
static ssize_t message_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf) {
    unsigned int rand_idx;
    get_random_bytes(&rand_idx, sizeof(rand_idx));
    rand_idx %= num_msgs;

    return scnprintf(buf, PAGE_SIZE, "%s\n", messages[rand_idx]);
}

static struct kobj_attribute message_attr = __ATTR_RO(message);

static int __init fortune_init(void) {
    int i;

    // If no parameters were passed, copy default messages
    if (num_msgs == 5) {
        for (i = 0; i < num_msgs; i++) {
            messages[i] = kstrdup(default_messages[i], GFP_KERNEL);
            if (!messages[i]) {
                // Cleanup on error
                while (--i >= 0) kfree(messages[i]);
                return -ENOMEM;
            }
        }
    }

    // Create sysfs entry
    fortune_kobj = kobject_create_and_add("fortunecookie", kernel_kobj->parent);
    if (!fortune_kobj) {
        for (i = 0; i < num_msgs; i++) kfree(messages[i]);
        return -ENOMEM;
    }

    if (sysfs_create_file(fortune_kobj, &message_attr.attr)) {
        kobject_put(fortune_kobj);
        for (i = 0; i < num_msgs; i++) kfree(messages[i]);
        return -ENOMEM;
    }

    printk(KERN_INFO "Fortune cookie module loaded!\n");
    return 0;
}

static void __exit fortune_exit(void) {
    int i;

    // Free all allocated messages
    for (i = 0; i < num_msgs; i++) {
        kfree(messages[i]);
    }

    kobject_put(fortune_kobj);
    printk(KERN_INFO "Fortune cookie crumbled. Goodbye!\n");
}

module_init(fortune_init);
module_exit(fortune_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Grass");
MODULE_DESCRIPTION("Dispenses kernel-themed fortunes");