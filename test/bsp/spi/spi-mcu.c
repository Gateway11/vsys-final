/*
 * SPI mcu->soc spi driver
 * 
 */

#include <linux/completion.h>
#include <linux/module.h>
#include <linux/sched/clock.h>
#include <linux/spi/spi.h>
#include <linux/miscdevice.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/poll.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/of_device.h>
#include <uapi/linux/spi/spi-mcu.h>

#define SPI_MSG_LEN 2048
static unsigned int delay_ms = 10;
static unsigned int spi_msg_len = 2048;
static unsigned int default_header = 0x55AA0002;

DEFINE_MUTEX(spimcu_lock);

struct spi_mcu_priv {
  struct spi_device *spi;
  struct task_struct *spisync_task;
  struct spi_transfer xfer;
  struct spi_message msg;
  wait_queue_head_t		rw_queue;
  unsigned char buf_r[SPI_MSG_LEN];
  unsigned char buf_w[SPI_MSG_LEN];
  bool tx_done;
  bool rx_done;
  bool xfer_start;
  struct class *spimcu_class;
  dev_t spimcu_devid;
  struct cdev spimcu_cdev;
};

static int thread_spi_xfer(void *data)
{
  struct spi_mcu_priv *spipriv = data;
  unsigned int *pVersion;
  int status;
  while(!kthread_should_stop())
  {
    mutex_lock(&spimcu_lock);
    status = spi_sync(spipriv->spi, &spipriv->msg);
    if (status != 0) {
      dev_err(&spipriv->spi->dev, "spi_sync() failed %d\n", status);
    }
    pVersion = (unsigned int *)spipriv->buf_r;
    if (*pVersion != default_header) {
      spipriv->xfer.len = 1;
      (void)spi_sync(spipriv->spi, &spipriv->msg);
      spipriv->xfer.len = spi_msg_len;
      mutex_unlock(&spimcu_lock);
      continue;
    }
    mutex_unlock(&spimcu_lock);
    spipriv->tx_done = true;
    spipriv->rx_done = true;
    wake_up_all(&spipriv->rw_queue);
    msleep(delay_ms);
  }
  return 0;
}

static ssize_t spimcu_read(struct file *fp, char __user *buf, size_t count, loff_t *f_pos)
{
  struct spi_mcu_priv *spipriv = fp->private_data;
  int cnt;
  unsigned long ret;

  //if (!access_ok(VERIFY_WRITE, buf, count))
  //  return -EFAULT;
  
  cnt = (count < spi_msg_len ? count : spi_msg_len);
  mutex_lock(&spimcu_lock);
  ret = copy_to_user(buf, spipriv->buf_r, cnt);
  spipriv->rx_done = false;
  mutex_unlock(&spimcu_lock);
  return ret != 0 ? -EFAULT : cnt;
}

static ssize_t spimcu_write(struct file *fp, const char __user *buf, size_t count, loff_t *f_pos)
{
  struct spi_mcu_priv *spipriv = fp->private_data;
  int cnt;
  ssize_t ret;
  
  //if (!access_ok(VERIFY_READ, buf, count))
  //  return -EFAULT;
  
  cnt = (count < spi_msg_len ? count : spi_msg_len);
  mutex_lock(&spimcu_lock);
  ret = copy_from_user(spipriv->buf_w, buf, cnt);
  spipriv->tx_done = false;
  mutex_unlock(&spimcu_lock);
  return ret != 0 ? -EFAULT : cnt;
}

static int spimcu_open(struct inode *ino, struct file *fp)
{
  struct spi_mcu_priv *spipriv;
  spipriv = container_of(ino->i_cdev, struct spi_mcu_priv, spimcu_cdev);
  fp->private_data = spipriv;

  if (!spipriv->xfer_start) {
    // no start/start failed, try start
    spipriv->spisync_task = kthread_run(thread_spi_xfer, spipriv, "%s", dev_name(&spipriv->spi->dev));
    if (IS_ERR(spipriv->spisync_task)) {
      dev_err(&spipriv->spi->dev, "kthread_run() failed\n");
      return PTR_ERR(spipriv->spisync_task);
    } else {
      mutex_lock(&spimcu_lock);
      spipriv->xfer_start = true;
      mutex_unlock(&spimcu_lock);
    }
  }
  return 0;
}

