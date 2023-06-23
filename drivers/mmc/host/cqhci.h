/* SPDX-License-Identifier: GPL-2.0-only */
/* Copyright (c) 2015-2021, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#ifndef LINUX_MMC_CQHCI_H
#define LINUX_MMC_CQHCI_H

#include <linux/compiler.h>
#include <linux/bitops.h>
#include <linux/spinlock_types.h>
#include <linux/types.h>
#include <linux/completion.h>
#include <linux/wait.h>
#include <linux/irqreturn.h>
#include <asm/io.h>
#include <linux/keyslot-manager.h>

/* registers */
/* version */
#define CQHCI_VER			0x00
#define CQHCI_VER_MAJOR(x)		(((x) & GENMASK(11, 8)) >> 8)
#define CQHCI_VER_MINOR1(x)		(((x) & GENMASK(7, 4)) >> 4)
#define CQHCI_VER_MINOR2(x)		((x) & GENMASK(3, 0))

/* capabilities */
#define CQHCI_CAP			0x04
#define CQHCI_CAP_CS			(1 << 28)
#define CQHCI_CCAP			0x100
#define CQHCI_CRYPTOCAP			0x104

/* configuration */
#define CQHCI_CFG			0x08
#define CQHCI_DCMD			0x00001000
#define CQHCI_TASK_DESC_SZ		0x00000100
#define CQHCI_ENABLE			0x00000001
#define CQHCI_ICE_ENABLE		0x00000002

/* control */
#define CQHCI_CTL			0x0C
#define CQHCI_CLEAR_ALL_TASKS		0x00000100
#define CQHCI_HALT			0x00000001

/* interrupt status */
#define CQHCI_IS			0x10
#define CQHCI_IS_HAC			BIT(0)
#define CQHCI_IS_TCC			BIT(1)
#define CQHCI_IS_RED			BIT(2)
#define CQHCI_IS_TCL			BIT(3)
#define CQHCI_IS_GCE			BIT(4)
#define CQHCI_IS_ICCE			BIT(5)

#define CQHCI_IS_MASK (CQHCI_IS_TCC | CQHCI_IS_RED | \
			CQHCI_IS_GCE | CQHCI_IS_ICCE)

/* interrupt status enable */
#define CQHCI_ISTE			0x14

/* interrupt signal enable */
#define CQHCI_ISGE			0x18

/* interrupt coalescing */
#define CQHCI_IC			0x1C
#define CQHCI_IC_ENABLE			BIT(31)
#define CQHCI_IC_RESET			BIT(16)
#define CQHCI_IC_ICCTHWEN		BIT(15)
#define CQHCI_IC_ICCTH(x)		(((x) & 0x1F) << 8)
#define CQHCI_IC_ICTOVALWEN		BIT(7)
#define CQHCI_IC_ICTOVAL(x)		((x) & 0x7F)

/* task list base address */
#define CQHCI_TDLBA			0x20

/* task list base address upper */
#define CQHCI_TDLBAU			0x24

/* door-bell */
#define CQHCI_TDBR			0x28

/* task completion notification */
#define CQHCI_TCN			0x2C

/* device queue status */
#define CQHCI_DQS			0x30

/* device pending tasks */
#define CQHCI_DPT			0x34

/* task clear */
#define CQHCI_TCLR			0x38

/* send status config 1 */
#define CQHCI_SSC1			0x40

/* send status config 2 */
#define CQHCI_SSC2			0x44

/*
 * Value n means CQE would send CMD13 during the transfer of data block
 * BLOCK_CNT-n
 */
#define SEND_QSR_INTERVAL 0x70001

/* response for dcmd */
#define CQHCI_CRDCT			0x48

/* response mode error mask */
#define CQHCI_RMEM			0x50

/* task error info */
#define CQHCI_TERRI			0x54

#define CQHCI_TERRI_C_INDEX(x)		((x) & GENMASK(5, 0))
#define CQHCI_TERRI_C_TASK(x)		(((x) & GENMASK(12, 8)) >> 8)
#define CQHCI_TERRI_C_VALID(x)		((x) & BIT(15))
#define CQHCI_TERRI_D_INDEX(x)		(((x) & GENMASK(21, 16)) >> 16)
#define CQHCI_TERRI_D_TASK(x)		(((x) & GENMASK(28, 24)) >> 24)
#define CQHCI_TERRI_D_VALID(x)		((x) & BIT(31))

/* command response index */
#define CQHCI_CRI			0x58

/* command response argument */
#define CQHCI_CRA			0x5C

/*
 * Add new macro for updated CQ vendor specific
 * register address for SDHC v5.0 onwards.
 */
#define CQE_V5_VENDOR_CFG	0x900
#define CQHCI_VENDOR_CFG   0x100
#define CMDQ_SEND_STATUS_TRIGGER (1 << 31)

