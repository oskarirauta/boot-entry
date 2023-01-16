#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/syscalls.h>
#include <linux/fcntl.h>
#include <linux/file.h>
#include <linux/fs.h>
#include <asm/uaccess.h>

static ssize_t read_file(char* filename, void* buffer, size_t size, loff_t offset) {

  ssize_t cnt = 0;
  struct file *f = filp_open(filename, O_RDONLY, 0);

  if ( !f ) {
    return -ENOENT;
  }

  cnt = kernel_read(f, buffer, size, &offset);
  filp_close(f, NULL);
  return cnt;
}

static int boot_entry_proc_show(struct seq_file *m, void *v) {

  char *buf;
  const size_t size = 1281;
  ssize_t cnt;
  char *res = NULL;
  const size_t rsize = 257;
  char is_found = 0;

  if ((buf = kmalloc(size, GFP_KERNEL))) {

    cnt = read_file("/proc/cmdline", buf, size, 0);

    if ( cnt > 0 ) {

      buf[cnt%(size-1)] = '\0';

      if ((res = kmalloc(rsize, GFP_KERNEL))) {

        char read_val = 0;
        char read_name = 1;
        char read_next = 0;
        char quoted = 0;
        char quote = 0;
        char *ch = &buf[0];
        int c = 0;

        memset(res, '\0', rsize);

        while ( *ch != '\0' && is_found == 0 ) {

          if ( !quoted && c == 0 && ( *ch == '\'' || *ch == '"' )) {

            quoted = 1;
            quote = *ch;
            ++ch;
            continue;

          } else if ( quoted && ( *ch == '\'' || *ch == '"' )) {

            quoted = 0;
            quote = *ch;
            ++ch;
            continue;
          }

          if (( !quoted && c == 0 && *ch == ' ' ) || *ch == '\r' || *ch == '\n' ) {
            // do nothing
          } else if ( quoted || ( read_val && *ch == '=' ) || ( !quoted && ( *ch != ' ' && *ch != '=' ))) {

            res[c] = *ch == '\t' ? ' ' : *ch;
            c++;
          } else if ( read_name && *ch == ' ' ) {

            memset(res, '\0', rsize);
            c = 0;
          } else if ( read_name && *ch == '=' ) {

            read_name = 0;
            read_val = 1;
            read_next = strcmp(res, "boot_entry") == 0 ? 1 : 0;
            memset(res, '\0', rsize);
            c = 0;
          } else if ( read_val && *ch == ' ' ) {

            if ( read_next ) {

              read_val = 0;
              read_name = 1;
              is_found = 1;
              break;
            }

            memset(res, '\0', rsize);
            read_name = 1;
            read_val = 0;
            c = 0;

          }

          ++ch;
        }

        if ( read_next && read_val && !is_found && c > 0 ) {
          is_found = 1;
        }

        if ( is_found ) {
          seq_printf(m, "%s\n", res);
        }

        kfree(res);

      }
    }

    kfree(buf);
  }

  if ( !is_found && cnt > 0 ) {
    seq_printf(m, "kernel command line is missing boot_entry variable\n");
  } else if ( !is_found && cnt == 0 ) {
    seq_printf(m, "kernel command line is empty???\n");
  } else if ( !is_found && cnt < 0 ) {
    seq_printf(m, "failed to read /proc/cmdline - is procfs mounted?\n");
  } else if ( !is_found ) {
    seq_printf(m, "unknown error\n");
  }

  return 0;
}

static int boot_entry_proc_open(struct inode *inode, struct  file *file) {
  return single_open(file, boot_entry_proc_show, NULL);
}

static const struct proc_ops boot_entry_proc_fops = {
  .proc_open = boot_entry_proc_open,
  .proc_read = seq_read,
  .proc_lseek = seq_lseek,
  .proc_release = single_release,
};

static int __init boot_entry_proc_init(void) {
  proc_create("boot_entry", 0, NULL, &boot_entry_proc_fops);
  return 0;
}

static void __exit boot_entry_proc_exit(void) {
  remove_proc_entry("boot_entry", NULL);
}

MODULE_LICENSE("GPL");
module_init(boot_entry_proc_init);
module_exit(boot_entry_proc_exit);
