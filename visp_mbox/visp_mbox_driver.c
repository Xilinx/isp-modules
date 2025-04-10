#include <linux/arm-smccc.h>
#include <linux/module.h>
#include <linux/of_graph.h>
#include <linux/of_reserved_mem.h>
#include <linux/platform_device.h>
#include <linux/pm_runtime.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/version.h>
#include <linux/vmalloc.h>
#include <media/v4l2-ctrls.h>
#include <media/v4l2-device.h>
#include <media/v4l2-event.h>
#include <media/v4l2-fh.h>
#include <media/v4l2-fwnode.h>
#include <media/v4l2-ioctl.h>
#include <media/v4l2-mc.h>
#include <media/v4l2-mediabus.h>
#include <media/videobuf2-dma-contig.h>
//#include <linux/sensor_cmd.h>
#include <linux/delay.h>
#include <linux/of_address.h>

#include "visp_driver.h"
#include "visp_event.h"
#include "visp_v4l2_common.h"
//#include "visp_ctrl.h"
#include <linux/delay.h>
#include <linux/interrupt.h>

#include "sensor_cmd.h"
#include "visp_procfs.h"
//#include "mailbox.h"
#include <linux/kernel.h>
#include <linux/ktime.h>
#include "visp_mbox_driver.h"
#include "mbox_api.h"
#include "mbox_cmd.h"
#include "visp_app.h"

#define RPU0_IPI_ID 3
#define RPU1_IPI_ID 4
#define RPU6 6
#define RPU7 7
#define RPU8 8
#define RPU9 9
#define MAX_ISP_INSTANCES 6
#define MAX_RPU_ID 8

#define RPU6_0 0
#define RPU6_1 1
#define RPU6_2 2
#define RPU6_3 3

struct class *mailbox_class;
static DEFINE_MUTEX(rpu_list_lock);
static LIST_HEAD(rpu_devices);
static DECLARE_COMPLETION(mailbox_completion);
int (*exported_func1)(void *, struct visp_dev *isp_dev);
static void my_tasklet_function(void *data);

int get_dest_cpu(int rpu_id) {
    int dest_cpu;
    switch (rpu_id) {
        case 6:
            dest_cpu = RPU6_0;
            break;
        case 7:
            dest_cpu = RPU6_1;
            break;
        case 8:
            dest_cpu = RPU6_2;
            break;
        case 9:
            dest_cpu = RPU6_3;
            break;
        default:
            // Handle invalid rpu_id (you might want to return an error code)
            dest_cpu = -1; // Or another indicator of an invalid ID
            break;
    }
    return dest_cpu;
}

static void handle_event_notified(struct work_struct *work)
{
	//struct visp_dev *isp_dev;
	struct rpu_dev *rpu;

	/* Get the visp_dev structure from the work struct */
	rpu = container_of(work, struct rpu_dev, mbox_work);
	if (!rpu)
	{
		pr_err("my_tasklet_function: rpu_dev is NULL\n");
		return;
	}
	/* Send a message to the RX mailbox channel */
	(void)mbox_send_message(rpu->rx_chan, NULL);

	/* Iterate over all notify IDs and process them */
	//    idr_for_each(&isp_dev->notifyids, event_notified_idr_cb, isp_dev);

	/* Execute the tasklet function */
	my_tasklet_function(rpu);
}

static void visp_mb_tx_done(struct mbox_client *cl, void *msg, int r)
{
	return;
}

