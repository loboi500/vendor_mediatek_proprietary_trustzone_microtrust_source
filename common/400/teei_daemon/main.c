/*
 * Copyright (c) 2015-2016 MICROTRUST Incorporated
 * All rights reserved
 *
 * This file and software is confidential and proprietary to MICROTRUST Inc.
 * Unauthorized copying of this file and software is strictly prohibited.
 * You MUST NOT disclose this file and software unless you get a license
 * agreement from MICROTRUST Incorporated.
 */

#define _LARGEFILE64_SOURCE 1
#define _FILE_OFFSET_BITS 64

#define LOG_TAG "[mTEE]"
#include <cutils/properties.h>
#include <errno.h>
#include <fcntl.h>
#include <malloc.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/ioctl.h>
#include <stdbool.h>
#include <sys/mman.h>
#include <time.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <log/log.h>
#include <dirent.h>
#include <linux/types.h>
#include <semaphore.h>
#include "utos_version.h"
#include "TEEI.h"
#include "teei_list.h"

#define TEEI_IOC_MAGIC 'T'
#define TEEI_CONFIG_IOCTL_INIT_TEEI _IOWR(TEEI_IOC_MAGIC, 3, struct init_param)
#define TEEI_CONFIG_IOCTL_UNLOCK _IOWR(TEEI_IOC_MAGIC, 4, int)
#define SOTER_TUI_ENTER _IOWR(TEEI_IOC_MAGIC, 0x70, int)
#define SOTER_TUI_LEAVE _IOWR(TEEI_IOC_MAGIC, 0x71, int)
#define TEEI_VFS_NOTIFY_DRM _IOWR(TEEI_IOC_MAGIC, 0x75, int)
#define BUFFER_SIZE (512 * 1024)
#define DEV_FILE "/dev/tz_vfs"

#define PERSIST_RPMB_PATH "/persist/rpmb/rpmb.txt"
#define VENDOR_PERSIST_RPMB_PATH "/vendor/persist/rpmb/rpmb.txt"
#define MNT_PERSIST_RPMB_PATH "/mnt/vendor/persist/rpmb/rpmb.txt"
#define RPMB_PATH "/data/vendor/thh/system/rpmb.txt"
#define SETTING_PROPKEY_HEAD "vendor.soter.teei."

#define MAX_DRV_UUIDS 30
#define UUID_LEN 32
#define REAL_DRV_UUIDS 1

/* #define SAVE_TEE_LOG */

struct init_param {
    char uuids[MAX_DRV_UUIDS][UUID_LEN+1];
    __u32 uuid_count;
    __u32 flag;
};

struct teei_property_struct {
	char *path;
	char *property_context;
	struct teei_list head;
};

static unsigned int unlock_state = 0;
static pthread_mutex_t property_list_mutex;

char drv_uuid[REAL_DRV_UUIDS][UUID_LEN+1] = {
    {"93feffccd8ca11e796c7c7a21acb4932"}
};

struct init_param init_param;

sem_t g_setting_sem;
struct teei_list g_property_list;

int property_setting_fn(void)
{
	int retVal = 0;
	struct teei_property_struct *entry = NULL;

	while (1) {
		retVal = sem_wait(&g_setting_sem);
		if (retVal != 0)
			continue;

		entry = list_first_entry(&g_property_list,
					struct teei_property_struct, head);

		if (entry == NULL) {
			ALOGE("g_property_list is NULL\n");
			continue;
		}

		if (pthread_mutex_lock(&property_list_mutex) != 0) {
			ALOGE("TEEI: Failed to lock property_list_mutex!\n");
			return -1;
		}

		teei_list_del(&(entry->head));

		pthread_mutex_unlock(&property_list_mutex);

		property_set(entry->path, entry->property_context);

		free(entry->path);
		free(entry->property_context);
		free(entry);

	}

	return 0;

}

#ifdef SAVE_TEE_LOG

