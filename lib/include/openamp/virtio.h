/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * $FreeBSD$
 */

#ifndef _VIRTIO_H_
#define _VIRTIO_H_

#include <openamp/virtqueue.h>
#include <metal/errno.h>
#include <metal/spinlock.h>

#if defined __cplusplus
extern "C" {
#endif

/* VirtIO device IDs. */
#define VIRTIO_ID_NETWORK        1UL
#define VIRTIO_ID_BLOCK          2UL
#define VIRTIO_ID_CONSOLE        3UL
#define VIRTIO_ID_ENTROPY        4UL
#define VIRTIO_ID_BALLOON        5UL
#define VIRTIO_ID_IOMEMORY       6UL
#define VIRTIO_ID_RPMSG          7UL /* remote processor messaging */
#define VIRTIO_ID_SCSI           8UL
#define VIRTIO_ID_9P             9UL
#define VIRTIO_ID_MAC80211_WLAN  10UL
#define VIRTIO_ID_RPROC_SERIAL   11UL
#define VIRTIO_ID_CAIF           12UL
#define VIRTIO_ID_MEMORY_BALLOON 13UL
#define VIRTIO_ID_GPU            16UL
#define VIRTIO_ID_CLOCK          17UL
#define VIRTIO_ID_INPUT          18UL
#define VIRTIO_ID_VSOCK          19UL
#define VIRTIO_ID_CRYPTO         20UL
#define VIRTIO_ID_SIGNAL_DIST    21UL
#define VIRTIO_ID_PSTORE         22UL
#define VIRTIO_ID_IOMMU          23UL
#define VIRTIO_ID_MEM            24UL
#define VIRTIO_ID_SOUND          25UL
#define VIRTIO_ID_FS             26UL
#define VIRTIO_ID_PMEM           27UL
#define VIRTIO_ID_RPMB           28UL
#define VIRTIO_ID_MAC80211_HWSIM 29UL
#define VIRTIO_ID_VIDEO_ENCODER  30UL
#define VIRTIO_ID_VIDEO_DECODER  31UL
#define VIRTIO_ID_SCMI           32UL
#define VIRTIO_ID_NITRO_SEC_MOD  33UL
#define VIRTIO_ID_I2C_ADAPTER    34UL
#define VIRTIO_ID_WATCHDOG       35UL
#define VIRTIO_ID_CAN            36UL
#define VIRTIO_ID_PARAM_SERV     38UL
#define VIRTIO_ID_AUDIO_POLICY   39UL
#define VIRTIO_ID_BT             40UL
#define VIRTIO_ID_GPIO           41UL
#define VIRTIO_ID_RDMA           42UL
#define VIRTIO_DEV_ANY_ID        -1UL

/* Status byte for guest to report progress. */
#define VIRTIO_CONFIG_STATUS_RESET       0x00
#define VIRTIO_CONFIG_STATUS_ACK         0x01
#define VIRTIO_CONFIG_STATUS_DRIVER      0x02
#define VIRTIO_CONFIG_STATUS_DRIVER_OK   0x04
#define VIRTIO_CONFIG_FEATURES_OK        0x08
#define VIRTIO_CONFIG_STATUS_NEEDS_RESET 0x40
#define VIRTIO_CONFIG_STATUS_FAILED      0x80

/* Virtio device role */
#define VIRTIO_DEV_DRIVER	0UL
#define VIRTIO_DEV_DEVICE	1UL

#define VIRTIO_DEV_MASTER	deprecated_virtio_dev_master()
#define VIRTIO_DEV_SLAVE	deprecated_virtio_dev_slave()

__deprecated static inline int deprecated_virtio_dev_master(void)
{
	/* "VIRTIO_DEV_MASTER is deprecated, please use VIRTIO_DEV_DRIVER" */
	return VIRTIO_DEV_DRIVER;
}

__deprecated static inline int deprecated_virtio_dev_slave(void)
{
	/* "VIRTIO_DEV_SLAVE is deprecated, please use VIRTIO_DEV_DEVICE" */
	return VIRTIO_DEV_DEVICE;
}

#ifdef VIRTIO_MASTER_ONLY
#define VIRTIO_DRIVER_ONLY
#warning "VIRTIO_MASTER_ONLY is deprecated, please use VIRTIO_DRIVER_ONLY"
#endif

#ifdef VIRTIO_SLAVE_ONLY
#define VIRTIO_DEVICE_ONLY
#warning "VIRTIO_SLAVE_ONLY is deprecated, please use VIRTIO_DEVICE_ONLY"
#endif

struct virtio_device_id {
	uint32_t device;
	uint32_t vendor;
};

/*
 * Generate interrupt when the virtqueue ring is
 * completely used, even if we've suppressed them.
 */
#define VIRTIO_F_NOTIFY_ON_EMPTY (1 << 24)

/*
 * The guest should never negotiate this feature; it
 * is used to detect faulty drivers.
 */
#define VIRTIO_F_BAD_FEATURE (1 << 30)

/*
 * Some VirtIO feature bits (currently bits 28 through 31) are
 * reserved for the transport being used (eg. virtio_ring), the
 * rest are per-device feature bits.
 */
#define VIRTIO_TRANSPORT_F_START      28
#define VIRTIO_TRANSPORT_F_END        32

typedef void (*virtio_dev_reset_cb)(struct virtio_device *vdev);

struct virtio_dispatch;

struct virtio_feature_desc {
	uint32_t vfd_val;
	const char *vfd_str;
};

/**
 * struct virtio_vring_info
 * @vq virtio queue
 * @info vring alloc info
 * @notifyid vring notify id
 * @io metal I/O region of the vring memory, can be NULL
 */
struct virtio_vring_info {
	struct virtqueue *vq;
	struct vring_alloc_info info;
	uint32_t notifyid;
	struct metal_io_region *io;
};

/*
 * Structure definition for virtio devices for use by the
 * applications/drivers
 */

struct virtio_device {
	uint32_t notifyid; /**< unique position on the virtio bus */
	struct virtio_device_id id; /**< the device type identification
				      *  (used to match it with a driver
				      */
	uint64_t features; /**< the features supported by both ends. */
	unsigned int role; /**< if it is virtio backend or front end. */
	virtio_dev_reset_cb reset_cb; /**< user registered device callback */
	const struct virtio_dispatch *func; /**< Virtio dispatch table */
	void *priv; /**< TODO: remove pointer to virtio_device private data */
	unsigned int vrings_num; /**< number of vrings */
	struct virtio_vring_info *vrings_info;
};

/*
 * Helper functions.
 */
const char *virtio_dev_name(uint16_t devid);
void virtio_describe(struct virtio_device *dev, const char *msg,
		     uint32_t features,
		     struct virtio_feature_desc *feature_desc);

/*
 * Functions for virtio device configuration as defined in Rusty Russell's
 * paper.
 * Drivers are expected to implement these functions in their respective codes.
 */

struct virtio_dispatch {
	int (*create_virtqueues)(struct virtio_device *vdev,
				 unsigned int flags,
				 unsigned int nvqs, const char *names[],
				 vq_callback callbacks[]);
	void (*delete_virtqueues)(struct virtio_device *vdev);
	uint8_t (*get_status)(struct virtio_device *dev);
	void (*set_status)(struct virtio_device *dev, uint8_t status);
	uint32_t (*get_features)(struct virtio_device *dev);
	void (*set_features)(struct virtio_device *dev, uint32_t feature);
	uint32_t (*negotiate_features)(struct virtio_device *dev,
				       uint32_t features);

	/*
	 * Read/write a variable amount from the device specific (ie, network)
	 * configuration region. This region is encoded in the same endian as
	 * the guest.
	 */
	void (*read_config)(struct virtio_device *dev, uint32_t offset,
			    void *dst, int length);
	void (*write_config)(struct virtio_device *dev, uint32_t offset,
			     void *src, int length);
	void (*reset_device)(struct virtio_device *dev);
	void (*notify)(struct virtqueue *vq);
};

/**
 * @brief Create the virtio device virtqueue.
 *
 * @param vdev		Pointer to virtio device structure.
 * @param flags		Create flag.
 * @param nvqs		The virtqueue number.
 * @param names		Virtqueue names.
 * @param callbacks	Virtqueue callback functions.
 *
 * @return 0 on success, otherwise error code.
 */
int virtio_create_virtqueues(struct virtio_device *vdev, unsigned int flags,
			     unsigned int nvqs, const char *names[],
			     vq_callback callbacks[]);

/**
 * @brief Delete the virtio device virtqueue.
 *
 * @param vdev	Pointer to virtio device structure.
 *
 * @return 0 on success, otherwise error code.
 */
static inline int virtio_delete_virtqueues(struct virtio_device *vdev)
{
	if (!vdev)
		return -EINVAL;

	if (!vdev->func || !vdev->func->delete_virtqueues)
		return -ENXIO;

	vdev->func->delete_virtqueues(vdev);
	return 0;
}

/**
 * @brief Retrieve device status.
 *
 * @param dev		Pointer to device structure.
 * @param status	Pointer to the virtio device status.
 *
 * @return 0 on success, otherwise error code.
 */
static inline int virtio_get_status(struct virtio_device *vdev, uint8_t *status)
{
	if (!vdev || !status)
		return -EINVAL;

	if (!vdev->func || !vdev->func->get_status)
		return -ENXIO;

	*status = vdev->func->get_status(vdev);
	return 0;
}

/**
 * @brief Set device status.
 *
 * @param dev		Pointer to device structure.
 * @param status	Value to be set as device status.
 *
 * @return 0 on success, otherwise error code.
 */
static inline int virtio_set_status(struct virtio_device *vdev, uint8_t status)
{
	if (!vdev)
		return -EINVAL;

	if (!vdev->func || !vdev->func->set_status)
		return -ENXIO;

	vdev->func->set_status(vdev, status);
	return 0;
}

/**
 * @brief Retrieve configuration data from the device.
 *
 * @param dev		Pointer to device structure.
 * @param offset	Offset of the data within the configuration area.
 * @param dst		Address of the buffer that will hold the data.
 * @param len		Length of the data to be retrieved.
 *
 * @return 0 on success, otherwise error code.
 */
static inline int virtio_read_config(struct virtio_device *vdev,
				     uint32_t offset, void *dst, int len)
{
	if (!vdev || !dst)
		return -EINVAL;

	if (!vdev->func || !vdev->func->read_config)
		return -ENXIO;

	vdev->func->read_config(vdev, offset, dst, len);
	return 0;
}

/**
 * @brief Write configuration data to the device.
 *
 * @param dev		Pointer to device structure.
 * @param offset	Offset of the data within the configuration area.
 * @param src		Address of the buffer that holds the data to write.
 * @param len		Length of the data to be written.
 *
 * @return 0 on success, otherwise error code.
 */
static inline int virtio_write_config(struct virtio_device *vdev,
				      uint32_t offset, void *src, int len)
{
	if (!vdev || !src)
		return -EINVAL;

	if (!vdev->func || !vdev->func->write_config)
		return -ENXIO;

	vdev->func->write_config(vdev, offset, src, len);
	return 0;
}

/**
 * @brief Get the virtio device features.
 *
 * @param dev		Pointer to device structure.
 * @param features	Pointer to features supported by both the driver and
 *			the device as a bitfield.
 *
 * @return 0 on success, otherwise error code.
 */
static inline int virtio_get_features(struct virtio_device *vdev,
				      uint32_t *features)
{
	if (!vdev || !features)
		return -EINVAL;

	if (!vdev->func || !vdev->func->get_features)
		return -ENXIO;

	*features = vdev->func->get_features(vdev);
	return 0;
}

/**
 * @brief Set features supported by the VIRTIO driver.
 *
 * @param dev		Pointer to device structure.
 * @param features	Features supported by the driver as a bitfield.
 *
 * @return 0 on success, otherwise error code.
 */
static inline int virtio_set_features(struct virtio_device *vdev,
				      uint32_t features)
{
	if (!vdev)
		return -EINVAL;

	if (!vdev->func || !vdev->func->set_features)
		return -ENXIO;

	vdev->func->set_features(vdev, features);
	return 0;
}

/**
 * @brief Negotiate features between virtio device and driver.
 *
 * @param dev			Pointer to device structure.
 * @param features		Supported features.
 * @param final_features	Pointer to the final features after negotiate.
 *
 * @return 0 on success, otherwise error code.
 */
static inline int virtio_negotiate_features(struct virtio_device *vdev,
					    uint32_t features,
					    uint32_t *final_features)
{
	if (!vdev || !final_features)
		return -EINVAL;

	if (!vdev->func || !vdev->func->negotiate_features)
		return -ENXIO;

	*final_features = vdev->func->negotiate_features(vdev, features);
	return 0;
}

/**
 * @brief Reset virtio device.
 *
 * @param vdev	Pointer to virtio_device structure.
 *
 * @return 0 on success, otherwise error code.
 */
static inline int virtio_reset_device(struct virtio_device *vdev)
{
	if (!vdev)
		return -EINVAL;

	if (!vdev->func || !vdev->func->reset_device)
		return -ENXIO;

	vdev->func->reset_device(vdev);
	return 0;
}

#if defined __cplusplus
}
#endif

#endif				/* _VIRTIO_H_ */