static void my_tasklet_function(void *data)
{
	struct rpu_dev *rpu = (struct rpu_dev *)data;
	uint32_t recv_cmd_id;
	uint32_t isp_id;
	struct visp_dev *isp_dev;
	static int cnt = 0;
	/* Check for a valid RPU device */
	if (!rpu)
	{
		pr_err("my_tasklet_function: rpu_dev is NULL\n");
		return;
	}

	/* Read command ID and ISP ID from mailbox */
	mutex_lock(&rpu->read_lock);
	recv_cmd_id = apu_mailbox_read(get_dest_cpu(rpu->rpu_id), &isp_id);
	if (isp_id < 0 || isp_id >= MAX_ISP_INSTANCES)
	{
		pr_err("my_tasklet_function: Invalid isp_id %u\n", isp_id);
		return;
	}
	/* Retrieve the corresponding ISP device */
	isp_dev = rpu->isp_dev[isp_id];
	if (!isp_dev)
	{
		pr_err("my_tasklet_function: ISP device is NULL for isp_id %u\n",
			   isp_id);
		return;
	}
	/* Handle specific commands */
	if (isp_dev->k_apu_ack_flag && (recv_cmd_id == MB_CMD_RES_SUCCESS))
	{
		complete(&isp_dev->apu_wait_for_ack);
	}
	else if (isp_dev->k_apu_data_flag && (recv_cmd_id == MB_CMD_GET_SUCCESS))
	{
		complete(&isp_dev->apu_wait_for_data);
	}
	else if ((rpu->app_wait_flag && (recv_cmd_id == MB_CMD_RES_SUCCESS)) ||
			 (rpu->app_wait_flag && (recv_cmd_id == MB_CMD_GET_SUCCESS)))
	{
		complete(&mailbox_completion);
	}
	else if (recv_cmd_id == RPU_2_APU_CMD_DISPLAY_BUFFER)
	{
		exported_func1 = symbol_get(Handle_Frameout_Buffer);
		if (!exported_func1)
		{
			pr_err(
				"my_tasklet_function: Handle_Frameout_Buffer function not "
				"available\n");
			mutex_unlock(&rpu->read_lock);
			return;
		}
		exported_func1(data_from_interrupt.data, isp_dev);
		mutex_unlock(&rpu->read_lock);
	}
	else
	{
		mutex_unlock(&rpu->read_lock);
		pr_err("my_tasklet_function: Unexpected command ID %d received\n",
			   recv_cmd_id);
	}
	cnt++;
	// mutex_unlock(&rpu->read_lock);
}

static int get_rpu_id(int ipi_id)
{
	int rpu_id;

	/* Determine the RPU ID based on the IPI ID */
	switch (ipi_id)
	{
		case RPU0_IPI_ID:
			rpu_id = RPU6;
			break;

		case RPU1_IPI_ID:
			rpu_id = RPU7;
			break;

		default:
			rpu_id = 6; //-1; // Return an error code for invalid IPI ID
			break;
	}

	return rpu_id;
}

static void visp_mb_rx_cb(struct mbox_client *cl, void *msg)
{
	int rpu_id = 0;
	struct rpu_dev *rpu = NULL;
	/* Get RPU ID from the received message */
	rpu_id = get_rpu_id(((struct zynqmp_ipi_message *)msg)->len);
	/* Get RPU device pointer using the RPU ID */
	rpu = get_rpu_dev(rpu_id);
	if (rpu != NULL)
	{
		/* Schedule work to handle the received event */
		if ( queue_work_on(1, system_wq, &rpu->mbox_work) )
		{
		}
		else
		{
			pr_err("visp_mb_rx_cb: Failed to schedule work for RPU ID %d\n",
				   rpu_id);
		}
	}
	else
	{
		pr_err("visp_mb_rx_cb: Failed to find RPU device for RPU ID %d\n",
			   rpu_id);
	}
}

static int visp_setup_mbox(struct rpu_dev *rpu, struct device_node *node)
{
	struct mbox_client *mclient;

	if (!rpu || !rpu->dev)
	{
		dev_err(NULL, "visp_setup_mbox: Invalid RPU device structure.\n");
		return -EINVAL;
	}

	/* Setup TX mailbox channel client */
	mclient = &rpu->tx_mc;
	mclient->rx_callback = NULL;
	mclient->tx_block = false;
	mclient->knows_txdone = false;
	mclient->tx_done = visp_mb_tx_done; // Set the TX done callback
	mclient->dev = rpu->dev;

	/* Setup RX mailbox channel client */
	mclient = &rpu->rx_mc;
	mclient->dev = rpu->dev;
	mclient->rx_callback = visp_mb_rx_cb; // Set the RX callback
	mclient->tx_block = false;
	mclient->knows_txdone = false;

	/* Initialize work */
	INIT_WORK(&rpu->mbox_work, handle_event_notified);
	dev_dbg(rpu->dev, "Mailbox work handler initialized.\n");

	/* Request TX channel */
	rpu->tx_chan = mbox_request_channel_byname(&rpu->tx_mc, "tx");
	if (IS_ERR(rpu->tx_chan))
	{
		dev_err(rpu->dev, "Failed to request TX mailbox channel.\n");
		rpu->tx_chan = NULL;
		return PTR_ERR(rpu->tx_chan);
	}

	/* Request RX channel */
	rpu->rx_chan = mbox_request_channel_byname(&rpu->rx_mc, "rx");
	if (IS_ERR(rpu->rx_chan))
	{
		dev_err(rpu->dev, "Failed to request RX mailbox channel.\n");
		rpu->rx_chan = NULL;
		return PTR_ERR(rpu->rx_chan);
	}

	/* Initialize TX mailbox client SKB queue */
	skb_queue_head_init(&rpu->tx_mc_skbs);

	return 0;
}