#define MAX_LOG_SIZE	4096
#define MAX_NAME_SIZE	100
#define MAX_LOG_CNT	5
#define MAX_FILE_SIZE  (2*1024*1024)

void rename_last_log(void)
{
	int retVal = 0;
	char from_name[100];
	char to_name[100];
	int i = 0;
	char *log_path = "/data/vendor/thh/tee_log";

	for (i = MAX_LOG_CNT; i > 1; i--) {
		memset(from_name, 0, MAX_NAME_SIZE);
		memset(to_name, 0, MAX_NAME_SIZE);

		sprintf(from_name, "%s/teei_log_LAST_%d", log_path, i - 1);

		retVal = access(from_name, F_OK);
		if (retVal == 0) {
			sprintf(to_name, "%s/teei_log_LAST_%d", log_path, i);
			ALOGI("from name%s, to name:%s",from_name, to_name );
			rename(from_name, to_name);
		}
	}

	memset(from_name, 0, MAX_NAME_SIZE);
	memset(to_name, 0, MAX_NAME_SIZE);

	sprintf(from_name, "%s/teei_log_current", log_path);

	retVal = access(from_name, F_OK);
	if (retVal == 0) {
		sprintf(to_name, "%s/teei_log_LAST_1", log_path);
		rename(from_name, to_name);
	}
}

int get_tee_log(int config_fd)
{
	int log_file_fd = 0;
	unsigned char log_context[MAX_LOG_SIZE];
	char log_file_date[100];
	struct tm *timeinfo;
	time_t nowtime;
	long read_len;
	long write_len;
	int year,month,day,hour,min,sec;
	int retVal;
	unsigned int total_wr_len;

retry_access_dir:
	time(&nowtime);
	timeinfo = localtime(&nowtime);


	year = timeinfo->tm_year + 1900;
	month = timeinfo->tm_mon + 1;
	day = timeinfo->tm_mday;
	hour = timeinfo->tm_hour;
	min = timeinfo->tm_min;
	sec = timeinfo->tm_sec;

	retVal = access("/data/vendor/thh/tee_log", F_OK);
	if (retVal != 0) {
		ALOGE("Create the /data/vendor/thh/tee_log !\n");
		usleep(1000);
		goto retry_access_dir;
	}

	rename_last_log();


	/* Write the log to the teei_log_current */
	memset(log_file_date, 0, sizeof(log_file_date));

	sprintf(log_file_date, ">>>>> Date: %4d%02d%02d_%02d%02d%02d",
				year, month, day, hour, min, sec);

	ALOGI("log_file_date = %s\n", log_file_date);

retry_open_file:
	log_file_fd = open("/data/vendor/thh/tee_log/teei_log_current",
				O_RDWR | O_SYNC | O_CREAT, S_IRWXU);

	if (log_file_fd < 0) {
		ALOGE("open the current file failed ret = %d\n", log_file_fd);
		usleep(1000);
		goto retry_open_file;
	}

	write(log_file_fd, log_file_date, strlen(log_file_date));
	write(log_file_fd, " <<<<<\n\n\n", sizeof(" <<<<<\n\n\n"));

	total_wr_len = 0;

	while (1) {
		read_len = read(config_fd, log_context, MAX_LOG_SIZE);
		if (read_len < 0) {
			ALOGE("%s Failed to read the log ret = %ld\n", __func__, read_len);
			continue;
		}
		write_len = write(log_file_fd, log_context, read_len);
		if (write_len < 0) {
			ALOGE("%s Failed to write the log ret = %ld\n", __func__, write_len);
			continue;
		}

		total_wr_len += write_len;
		if (total_wr_len >= MAX_FILE_SIZE) {
			close(log_file_fd);
			goto retry_access_dir;
		}
	}

	return 0;
}
#endif