#define CQHCI_INT_ALL			0xF
#define CQHCI_IC_DEFAULT_ICCTH		31
#define CQHCI_IC_DEFAULT_ICTOVAL	1

/* attribute fields */
#define CQHCI_VALID(x)			(((x) & 1) << 0)
#define CQHCI_END(x)			(((x) & 1) << 1)
#define CQHCI_INT(x)			(((x) & 1) << 2)
#define CQHCI_ACT(x)			(((x) & 0x7) << 3)

/* data command task descriptor fields */
#define CQHCI_FORCED_PROG(x)		(((x) & 1) << 6)
#define CQHCI_CONTEXT(x)		(((x) & 0xF) << 7)
#define CQHCI_DATA_TAG(x)		(((x) & 1) << 11)
#define CQHCI_DATA_DIR(x)		(((x) & 1) << 12)
#define CQHCI_PRIORITY(x)		(((x) & 1) << 13)
#define CQHCI_QBAR(x)			(((x) & 1) << 14)
#define CQHCI_REL_WRITE(x)		(((x) & 1) << 15)
#define CQHCI_BLK_COUNT(x)		(((x) & 0xFFFF) << 16)
#define CQHCI_BLK_ADDR(x)		(((x) & 0xFFFFFFFF) << 32)

/* direct command task descriptor fields */
#define CQHCI_CMD_INDEX(x)		(((x) & 0x3F) << 16)
#define CQHCI_CMD_TIMING(x)		(((x) & 1) << 22)
#define CQHCI_RESP_TYPE(x)		(((x) & 0x3) << 23)

/* transfer descriptor fields */
#define CQHCI_DAT_LENGTH(x)		(((x) & 0xFFFF) << 16)
#define CQHCI_DAT_ADDR_LO(x)		(((x) & 0xFFFFFFFF) << 32)
#define CQHCI_DAT_ADDR_HI(x)		(((x) & 0xFFFFFFFF) << 0)
#define DATA_UNIT_NUM(x)		(((u64)(x) & 0xFFFFFFFF) << 0)
#define CRYPTO_CONFIG_INDEX(x)		(((u64)(x) & 0xFF) << 32)
#define CRYPTO_ENABLE(x)		(((u64)(x) & 0x1) << 47)

/* ICE context is present in the upper 64bits of task descriptor */
#define CQHCI_TASK_DESC_ICE_PARAM_OFFSET	8
/* ICE descriptor size */
#define CQHCI_TASK_DESC_ICE_PARAMS_SIZE		8

struct cqhci_host_ops;
struct mmc_host;
struct cqhci_slot;
struct cqhci_host;

/* CCAP - Crypto Capability 100h */
union cqhci_crypto_capabilities {
	__le32 reg_val;
	struct {
		u8 num_crypto_cap;
		u8 config_count;
		u8 reserved;
		u8 config_array_ptr;
	};
};

enum cqhci_crypto_key_size {
	CQHCI_CRYPTO_KEY_SIZE_INVALID	= 0x0,
	CQHCI_CRYPTO_KEY_SIZE_128	= 0x1,
	CQHCI_CRYPTO_KEY_SIZE_192	= 0x2,
	CQHCI_CRYPTO_KEY_SIZE_256	= 0x3,
	CQHCI_CRYPTO_KEY_SIZE_512	= 0x4,
};

enum cqhci_crypto_alg {
	CQHCI_CRYPTO_ALG_AES_XTS		= 0x0,
	CQHCI_CRYPTO_ALG_BITLOCKER_AES_CBC	= 0x1,
	CQHCI_CRYPTO_ALG_AES_ECB		= 0x2,
	CQHCI_CRYPTO_ALG_ESSIV_AES_CBC		= 0x3,
};

/* x-CRYPTOCAP - Crypto Capability X */
union cqhci_crypto_cap_entry {
	__le32 reg_val;
	struct {
		u8 algorithm_id;
		u8 sdus_mask; /* Supported data unit size mask */
		u8 key_size;
		u8 reserved;
	};
};

#define CQHCI_CRYPTO_CONFIGURATION_ENABLE (1 << 7)
#define CQHCI_CRYPTO_KEY_MAX_SIZE 64
/* x-CRYPTOCFG - Crypto Configuration X */
union cqhci_crypto_cfg_entry {
	__le32 reg_val[32];
	struct {
		u8 crypto_key[CQHCI_CRYPTO_KEY_MAX_SIZE];
		u8 data_unit_size;
		u8 crypto_cap_idx;
		u8 reserved_1;
		u8 config_enable;
		u8 reserved_multi_host;
		u8 reserved_2;
		u8 vsb[2];
		u8 reserved_3[56];
	};
};