static int char_dev_open(struct inode *inode, struct file *file)
{
	struct rpu_dev *rpu = NULL;

	/* Retrieve the RPU device structure using container_of */
	rpu = container_of(inode->i_cdev, struct rpu_dev, cdev);
	if (!rpu)
	{
		pr_err("%s: Failed to retrieve RPU device\n", __func__);
		return -ENODEV; // Return error if RPU is not found
	}

	/* Set private_data for the file structure */
	file->private_data = rpu;

	return 0; // Indicate successful open
}

static int char_dev_release(struct inode *inode, struct file *file)
{
	struct rpu_dev *rpu = file->private_data;

	/* Perform any necessary cleanup or validation */
	if (!rpu)
	{
		pr_err("%s: Release called with invalid RPU device\n", __func__);
		return -ENODEV; // Return error if RPU is invalid
	}

	return 0; // Indicate successful release
}

static ssize_t char_dev_write(struct file *file, const char __user *buf,
							  size_t lbuf, loff_t *offset)
{
	struct rpu_dev *rpu = file->private_data;
	payload_packet *packet = NULL;
	payload_user_packet *user_packet = NULL;
	uint32_t instanceId = 0x99;
	uint8_t *p_data;
	int ret = 0;
	/* Validate input buffer size */
	if (lbuf < sizeof(payload_user_packet))
	{
		pr_err("%s: Insufficient input buffer size\n", __func__);
		return -EINVAL;
	}
	/* Validate RPU instance */
	if (!rpu)
	{
		pr_err("%s: RPU instance is NULL\n", __func__);
		kfree(packet);
		kfree(user_packet);
		return -EINVAL;
	}

	/* Allocate memory for user_packet */
	user_packet = kmalloc(sizeof(payload_user_packet), GFP_KERNEL);
	if (!user_packet)
	{
		pr_err("%s: Failed to allocate memory for user_packet\n", __func__);
		return -ENOMEM;
	}
	/* Allocate memory for packet */
	packet = kmalloc(sizeof(payload_packet), GFP_KERNEL);
	if (!packet)
	{
		pr_err("%s: Failed to allocate memory for packet\n", __func__);
		kfree(packet);
		kfree(user_packet);
		return -ENOMEM;
	}

	/* Copy data from user space to kernel space */
	if (copy_from_user(user_packet, buf, sizeof(payload_user_packet)))
	{
		pr_err("%s: copy_from_user failed\n", __func__);
		kfree(packet);
		kfree(user_packet);
		return -EFAULT;
	}

	/* Populate packet with data from user_packet */
	packet->type = user_packet->type;
	packet->cookie = user_packet->cookie;
	packet->payload_size = user_packet->payload_size;
	memcpy(packet->payload, user_packet->payload, sizeof(user_packet->payload));

	/* Extract instanceId from payload */
	p_data = packet->payload;
	memcpy(&instanceId, p_data, sizeof(uint32_t));

	mutex_lock(&rpu->write_lock); // Replaced spinlock with mutex
	/* Send command and message */
	ret = Send_Command(user_packet->cmd_id, packet, sizeof(payload_packet),
					  get_dest_cpu(rpu->rpu_id), MBOX_CORE_APU);

	if (ret < 0)
	{
		pr_err("%s: Send_Command failed with error: %d\n", __func__, ret);
		kfree(packet);
		kfree(user_packet);
		return -EIO;
	}

	ret = mbox_send_message(rpu->tx_chan, NULL); // Trigger irq
	if (ret < 0)
	{
		pr_err("%s: mbox_send_message() failed: %d\n", __func__, ret);
		kfree(packet);
		kfree(user_packet);
		return -EIO;
	}
	mutex_unlock(&rpu->write_lock);
	kfree(packet);
	kfree(user_packet);
	return lbuf; // Return the number of bytes written
}

