/**
 * @file    boot_variant.c
 * @author  Oskari Rauta
 * @date    20 January 2023
 * @version 0.99
 * @brief   A module that exposes a string defined as boot_variant.name on kernel's
 * cmdline or "unknown" in /proc/boot_variant
*/

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Oskari Rauta");
MODULE_DESCRIPTION("A module that exposes boot variant name in procfs");
MODULE_VERSION("0.99");

static char *name = "unknown";

module_param(name, charp, S_IRUGO);
MODULE_PARM_DESC(name, "boot variant name");

static int boot_variant_proc_show(struct seq_file *m, void *v) {

  seq_printf(m, "%s\n", name);
  return 0;
}

static int boot_variant_proc_open(struct inode *inode, struct  file *file) {
  return single_open(file, boot_variant_proc_show, NULL);
}

static const struct proc_ops boot_variant_proc_fops = {
  .proc_open = boot_variant_proc_open,
  .proc_read = seq_read,
  .proc_lseek = seq_lseek,
  .proc_release = single_release,
};

static int __init boot_variant_proc_init(void) {
  proc_create("boot_variant", 0, NULL, &boot_variant_proc_fops);
  return 0;
}

static void __exit boot_variant_proc_exit(void) {
  remove_proc_entry("boot_variant", NULL);
}

module_init(boot_variant_proc_init);
module_exit(boot_variant_proc_exit);