struct cqhci_host_crypto_variant_ops {
	void (*setup_rq_keyslot_manager)(struct cqhci_host *host,
					 struct request_queue *q);
	void (*destroy_rq_keyslot_manager)(struct cqhci_host *host,
					   struct request_queue *q);
#ifdef CONFIG_BLK_INLINE_ENCRYPTION
	int (*host_init_crypto)(struct cqhci_host *host,
				const struct keyslot_mgmt_ll_ops *ksm_ops);
#endif
	void (*enable)(struct cqhci_host *host);
	void (*disable)(struct cqhci_host *host);
	int (*suspend)(struct cqhci_host *host);
	int (*resume)(struct cqhci_host *host);
	int (*debug)(struct cqhci_host *host);
	int (*prepare_crypto_desc)(struct cqhci_host *host,
				   struct mmc_request *mrq,
				   u64 *ice_ctx);
	int (*complete_crypto_desc)(struct cqhci_host *host,
				    struct mmc_request *mrq,
				    u64 *ice_ctx);
	int (*reset)(struct cqhci_host *host);
	int (*recovery_finish)(struct cqhci_host *host);
	int (*program_key)(struct cqhci_host *host,
			   const union cqhci_crypto_cfg_entry *cfg,
			   int slot);
	void *priv;
};

struct cqhci_host {
	const struct cqhci_host_ops *ops;
	void __iomem *mmio;
	void __iomem *icemmio;
	struct mmc_host *mmc;

	spinlock_t lock;

	/* relative card address of device */
	unsigned int rca;

	/* 64 bit DMA */
	bool dma64;
	int num_slots;
	int qcnt;

	u32 dcmd_slot;
	u32 caps;
#define CQHCI_TASK_DESC_SZ_128		0x1
#define CQHCI_CAP_CRYPTO_SUPPORT	0x2

	u32 quirks;
#define CQHCI_QUIRK_SHORT_TXFR_DESC_SZ	0x1

	bool enabled;
	bool halted;
	bool init_done;
	bool activated;
	bool waiting_for_idle;
	bool recovery_halt;
	bool offset_changed;

	size_t desc_size;
	size_t data_size;

	u8 *desc_base;

	/* total descriptor size */
	u8 slot_sz;

	/* 64/128 bit depends on CQHCI_CFG */
	u8 task_desc_len;

	/* 64 bit on 32-bit arch, 128 bit on 64-bit */
	u8 link_desc_len;

	u8 *trans_desc_base;
	/* same length as transfer descriptor */
	u8 trans_desc_len;

	dma_addr_t desc_dma_base;
	dma_addr_t trans_desc_dma_base;

	struct completion halt_comp;
	wait_queue_head_t wait_queue;
	struct cqhci_slot *slot;
	const struct cqhci_host_crypto_variant_ops *crypto_vops;

#ifdef CONFIG_MMC_CQHCI_CRYPTO
	union cqhci_crypto_capabilities crypto_capabilities;
	union cqhci_crypto_cap_entry *crypto_cap_array;
	u32 crypto_cfg_register;
#ifdef CONFIG_BLK_INLINE_ENCRYPTION
	struct keyslot_manager *ksm;
#endif /* CONFIG_BLK_INLINE_ENCRYPTION */
#endif /* CONFIG_SCSI_CQHCI_CRYPTO */
};

struct cqhci_host_ops {
	void (*dumpregs)(struct mmc_host *mmc);
	void (*write_l)(struct cqhci_host *host, u32 val, int reg);
	u32 (*read_l)(struct cqhci_host *host, int reg);
	void (*enable)(struct mmc_host *mmc);
	void (*disable)(struct mmc_host *mmc, bool recovery);
	void (*enhanced_strobe_mask)(struct mmc_host *mmc, bool set);
};

static inline void cqhci_writel(struct cqhci_host *host, u32 val, int reg)
{
	if (unlikely(host->ops && host->ops->write_l))
		host->ops->write_l(host, val, reg);
	else
		writel_relaxed(val, host->mmio + reg);
}

static inline u32 cqhci_readl(struct cqhci_host *host, int reg)
{
	if (unlikely(host->ops && host->ops->read_l))
		return host->ops->read_l(host, reg);
	else
		return readl_relaxed(host->mmio + reg);
}

struct platform_device;

irqreturn_t cqhci_irq(struct mmc_host *mmc, u32 intmask, int cmd_error,
		      int data_error);
int cqhci_init(struct cqhci_host *cq_host, struct mmc_host *mmc, bool dma64);
struct cqhci_host *cqhci_pltfm_init(struct platform_device *pdev);
int cqhci_suspend(struct mmc_host *mmc);
int cqhci_resume(struct mmc_host *mmc);

#endif