static ssize_t my_driver_read(struct file *file, char __user *buf, size_t count,
							  loff_t *offset)
{
	struct rpu_dev *rpu = file->private_data;
	int result;
	ssize_t bytes_copied = sizeof(struct response_user_packet);
	if (!rpu)
	{
		pr_err("%s: Invalid RPU device pointer\n", __func__);
		mutex_unlock(&rpu->read_lock);
		return -EINVAL;
	}

	mutex_lock(&rpu->ack_lock);
	rpu->app_wait_flag = 1;
	/* Acquire lock to protect RPU structure */
	/* Wait for completion interruptibly */
	result = wait_for_completion_interruptible(&mailbox_completion);
	if (result != 0)
	{
		pr_err("%s: Completion wait interrupted, error code: %d\n", __func__,
			   result);
		mutex_unlock(&rpu->read_lock);
		return -ERESTARTSYS; // Return appropriate error for interruption
	}

	//    mutex_lock(&rpu->read_lock);
	/* Copy data to user space */
	if (copy_to_user(buf, &data_from_interrupt, bytes_copied))
	{
		pr_err("%s: Failed to copy data to user space\n", __func__);
		mutex_unlock(&rpu->read_lock);
		return -EFAULT; // Handle the copy_to_user error
	}
	rpu->app_wait_flag = 0;
	mutex_unlock(&rpu->ack_lock);
	mutex_unlock(&rpu->read_lock);

	return bytes_copied; // Return the size of data copied on success
}

static struct file_operations char_dev_fops = {.owner = THIS_MODULE,
											   .read = my_driver_read,
											   .write = char_dev_write,
											   .open = char_dev_open,
											   .release = char_dev_release};
struct reserved_memory reserved_memory;
/* Function to find an existing rpu_dev by rpu_id */

struct rpu_dev *get_rpu_dev(int rpu_id)
{
	struct rpu_dev *rpu;

	/* Acquire lock to protect the RPU list */
	mutex_lock(&rpu_list_lock);

	/* Check if the RPU devices list is empty */
	if (list_empty(&rpu_devices))
	{
		pr_warn("get_rpu_dev: RPU devices list is empty.\n");
		mutex_unlock(&rpu_list_lock);
		return NULL;
	}

	/* Search for the RPU with the given ID */
	list_for_each_entry(rpu, &rpu_devices, node)
	{
		if (rpu->rpu_id == rpu_id)
		{
			//        pr_debug("get_rpu_dev: Found RPU device with ID %d.\n", rpu_id);
			mutex_unlock(&rpu_list_lock);
			return rpu;
		}
	}

	/* RPU not found, log an informational message */
	// pr_info("get_rpu_dev: RPU device with ID %d not found.\n", rpu_id);

	/* Unlock the mutex before returning */
	mutex_unlock(&rpu_list_lock);

	/* Return NULL as the device was not found */
	return NULL;
}
EXPORT_SYMBOL(get_rpu_dev);

static struct rpu_dev *find_or_create_rpu(struct platform_device *pdev,
										  int rpu_id)
{
	struct rpu_dev *rpu;
	struct device_node *node;
	char dev_name[20];
	int ret;

	node = pdev->dev.of_node;
	mutex_lock(&rpu_list_lock);

	/* Look for an existing RPU with the given rpu_id */
	list_for_each_entry(rpu, &rpu_devices, node)
	{
		if (rpu->rpu_id == rpu_id)
		{
			kref_get(&rpu->refcount);
			//      dev_dbg(&pdev->dev, "Found existing RPU with ID %d\n", rpu_id);
			mutex_unlock(&rpu_list_lock);
			return rpu;
		}
	}

	/* Create a new RPU */
	rpu = devm_kzalloc(&pdev->dev, sizeof(*rpu), GFP_KERNEL);
	if (!rpu)
	{
		dev_err(&pdev->dev, "Failed to allocate memory for RPU\n");
		mutex_unlock(&rpu_list_lock);
		return ERR_PTR(-ENOMEM);
	}

	snprintf(dev_name, sizeof(dev_name), "%s_%d", CHAR_DEV_NAME, rpu_id);

	rpu->dev = &pdev->dev;
	rpu->rpu_id = rpu_id;

	/* Initialize mutexes, cdev, and reference count */
	mutex_init(&rpu->lock);
	mutex_init(&rpu->rpu_lock);
	mutex_init(&rpu->ack_lock);
	mutex_init(&rpu->read_lock);
	mutex_init(&rpu->write_lock);
	cdev_init(&rpu->cdev, &char_dev_fops);
	rpu->cdev.owner = THIS_MODULE;

	/* Allocate a device number */
	ret = alloc_chrdev_region(&rpu->devno, rpu_id, 1, dev_name);
	if (ret)
	{
		dev_err(&pdev->dev, "Failed to allocate device number (error %d)\n",
				ret);
		devm_kfree(&pdev->dev, rpu);
		mutex_unlock(&rpu_list_lock);
		return ERR_PTR(ret);
	}

	/* Add the character device */
	ret = cdev_add(&rpu->cdev, rpu->devno, 1);
	if (ret)
	{
		dev_err(&pdev->dev, "Failed to add cdev (error %d)\n", ret);
		unregister_chrdev_region(rpu->devno, 1);
		devm_kfree(&pdev->dev, rpu);
		mutex_unlock(&rpu_list_lock);
		return ERR_PTR(ret);
	}

	/* Create a device node */
	if (!device_create(mailbox_class, NULL, rpu->devno, NULL, dev_name))
	{
		dev_err(&pdev->dev, "Failed to create device node\n");
		cdev_del(&rpu->cdev);
		unregister_chrdev_region(rpu->devno, 1);
		devm_kfree(&pdev->dev, rpu);
		mutex_unlock(&rpu_list_lock);
		return ERR_PTR(-ENOMEM);
	}

	kref_init(&rpu->refcount);

	/* Add the new RPU to the list */
	list_add_tail(&rpu->node, &rpu_devices);

	/* Setup mailbox if required */
	if (of_property_read_bool(node, "mboxes"))
	{
		dev_dbg(&pdev->dev, "Setting up mailboxes for RPU with ID %d\n",
				rpu_id);
		ret = visp_setup_mbox(rpu, node);
		if (ret)
		{
			dev_err(&pdev->dev, "Failed to set up mailboxes (error %d)\n", ret);
			device_destroy(mailbox_class, rpu->devno);
			cdev_del(&rpu->cdev);
			unregister_chrdev_region(rpu->devno, 1);
			devm_kfree(&pdev->dev, rpu);
			mutex_unlock(&rpu_list_lock);
			return ERR_PTR(ret);
		}
	}

	mutex_unlock(&rpu_list_lock);
	return rpu;
}

