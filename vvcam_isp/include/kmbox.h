#ifndef __KMBOX__
#define  __KMBOX__
#include <linux/mailbox_controller.h>
//int xlnx_mailbox_send_msg_client(void *mssg, MBCmdId_E cmd_id);
#if 1
/**
 * struct mbox_chan - s/w representation of a communication chan
 * @mbox:       Pointer to the parent/provider of this channel
 * @txdone_method:  Way to detect TXDone chosen by the API
 * @cl:         Pointer to the current owner of this channel
 * @tx_complete:    Transmission completion
 * @active_req:     Currently active request hook
 * @msg_count:      No. of mssg currently queued
 * @msg_free:       Index of next available mssg slot
 * @msg_data:       Hook for data packet
 * @lock:       Serialise access to the channel
 * @con_priv:       Hook for controller driver to attach private data
 */
#if 0
struct mbox_chan {
    struct mbox_controller *mbox;
    unsigned txdone_method;
    struct mbox_client *cl;
    struct completion tx_complete;
    void *active_req;
    unsigned msg_count, msg_free;
    void *msg_data[MBOX_TX_QUEUE_LEN];
    spinlock_t lock; /* Serialise access to the channel */
    void *con_priv;
};
#endif

int xlnx_mailbox_send_msg_client(struct mbox_chan *chan,void *mssg, MBCmdId_E cmd_id);
uint8_t xlnx_mbox_apu_wait_for_data(void *data);
void xlnx_mbox_apu_wait_for_ack(void);
int Send_Command(MBCmdId_E cmd, void *data, uint32_t size, uint8_t dest_cpu, uint8_t src_cpu );
#endif
/**
 *  * @brief Enum structure of Mbox core id
 *   */
#if 0
typedef enum MboxCoreId {
    MBOX_CORE_RPU0,
        MBOX_CORE_RPU1,  
        MBOX_CORE_APU,
    MBOX_CORE_MAX
} MboxCoreId;
#endif
#if 0
#include "mbox_cmd.h"
#include "mbox_error_code.h"
#include "mbox_hardware.h"
#include "xlnx_isp_mailbox.h"
#endif

#endif
