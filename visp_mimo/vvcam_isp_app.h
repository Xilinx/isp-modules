
#ifndef __VVCAM_ISP_APP_H__
#define __VVCAM_ISP_APP_H__

#include <linux/kernel.h>  
#include <linux/module.h>  


struct Chn_info
{
	uint32_t HwId;
	uint32_t Mode;
	uint32_t VtId;
	uint32_t path;
};

int MediaIspDeviceCameraDisConnect(struct vvcam_isp_dev *isp_dev, uint8_t Port, uint8_t Chn);

#define ENABLE_LOG 1

#ifdef ENABLE_LOG
  
    /* warning */
    #define logs(fmt, ...) \
       printk(KERN_ERR "%s %s:%d: " fmt,__FILE__, __func__, __LINE__, ##__VA_ARGS__)


    /* warning */
    #define logi(fmt, ...) \
      printk(KERN_ERR  ("WARNING: %s:%d: " fmt, __func__, __LINE__, ##__VA_ARGS__)

    /* warning */
    #define logw(fmt, ...) \
        printk(KERN_ERR  ("WARNING: %s:%d: " fmt, __func__, __LINE__, ##__VA_ARGS__)

    /* error */
    #define loge(fmt, ...) \
        printk(KERN_ERR  " %s:%d: " fmt, __func__, __LINE__, ##__VA_ARGS__)

    /* error */
    #define logr() \
         printk(KERN_ERR  " %s:%d:-R ", __func__, __LINE__)
#else
    /* Disable log*/
    #define logs(fmt, ...)
    #define logw(fmt, ...)
    #define loge(fmt, ...)
#endif
#endif