//TODO make this as static and keep appropriate name
static int reserved_memory_init(const char *node_name)
{
	struct device_node *node;
	int ret;

	if (!node_name)
	{
		pr_err("reserved_memory_init: Invalid node name (NULL pointer).\n");
		return -EINVAL;
	}

	/* Find the reserved memory node */
	node = of_find_node_by_name(NULL, node_name);
	if (!node)
	{
		pr_err("reserved_memory_init: Reserved memory node '%s' not found.\n",
			   node_name);
		return -ENODEV;
	}

	/* Get the physical address */
	ret = of_property_read_u64(node, "reg", &reserved_memory.phys_addr);
	if (ret)
	{
		pr_err(
			"reserved_memory_init: Failed to read 'reg' property for node "
			"'%s'. Error: %d\n",
			node_name, ret);
		return -EINVAL;
	}

	/* Map to virtual address */
	reserved_memory.virt_addr = of_iomap(node, 0);
	if (!reserved_memory.virt_addr)
	{
		pr_err(
			"reserved_memory_init: Failed to map reserved memory to virtual "
			"address for node '%s'.\n",
			node_name);
		return -ENOMEM;
	}

	pr_info(
		"reserved_memory_init: Successfully initialized reserved memory for "
		"node '%s'.\n",
		node_name);
	return 0;
}

static void reserved_memory_exit(void)
{
	if (reserved_memory.virt_addr)
	{
		iounmap(reserved_memory.virt_addr);
		pr_info(
			"reserved_memory_exit: Unmapped reserved memory virtual "
			"address.\n");
		reserved_memory.virt_addr = NULL;
	}
	else
	{
		pr_warn("reserved_memory_exit: No reserved memory to unmap.\n");
	}
}

uint8_t xlnx_mbox_apu_wait_for_data(struct visp_dev *isp_dev, void *data)
{
	int result = 0;
	struct rpu_dev *rpu = NULL;

	if (!isp_dev)
	{
		pr_err(
			"xlnx_mbox_apu_wait_for_data: Invalid ISP device (NULL "
			"pointer).\n");
		return -EINVAL;
	}

	rpu = isp_dev->rpu;
	if (!rpu)
	{
		dev_err(isp_dev->dev, "RPU device is NULL in %s.\n", __func__);
		return -ENOMEM;
	}

	if (!data)
	{
		dev_err(isp_dev->dev, "Invalid data pointer in %s.\n", __func__);
		return -EINVAL;
	}

	/* Lock the data mutex */
	/* Wait for data to be available */
	result = wait_for_completion_interruptible(&isp_dev->apu_wait_for_data);
	if (result)
	{
		dev_err(isp_dev->dev, "Waiting for data interrupted. Error: %d\n",
				result);
		isp_dev->k_apu_data_flag = 0; // Clear the data flag
		//    mutex_unlock(&rpu->lock);
		mutex_unlock(&rpu->read_lock);
		return -EINTR; // Return specific error code for interrupted wait
	}

	/* Copy the received data */
	memcpy(data, data_from_interrupt.res_payload_pkt.payload,
		   sizeof(data_from_interrupt.res_payload_pkt.payload));

	/* Clear the data flag */
	isp_dev->k_apu_data_flag = 0;

	/* Unlock the data mutex */
	mutex_unlock(&rpu->read_lock);
	/* Return the error code from the interrupt's response payload */
	return data_from_interrupt.res_payload_pkt.resp_field.error_subcode_t;
}
EXPORT_SYMBOL(xlnx_mbox_apu_wait_for_data);