static int init_soter_fn(void)
{
    int fd = 0;
    __u32 i;

    fd = open("/dev/teei_config", O_RDWR);
    if (fd < 0) {
            ALOGE("[%s-%d] open node failed, fd = %d\n", __func__,__LINE__,fd);
            return -1;
    }
    ALOGI("come in init soter thread");

    ALOGD("driver to be loaded:\n");
    for (i = 0; i < init_param.uuid_count; i++) {
        ALOGD("UUID=%s\n", init_param.uuids[i]);
    }

    int ret = ioctl(fd, TEEI_CONFIG_IOCTL_INIT_TEEI, &init_param);
    if (ret) {
        ALOGE("can't init soter ,ret = %d !", ret);

    } else {
        ALOGI("init soter finish. flag = %d", init_param.flag);
        property_set("vendor.soter.teei.init", "INIT_OK");
    }

#ifdef SAVE_TEE_LOG
	ret = get_tee_log(fd);
#endif

    close(fd);

    return 0;
}

/****************************KEYMASTER FUNCTIONS***********************/
/* for unlocking keymaster,and then can load tee in (kernel/tz_driver) */

static void keymaster_unlock(void)
{
    int fd = 0;
    int flag = 0;

    fd = open("/dev/teei_config", O_RDWR);
    if (fd < 0) {
            ALOGE("!!keymaster open nod failed, fd = %d!!\n",fd);
            return;
    }
    int ret = ioctl(fd, TEEI_CONFIG_IOCTL_UNLOCK, &flag);
    if (ret) {
        ALOGE("keymaster unlock boot_decrypt_lock failed!\n");
    } else {
        ALOGI("keymaster unlock boot_decrypt_lock success\n");
    }
    close(fd);
}

/* for the system first time boot, then the keymaster can store the key
 * in rpmb block 38, this rpmb place is just for all disk encrypt rsa key */

