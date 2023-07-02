#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h> 
#include <linux/delay.h>

#include <linux/unistd.h>
#include <linux/string.h>

#if defined(__KERNEL__)
#include <linux/inet.h>
#include <uapi/linux/rpmsg_socket.h>
#endif

#define DP_CR5_SAF 0
#define AUDIO_MONITOR_EPT 125

#if 1
/* user space needs this */
#ifndef AF_RPMSG
#define AF_RPMSG	44
#define PF_RPMSG	AF_RPMSG

struct sockaddr_rpmsg {
	__kernel_sa_family_t family;
	__u32 vproc_id;
	__u32 addr;
};

#define RPMSG_LOCALHOST ((__u32)~0UL)
#endif
#endif

#if defined(__KERNEL__)
static int __init hello_init(void)
{
	//https://www.jianshu.com/p/d36cd66eec43
	//https://blog.csdn.net/qq_28779021/article/details/78583981
    struct sockaddr_rpmsg servaddr, cliaddr;

    printk(KERN_ALERT"~~~~~~~~~~~~~~~~~~~~~~~~Hello world~~~~~~~~~~~~~~~~~~~~~~~~\n");

    memset(&servaddr, 0, sizeof(struct sockaddr_rpmsg));
    servaddr.family = AF_RPMSG;
    servaddr.vproc_id = DP_CR5_SAF;
    servaddr.addr = 132;

    int32_t buf[128], ret;
    struct socket *sock = NULL;

    struct msghdr msg;
    struct kvec iov;
    //struct msghdr msg = {.msg_flags = msg_flags | MSG_NOSIGNAL};
    //struct kvec iov = {.iov_base = buf, .iov_len = size};

    memset(&msg, 0, sizeof(msg));
    msg.msg_name = &servaddr;
    msg.msg_namelen = sizeof(servaddr);

    //if ((server_socket = socket(PF_RPMSG, SOCK_SEQPACKET, 0)) != -1) {
    if ((ret = sock_create(servaddr.family, SOCK_SEQPACKET, 0, &sock)) >= 0) {
        //ALOGE("connect ret=%d\n", connect(server_socket, (struct sockaddr *)&servaddr, sizeof(servaddr)));
        printk(KERN_ALERT"kernel_connect ret=%d\n", kernel_connect(sock, (struct sockaddr *)&servaddr, sizeof(servaddr), 0));
        for (uint32_t i = 0; i < 10; i++) { 
          buf[1] = 23333;
          buf[0] = i;

          iov.iov_base = buf;
          iov.iov_len = 8;
    
          ret = kernel_sendmsg(sock, &msg, &iov, 1, iov.iov_len);
          printk(KERN_ALERT "sendmsg ret=%d\n", ret);
          ret = kernel_recvmsg(sock, &msg, &iov, 1, iov.iov_len, 0);
          printk(KERN_ALERT "recvmsg ret=%d, buf=%d, %d\n", ret, buf[0], buf[1]);
          //ret = sock_recvmsg(sock, &msg, msg.msg_flags); 


          //iov_iter_kvec(&msg.msg_iter, WRITE, &iov, 1, size);
          //ret = sock_sendmsg(sock, &msg);
          //printk(KERN_ALERT"write ret=%zd\n", write(server_socket, buf, 8));
          //ret = read(server_socket, buf, sizeof(buf));
          //printk(KERN_ALERT"read  ret=%d, buf=%d, %d\n", ret, buf[0], buf[1]);
        }
    }
    //mdelay(50);
}

static void __exit hello_exit(void)
{
    printk("Exit Hello world\n");
}

//module_platform_driver(my_driver);
subsys_initcall(hello_init);
module_exit(hello_exit);

MODULE_AUTHOR("Gateway");
MODULE_DESCRIPTION("hello world");
MODULE_LICENSE("GPL");

#endif