int xlnx_send_mbox_data_cmd(struct visp_dev *isp_dev, MBCmdId_E cmd,
			void *data, uint32_t size, uint8_t dest_cpu, uint8_t src_cpu)
{
	int result;
	struct rpu_dev *rpu;

	if (!isp_dev || !isp_dev->rpu || !data) {
		pr_err("%s: Invalid input parameters\n", __func__);
		return -EINVAL;
	}

	rpu = isp_dev->rpu;

	mutex_lock(&rpu->write_lock);

	result = Send_Command(cmd, data, size,get_dest_cpu(dest_cpu), src_cpu);
	if (result != 0) {
		pr_err("%s: Mailbox Send message failed at line %d\n", __func__, __LINE__);
		goto unlock_and_exit;
	}

	/* Set the data flag */
	isp_dev->k_apu_data_flag = 1;

	result = mbox_send_message(isp_dev->tx_chan, NULL);
	if (result < 0) {
		pr_err("%s: mbox_send_message failed at line %d\n", __func__, __LINE__);
		goto unlock_and_exit;
	}

	payload_packet *pkt = (payload_packet *)data;
	if (!pkt) {
		pr_err("%s: Payload data is NULL\n", __func__);
		result = -EINVAL;
		goto unlock_and_exit;
	}

	result = xlnx_mbox_apu_wait_for_data(isp_dev, pkt->payload);
	if (result) {
		pr_err("%s: Failed to get buffer data\n", __func__);
		goto unlock_and_exit;
	}

unlock_and_exit:
	mutex_unlock(&rpu->write_lock);
	return result;
}
EXPORT_SYMBOL(xlnx_send_mbox_data_cmd);


int xlnx_send_mbox_acked_cmd(struct visp_dev *isp_dev, MBCmdId_E cmd,
			void *data, uint32_t size, uint8_t dest_cpu, uint8_t src_cpu)
{
    int result;
	struct rpu_dev *rpu;

	if (!isp_dev || !isp_dev->rpu) {
		pr_err("%s: Invalid ISP device\n", __func__);
		return -EINVAL;
	}

	rpu = isp_dev->rpu;

	mutex_lock(&rpu->write_lock);

	result = Send_Command(cmd, data, size, get_dest_cpu(dest_cpu), src_cpu);
	if (result != 0) {
		pr_err("%s: Mailbox Send message failed at line %d\n", __func__, __LINE__);
		mutex_unlock(&rpu->write_lock);
		return result;
	}

	/* Set acknowledgment flag */
	isp_dev->k_apu_ack_flag = 1;

	result = mbox_send_message(isp_dev->tx_chan, NULL);
	if (result < 0) {
		pr_err("%s: mbox_send_message failed at line %d\n", __func__, __LINE__);
		mutex_unlock(&rpu->write_lock);
		return result;
	}

	xlnx_mbox_apu_wait_for_ack(isp_dev);

	mutex_unlock(&rpu->write_lock);

	return 0; // Success
}
EXPORT_SYMBOL(xlnx_send_mbox_acked_cmd);