static int load_tee_fn(void)
{
    ALOGI("come in load tee thread");
    char value[PROPERTY_VALUE_MAX] = {0};
    char value_type[PROPERTY_VALUE_MAX] = {0};
    char vendor_value_state[PROPERTY_VALUE_MAX] = {0}; //used for meta and factory first time start up

    property_get("ro.crypto.state", value, "");
    property_get("ro.crypto.type", value_type, "");

    if (strcmp("unencrypted", value) != 0 && strcmp("unsupported", value) != 0)
    {
        /*data encrypted, wait for decrption.*/
        if(strcmp("file", value_type) != 0) {
            property_get("vold.decrypt", value, "");
            while (strcmp("trigger_restart_framework", value) != 0) {

                property_get("vendor.soter.teei.crypto.state", vendor_value_state, "");
                if (strcmp("unencrypted", vendor_value_state) == 0) {
                    ALOGI("meta or factory firsttime start up");
                    break;
                }

                memset(value_type, 0, PROPERTY_VALUE_MAX);
                property_get("ro.crypto.state", value_type, "");
                if (strcmp("unencrypted", value_type) == 0 || strcmp("unsupported", value_type) == 0) {
                    ALOGI("unencrypted late");
                    break;
                }

                memset(value_type, 0, PROPERTY_VALUE_MAX);
                property_get("ro.crypto.type", value_type, "");
                if (strcmp("file", value_type) == 0) {
                    ALOGI("fbe mode late");
                    break;
                }

                /*still decrypting... wait one second.*/
                usleep(10000);
                property_get("vold.decrypt", value, "");
            }
        }
    }

	ALOGI("daemon unlock keymaster signal to tz-driver");
    keymaster_unlock();

    unlock_state = 1;

    return 0;
}
/****************************MAIN THREAD ******************************/
int main(int argc, char** argv) {

    int vfs_fd = 0;
    unsigned char* rw_buffer = NULL;
    int opt;
    int i = 0;

    pthread_t ntid = 0;
    pthread_t loadtee_id = 0;
    pthread_t property_setting_id = 0;

    ALOGI("daemon version [%s]", UTOS_VERSION);
    property_set("vendor.soter.teei.init", "INIT_START");

    memset((void *)&init_param, 0, sizeof(struct init_param));

    rw_buffer = (unsigned char*)malloc(BUFFER_SIZE);
    if (rw_buffer == NULL) {
        ALOGE("daemon can not malloc enough memory\n");
        return -1;
    }
    init_param.flag = 0;
    while ((opt = getopt(argc, argv, "r:t:h")) != -1) {
        switch (opt) {
        case 'r':
#if !defined(MTK_DRM_SUPPORT) && !defined(MTK_CAM_SUPPORT) && !defined(MTK_SDSP_SUPPORT)
            break;
#endif
            if (strlen(optarg) != UUID_LEN) {
                ALOGE("UUID length should be %d (uuid=%s)\n", UUID_LEN, optarg);
                exit(EXIT_FAILURE);
            }
            if(init_param.uuid_count >= MAX_DRV_UUIDS)
            {
                ALOGE("Too Much TA Boot -- MAX_DRV_UUIDS[%d]\n", MAX_DRV_UUIDS);
                break;
            }
            memcpy((void*)init_param.uuids[init_param.uuid_count], optarg, UUID_LEN);
            init_param.flag = init_param.flag & (~(0x1 << init_param.uuid_count));
            init_param.uuid_count++;
            break;
        case  't':
#if !defined(MTK_DRM_SUPPORT) && !defined(MTK_CAM_SUPPORT) && !defined(MTK_SDSP_SUPPORT)
            break;
#endif
            if (strlen(optarg) != UUID_LEN) {
                ALOGE("UUID length should be %d (uuid=%s)\n", UUID_LEN, optarg);
                exit(EXIT_FAILURE);
            }

            if(init_param.uuid_count >= MAX_DRV_UUIDS)
            {
                ALOGE("Too Much TA Boot -- MAX_DRV_UUIDS[%d]\n", MAX_DRV_UUIDS);
                break;
            }
            memcpy((void*)init_param.uuids[init_param.uuid_count], optarg, UUID_LEN);
	    ALOGI("[%s][%d]TA UUID[%s]\n", __func__, __LINE__, init_param.uuids[init_param.uuid_count]);
            init_param.flag = init_param.flag | (0x1 << init_param.uuid_count);
            init_param.uuid_count++;
            break;

        case 'h':
        default:
            fprintf(stderr, "Usage: %s [-h|-r|-t <UUID>]\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }

#ifndef MIN_MEM_SUPPORT
    for (i = 0; i < REAL_DRV_UUIDS; i++) {
        if (strlen(drv_uuid[i]) != UUID_LEN) {
            ALOGE("UUID length should be %d, (drv_uuid[%d]=%s)\n", UUID_LEN, i, drv_uuid[i]);
            break;
        }
        memcpy((void*)init_param.uuids[init_param.uuid_count], drv_uuid[i], UUID_LEN);
        init_param.uuid_count++;
    }
#endif
    while (1)
    {
        vfs_fd = open(DEV_FILE, O_RDWR);
        if (vfs_fd < 0) {
            ALOGE("daemon wait open the vfs node %d\n", vfs_fd);
            continue;
        }
        break;
    }

    INIT_LIST_HEAD(&g_property_list);
    sem_init(&g_setting_sem, 0, 0);

    if (pthread_mutex_init(&property_list_mutex, NULL) != 0) {
        ALOGE("TEEI: Failed to init property_list_mutex\n");
        return -1;
    }

    read_property();
    ALOGD("prepare property finish\n");
    /*create a thread for start data area working*/
    pthread_create(&ntid, NULL, (void *)init_soter_fn, NULL);
    pthread_create(&loadtee_id, NULL, (void *)load_tee_fn, NULL);
    pthread_create(&property_setting_id, NULL,
				(void *)property_setting_fn, NULL);

    while (1)
    {
        handle_vfs_command(vfs_fd, rw_buffer, &property_list_mutex, &g_setting_sem, &g_property_list, &unlock_state);
    }

    free(rw_buffer);
    close(vfs_fd);

    ALOGI("daemon start OK ...");
    return 0;
}