static int spimcu_release(struct inode *ino, struct file *fp)
{
  int ret;
  struct spi_mcu_priv *spipriv;
  (void)fp;
  spipriv = container_of(ino->i_cdev, struct spi_mcu_priv, spimcu_cdev);

  ret = kthread_stop(spipriv->spisync_task);
  mutex_lock(&spimcu_lock);
  spipriv->xfer_start = false;
  mutex_unlock(&spimcu_lock);
  return ret;
}

static long spimcu_ioctl(struct file *fp, unsigned int cmd,
             unsigned long arg)
{
  int ret;

  ret = 0;
  switch(cmd) {
  case SPIMCU_SET_DELAY_IOCTL:
    mutex_lock(&spimcu_lock);
    delay_ms = (unsigned int)arg;
    mutex_unlock(&spimcu_lock);
    break;

  case SPIMCU_SET_LEN_IOCTL:
    break;

  default:
    ret = -EINVAL;
    break;
  }

  return ret;
}

static unsigned int spimcu_poll(struct file *fp, poll_table *wait)
{
  struct spi_mcu_priv *spipriv = fp->private_data;
  unsigned int mask = 0;

  poll_wait(fp, &spipriv->rw_queue, wait);

  mutex_lock(&spimcu_lock);
  if (spipriv->rx_done) {
    mask |= (POLLIN | POLLRDNORM);
  }

  if (spipriv->tx_done) {
    mask |= (POLLOUT | POLLWRNORM);
  }
  mutex_unlock(&spimcu_lock);

  return mask;
}

static struct file_operations spimcu_fops = {
  .owner	 = THIS_MODULE,
  .read	= spimcu_read,
  .write	 = spimcu_write,
  .open	 = spimcu_open,
  .release = spimcu_release,
  .unlocked_ioctl = spimcu_ioctl,
  .poll =		spimcu_poll,
};

static int spi_mcu_probe(struct spi_device *spi)
{
  struct spi_mcu_priv *spipriv;

  spipriv = devm_kzalloc(&spi->dev, sizeof(*spipriv), GFP_KERNEL);
  if (!spipriv) {
    return -ENOMEM;
  }

  spipriv->spi = spi;
  spipriv->xfer.tx_buf = spipriv->buf_w;
  spipriv->xfer.rx_buf = spipriv->buf_r;
  spipriv->xfer.len = spi_msg_len;

  spipriv->tx_done = false;
  spipriv->rx_done = false;

  spi_message_init_with_transfers(&spipriv->msg, &spipriv->xfer, 1);
  spi_set_drvdata(spi, spipriv);
  
  (void)alloc_chrdev_region(&spipriv->spimcu_devid, 0, 1, "spimcu");
  spipriv->spimcu_cdev.owner = THIS_MODULE;
  cdev_init(&spipriv->spimcu_cdev, &spimcu_fops);
  (void)cdev_add(&spipriv->spimcu_cdev, spipriv->spimcu_devid, 1);
  init_waitqueue_head(&spipriv->rw_queue);
  
  
  spipriv->spimcu_class = class_create(THIS_MODULE, "spimcu_class");
  (void)device_create(spipriv->spimcu_class, NULL, spipriv->spimcu_devid, NULL, "spimcu");
  return 0;
}

static int spi_mcu_remove(struct spi_device *spi)
{
  struct spi_mcu_priv *spipriv = spi_get_drvdata(spi);
  cdev_del(&spipriv->spimcu_cdev);
  unregister_chrdev_region(spipriv->spimcu_devid, 1);
  
  device_destroy(spipriv->spimcu_class, spipriv->spimcu_devid);
  class_destroy(spipriv->spimcu_class);

  return 0;
}
#ifdef CONFIG_OF
static const struct of_device_id mcu_spi_dt_ids[] = {
  { .compatible = "spi-mcu" },
  {}
};
MODULE_DEVICE_TABLE(of, mcu_spi_dt_ids);
#endif

static struct spi_driver spi_mcu_driver = {
  .driver = {
    .name	= "spi-mcu",
    .of_match_table = of_match_ptr(mcu_spi_dt_ids),
  },
  .probe		= spi_mcu_probe,
  .remove 	= spi_mcu_remove,
};
module_spi_driver(spi_mcu_driver);

MODULE_AUTHOR("daweij <jin.dawei@kotei.com.cn>");
MODULE_DESCRIPTION("SPI mcu->soc test");
MODULE_LICENSE("GPL v2");