void xlnx_mbox_apu_wait_for_ack(struct visp_dev *isp_dev)
{
	struct rpu_dev *rpu = NULL;
	int result;

	if (!isp_dev)
	{
		pr_err(
			"xlnx_mbox_apu_wait_for_ack: Invalid ISP device (NULL pointer).\n");
		return;
	}
	rpu = isp_dev->rpu;
	if (!rpu)
	{
		dev_err(isp_dev->dev, "RPU device is NULL in %s.\n", __func__);
		return;
	}

	/* Lock the acknowledgment mutex */
	//    mutex_lock(&rpu->ack_lock);


	//dev_err(isp_dev->dev, "%s %d acquired lock and waiting for ack to receive from rpu\n",__func__,__LINE__);
	/* Wait for acknowledgment completion */
	result = wait_for_completion_interruptible(&isp_dev->apu_wait_for_ack);
	if (result)
	{
		dev_err(isp_dev->dev,
				"Waiting for acknowledgment interrupted. Error: %d\n", result);
		isp_dev->k_apu_ack_flag = 0; // Clear the acknowledgment flag
		mutex_unlock(&rpu->read_lock);
		//      mutex_unlock(&rpu->ack_lock);
		return;
	}

	/* Clear acknowledgment flag */
	isp_dev->k_apu_ack_flag = 0;
	//mutex_unlock(&rpu->ack_lock);

	/* Unlock the acknowledgment mutex */
	mutex_unlock(&rpu->read_lock);
}
EXPORT_SYMBOL(xlnx_mbox_apu_wait_for_ack);

static int Mailbox_Initialization(struct rpu_dev *rpu)
{
	int ret;

	if (!rpu)
	{
		pr_err("Mailbox_Initialization: Invalid RPU device (NULL pointer).\n");
		return -EINVAL;
	}

	/* Initialize reserved memory */
	ret = reserved_memory_init("rpu0mbox0buffer");
	if (ret)
	{
		dev_err(rpu->dev, "Failed to initialize reserved memory. Error: %d\n",
				ret);
		return ret;
	}

	/* Initialize mailbox with reserved memory */
	mailbox_init(MBOX_CORE_APU, (uintptr_t)reserved_memory.virt_addr,
				 (uintptr_t)reserved_memory.phys_addr);

	/* Trigger an inter-processor interrupt (IPI) */
	ret = mbox_send_message(rpu->tx_chan, NULL);
	if (ret < 0)
	{
		dev_err(rpu->dev, "Failed to trigger IPI. Error: %d\n", ret);
		return ret;
	}

	return 0;
}

static void rpu_cleanup(struct kref *ref)
{
	/*    struct rpu_dev *rpu = container_of(ref, struct rpu_dev, refcount);
	 *        devm_kfree(&rpu->pdev->dev, rpu);*/
}
static int visp_mbox_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct rpu_dev *rpu = NULL;
	int ret;
	struct device_node *node;
	int rpu_id = -1;

	node = pdev->dev.of_node;

	/* Read the rpu_id property */
	ret = of_property_read_u32(node, "rpu_id", &rpu_id);
	if (ret)
	{
		dev_err(dev, " Failed to read rpu_id from device tree\n");
		return -EINVAL;
	}

	/* Log rpu_id for debug purposes */
	dev_dbg(dev, "rpu_id read from device tree: %u\n", rpu_id);

	/* Validate rpu_id if there is a known range */
	if (rpu_id < 0 || rpu_id >= MAX_RPU_ID)
	{ // Replace MAX_RPU_ID with your maximum allowed value
		dev_err(dev, "Invalid rpu_id: %d\n", rpu_id);
		return -EINVAL;
	}

	/* Find or create a new RPU with the given rpu_id */
	rpu = find_or_create_rpu(pdev, rpu_id);
	if (!rpu)
	{
		dev_err(dev, "Failed to find or create RPU with ID: %d\n", rpu_id);
		return -ENOMEM;
	}

	/* Store the RPU pointer in the platform device's driver data */
	platform_set_drvdata(pdev, rpu);

	/* Initialize the mailbox */
	ret = Mailbox_Initialization(rpu);
	if (ret)
	{
		dev_err(dev, "Failed to initialize mailbox for RPU ID: %d\n", rpu_id);
		platform_set_drvdata(pdev, NULL); // Clear driver data on failure
		return ret;
	}

	dev_info(dev, "Mailbox successfully initialized for RPU ID: %d\n", rpu_id);
	return 0;
}

