/*
 * Copyright (c) 2015-2016 MICROTRUST Incorporated
 * All rights reserved
 *
 * This file and software is confidential and proprietary to MICROTRUST Inc.
 * Unauthorized copying of this file and software is strictly prohibited.
 * You MUST NOT disclose this file and software unless you get a license
 * agreement from MICROTRUST Incorporated.
 */

#include "teei_list.h"

#define RPMB_IOCTL_SOTER_WRITE_DATA 5
#define RPMB_IOCTL_SOTER_READ_DATA 6
#define RPMB_IOCTL_SOTER_GET_CNT 7

#define RPMB_BUFF_SIZE 512
#define PAGE_SIZE_4K (0x1000)

extern int handle_vfs_command(int vfs_fd, unsigned char* rw_buffer, pthread_mutex_t *property_list_mutex, sem_t *setting_sem, struct teei_list *property_list, unsigned int *unlock_state);
extern void read_property(void);
