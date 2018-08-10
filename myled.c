#include <linux/cdev.h>  
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>     


MODULE_AUTHOR("Ryuya Saito");
MODULE_DESCRIPTION("driver for LED control");
MODULE_LICENSE("GPI2.0");
MODULE_VERSION("0.1");

static dev_t dev;    // メジャー番号格納
static struct cdev cdv;  // デバイス情報を記憶する構造体


static ssize_t led_write(struct file* filp, const char* buf, size_t count,
	                 loff_t* pos)
{
    printk(KERN_INFO "led_write is called\n");
    return 1;
}


static struct file_operations led_fops = {
    /* file_operations構造体はデバイスの挙動を書いた関数のリストを格納する
       THIS_MODULEマクロ(カーネルモジュールの情報を管理する構造体のポインタに置き換える)
       writeの指す先はデバイスファイル(/dev/myled0)に何か書き込まれると呼び出されるので
            関数のポインタを渡す
       cdev_initに渡すことで挙動を設定できる
        */
    .owner = THIS_MODULE,
    .write = led_write
};


static int __init init_mod(void)
{
    int retval;
    /* 番号の取得(デバイス番号の入れ物, マイナー番号を0から, 1個よこせ, デバイス名) */
    retval = alloc_chrdev_region(&dev, 0, 1, "myled");
    if(retval < 0)
    {
         printk(KERN_ERR "alloc_chrdev_region failure.\n");
         return retval;
    }
    /* MAJORマクロでメジャー番号抽出 */
    printk(KERN_INFO "%s is loaded. major:%d\n", __FILE__, MAJOR(dev));

    /* デバイスの挙動を書いた関数のリストを格納する構造体とcdvを結びつける */
    cdev_init(&cdv, &led_fops);
    cdv.owner = THIS_MODULE;
    /* デバイスをカーネルに登録(cdvのアドレス, デバイス番号, デバイスの個数) 
      補足：２つ以上デバイス番号を作るときはMKDEVマクロで
      MKDEV(MAJOR(dev), MINOR(dev)+1)とdev_t型のデバイス番号を作りなおす*/
    retval = cdev_add(&cdv, dev, 1); 
    if(retval < 0)
    {
         printk(KERN_ERR "cdev_add failed. major:%d, minor: %d\n",
                MAJOR(dev), MINOR(dev));
         return retval;
    }

    return 0;
}


static void __exit cleanup_mod(void)
{
    cdev_del(&cdv);
    /* デバイス番号の開放(デバイス番号の入れ物, マイナー番号の数) */
    unregister_chrdev_region(dev, 1);
    printk(KERN_INFO "%s is unloaded. major:%d\n", __FILE__, MAJOR(dev));
}

module_init(init_mod);
module_exit(cleanup_mod);