static void rpu_remove(void)
{
	struct rpu_dev *rpu, *tmp;

	/* Lock the RPU list to prevent concurrent access */
	mutex_lock(&rpu_list_lock);

	/* Iterate through the list of RPU devices */
	list_for_each_entry_safe(rpu, tmp, &rpu_devices, node)
	{
		if (!rpu)
		{
			/* Handle NULL RPU device (shouldn't normally happen) */
			pr_err("Encountered a NULL RPU device in the list, skipping.\n");
			continue;
		}

		/* Debug log the current RPU's reference count */
		pr_debug("RPU ID %d - refcount = %d\n", rpu->rpu_id,
				 atomic_read(&rpu->refcount.refcount.refs));

		/* Check if the device is safe to remove */
		if (atomic_read(&rpu->refcount.refcount.refs) == 1)
		{
			/* Perform cleanup and removal */
			device_destroy(mailbox_class, rpu->devno);
			cdev_del(&rpu->cdev);
			unregister_chrdev_region(rpu->devno, 1);

			/* Free resources using the kref_put mechanism */
			kref_put(&rpu->refcount, rpu_cleanup);

			/* Remove the RPU from the list */
			list_del(&rpu->node);

			/* Log successful removal */
			pr_info("RPU with ID %d removed successfully.\n", rpu->rpu_id);
		}
		else
		{
			/* If still in use, decrement reference and log */
			kref_put(&rpu->refcount, rpu_cleanup);
			pr_warn(
				"Cannot remove RPU with ID %d - still in use (refcount = "
				"%d).\n",
				rpu->rpu_id, atomic_read(&rpu->refcount.refcount.refs));
		}
	}

	/* Unlock the RPU list */
	mutex_unlock(&rpu_list_lock);
}

static void visp_mbox_remove(struct platform_device *pdev)
{
	struct rpu_dev *rpu;

	/* Retrieve the RPU device associated with this platform device */
	rpu = platform_get_drvdata(pdev);
	if (!rpu)
	{
		dev_err(&pdev->dev, "Failed to retrieve RPU device during remove.\n");
		return;
	}

	/* Check and free mailbox channels if defined in the device tree */
	if (of_property_read_bool(rpu->dev->of_node, "mboxes"))
	{
		dev_info(rpu->dev, "Freeing mailbox channels.\n");
		if (rpu->tx_chan)
		{
			mbox_free_channel(rpu->tx_chan);
			rpu->tx_chan = NULL; // Avoid dangling pointers
		}
		if (rpu->rx_chan)
		{
			mbox_free_channel(rpu->rx_chan);
			rpu->rx_chan = NULL; // Avoid dangling pointers
		}
	}
	else
	{
		dev_dbg(rpu->dev, "No mailbox channels to free.\n");
	}

	/* Clean up reserved memory structure */
	reserved_memory_exit();
	dev_dbg(&pdev->dev, "Reserved memory cleaned up.\n");

	/* Call rpu_remove to handle RPU-specific cleanup */
	rpu_remove();

	return ;
}

static const struct of_device_id visp_mbox_of_match[] = {
	{
		.compatible = "xlnx,mbox",
	},
	{/* sentinel */},
};
static struct platform_driver visp_mbox_driver = {
	.probe = visp_mbox_probe,
	.remove = visp_mbox_remove,
	.driver = {
		.name = "visp_mbox_driver",
		.owner = THIS_MODULE,
		.of_match_table = visp_mbox_of_match,
	}};

static int __init visp_mbox_init_module(void)
{
	int ret;

	pr_info("Initializing AMD MBox driver.\n");

	/* Create the mailbox class */
	// mailbox_class = class_create(THIS_MODULE, "mailbox-class");
	mailbox_class = class_create("mailbox-class");
	if (IS_ERR(mailbox_class))
	{
		pr_err("Failed to create mailbox class.\n");
		return PTR_ERR(mailbox_class);
	}

	/* Register the platform driver */
	ret = platform_driver_register(&visp_mbox_driver);
	if (ret)
	{
		pr_err("Failed to register AMD MBox driver. Error: %d\n", ret);
		class_destroy(mailbox_class);
		return ret;
	}

	pr_info("AMD MBox driver registered successfully.\n");
	return 0;
}

static void __exit visp_mbox_exit_module(void)
{
	pr_info("Exiting AMD MBox driver.\n");

	/* Cleanup RPU devices */
	rpu_remove();

	/* Unregister the platform driver */
	platform_driver_unregister(&visp_mbox_driver);
	pr_info("AMD MBox driver unregistered successfully.\n");

	/* Destroy the mailbox class */
	if (mailbox_class)
	{
		class_destroy(mailbox_class);
		pr_info("Mailbox class destroyed successfully.\n");
	}
}

module_init(visp_mbox_init_module);
module_exit(visp_mbox_exit_module);

MODULE_DESCRIPTION("AMD MBOX driver");
MODULE_AUTHOR("Ajay Kumar Nandam anandam@amd.com");
MODULE_AUTHOR("Anil Mamidala amamidal@amd.com");
MODULE_LICENSE("GPL");
