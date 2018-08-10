#include <linux/module.h>
#include <linux/fs.h>     
#include <linux/cdev.h>  
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/io.h>

#define GPIO_PORT_R 16
#define GPIO_PORT_L 25
#define GPIO_PORT_C 21
#define GPIO_COUNT 3
int gpio_ports[GPIO_COUNT] = {GPIO_PORT_R, GPIO_PORT_L, GPIO_PORT_C};

MODULE_AUTHOR("Ryuya Saito");
MODULE_DESCRIPTION("driver for LED control");
MODULE_LICENSE("GPL");
MODULE_VERSION("0.1");

static dev_t dev;    // メジャー番号格納
static struct cdev cdv;  // デバイス情報を記憶する構造体
static struct class *cls = NULL;   
static volatile u32 *gpio_base = NULL;


static ssize_t led_write(struct file* filp, const char* buf, size_t count,
	                 loff_t* pos)
{
    char c;
    if(copy_from_user(&c, buf, sizeof(char)))
    {
         return -EFAULT;	    
    }
    if(c == '1')
    {
         gpio_base[7] = 1 << GPIO_PORT_R;	    
         gpio_base[10] = 1 << GPIO_PORT_L;	    
         gpio_base[10] = 1 << GPIO_PORT_C;	    
    }
    else if(c == '2')
    {
         gpio_base[7] = 1 << GPIO_PORT_L;	    
         gpio_base[10] = 1 << GPIO_PORT_R;	    
         gpio_base[10] = 1 << GPIO_PORT_C;	    
    }
    else if(c == '3')
    {
         gpio_base[7] = 1 << GPIO_PORT_C;	    
         gpio_base[10] = 1 << GPIO_PORT_R;	    
         gpio_base[10] = 1 << GPIO_PORT_L;	    
    }
    else if(c == '0')
    {
         gpio_base[10] = 1 << GPIO_PORT_R;	    
         gpio_base[10] = 1 << GPIO_PORT_L;	    
         gpio_base[10] = 1 << GPIO_PORT_C;	    
    }
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
    int gpio_count;

    /* gpioアドレス0.3f200000を0xA0までアドレスを設定する */
    gpio_base = ioremap_nocache(0x3f200000, 0xA0);

    gpio_count = 0;
    while(gpio_count < GPIO_COUNT)
    {
        const u32 led = gpio_ports[gpio_count];
        const u32 index = led/10;  // GPFSEL2
        const u32 shift = (led%10)*3;  // 15bit
        const u32 mask = ~(0x7 << shift);  
        gpio_base[index] = (gpio_base[index] & mask) | (0x1 << shift); // 001: output
	gpio_count++;
    }

    /* 番号の取得(デバイス番号の入れ物, マイナー番号を0から, 1個よこせ, デバイス名) */
    retval = alloc_chrdev_region(&dev, 0, 1, "myled_cls");
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

    /* クラスの初期化 */
    cls = class_create(THIS_MODULE, "myled_cls");
    if(IS_ERR(cls))
    {
         printk(KERN_ERR "class_create failed.");    
	 /* cls構造体にエラー情報が入っている */
	 return PTR_ERR(cls);
    }
    /* デバイス情報を追加 */
    /* device_create(クラス構造体アドレス, デバイス番号, 名前)*/
    device_create(cls, NULL, dev, NULL, "myled_cls%d", MINOR(dev));

    return 0;
}


static void __exit cleanup_mod(void)
{
    cdev_del(&cdv);
    device_destroy(cls, dev);
    class_destroy(cls);
    /* デバイス番号の開放(デバイス番号の入れ物, マイナー番号の数) */
    unregister_chrdev_region(dev, 1);
    printk(KERN_INFO "%s is unloaded. major:%d\n", __FILE__, MAJOR(dev));
}

module_init(init_mod);
module_exit(cleanup_mod);
