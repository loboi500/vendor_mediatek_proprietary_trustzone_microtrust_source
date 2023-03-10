/*
 * Copyright (C) 2015 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "beanpodkeymaster"

// TODO: make this generic in lib

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/uio.h>
#include <unistd.h>
#include <algorithm>
#include <log/log.h>

#include <trusty/tipc.h>

#include <keymaster_ipc.h>
#include <beanpod_keymaster_ipc.h>

#define TRUSTY_DEVICE_NAME "/dev/trusty-ipc-dev0"

static int handle_ = -1;

int bp_keymaster_connect() {
    int rc = tipc_connect(TRUSTY_DEVICE_NAME, KEYMASTER_PORT);
    if (rc < 0) {
        return rc;
    }

    handle_ = rc;
    return 0;
}
#define MAX_SEND_BUF_SIZE  (3*1024)
int bp_keymaster_call(uint32_t cmd, void* in, uint32_t in_size, uint8_t* out,
                          uint32_t* out_size) {
    if (handle_ < 0) {
      ALOGE("not connected\n");
      return -EINVAL;
    }

    uint32_t remaind_size = 0;
    void *send_buf = in;

    remaind_size = in_size;
    ssize_t rc = 0;

    while (true) {

        ALOGI("remaind_size len=%d", remaind_size);

        if (remaind_size > MAX_SEND_BUF_SIZE) {

            size_t msg_size = MAX_SEND_BUF_SIZE + sizeof(struct keymaster_message);
            struct keymaster_message* msg = reinterpret_cast<struct keymaster_message*>(malloc(msg_size));
            if (!msg) {
              ALOGE("failed to allocate msg buffer\n");
              return -EINVAL;
            }

            msg->cmd = cmd | KEYMASTER_STOP_BIT;
            memcpy(msg->payload, send_buf, MAX_SEND_BUF_SIZE);

            rc = write(handle_, msg, msg_size);
            free(msg);

            remaind_size -= MAX_SEND_BUF_SIZE;
            send_buf = (void *)((uint8_t*)send_buf + MAX_SEND_BUF_SIZE);
            if (rc < 0) {
              ALOGE("failed to send cmd (%d) to %s: %s\n", cmd, KEYMASTER_PORT, strerror(errno));
              return -errno;
            }

        } else {
            size_t msg_size = remaind_size + sizeof(struct keymaster_message);
            struct keymaster_message* msg = reinterpret_cast<struct keymaster_message*>(malloc(msg_size));
            if (!msg) {
              ALOGE("failed to allocate msg buffer\n");
              return -EINVAL;
            }

            msg->cmd = cmd;
            if (remaind_size)
                memcpy(msg->payload, send_buf, remaind_size);

            ssize_t rc = write(handle_, msg, msg_size);
            free(msg);

            if (rc < 0) {
              ALOGE("failed to send cmd (%d) to %s: %s\n", cmd, KEYMASTER_PORT, strerror(errno));
              return -errno;
            }

            break;
        }
    }
    ALOGE("wait response");
    size_t out_max_size = *out_size;
    *out_size = 0;
    struct iovec iov[2];
    struct keymaster_message header;
    iov[0] = {.iov_base = &header, .iov_len = sizeof(struct keymaster_message)};
    while (true) {
      iov[1] = {.iov_base = out + *out_size,
                .iov_len = std::min<uint32_t>(KEYMASTER_MAX_BUFFER_LENGTH,
                                              out_max_size - *out_size)};
      rc = readv(handle_, iov, 2);
      if (rc < 0) {
          ALOGE("failed to retrieve response for cmd (%d) to %s: %s\n", cmd, KEYMASTER_PORT,
                strerror(errno));
          return -errno;
      }

      if ((size_t)rc < sizeof(struct keymaster_message)) {
          ALOGE("invalid response size (%d)\n", (int)rc);
          return -EINVAL;
      }

      if ((cmd | KEYMASTER_RESP_BIT) != (header.cmd & ~(KEYMASTER_STOP_BIT))) {
          ALOGE("invalid command (%d)", header.cmd);
          return -EINVAL;
      }
      *out_size += ((size_t)rc - sizeof(struct keymaster_message));
      if (header.cmd & KEYMASTER_STOP_BIT) {
          break;
      }
    }

    return rc;
}

void bp_keymaster_disconnect() {
    if (handle_ >= 0) {
        tipc_close(handle_);
    }
    handle_ = -1;
}

keymaster_error_t translate_error(int err) {
    switch (err) {
        case 0:
            return KM_ERROR_OK;
        case -EPERM:
        case -EACCES:
            return KM_ERROR_SECURE_HW_ACCESS_DENIED;

        case -ECANCELED:
            return KM_ERROR_OPERATION_CANCELLED;

        case -ENODEV:
            return KM_ERROR_UNIMPLEMENTED;

        case -ENOMEM:
            return KM_ERROR_MEMORY_ALLOCATION_FAILED;

        case -EBUSY:
            return KM_ERROR_SECURE_HW_BUSY;

        case -EIO:
            return KM_ERROR_SECURE_HW_COMMUNICATION_FAILED;

        case -EOVERFLOW:
            return KM_ERROR_INVALID_INPUT_LENGTH;

        default:
            return KM_ERROR_UNKNOWN_ERROR;
    }
}

uint8_t send_buf[BP_KEYMASTER_SEND_BUF_SIZE];
uint8_t recv_buf[BP_KEYMASTER_RECV_BUF_SIZE];

keymaster_error_t bp_keymaster_send(uint32_t command, const keymaster::Serializable& req,
                                        keymaster::KeymasterResponse* rsp) {
    uint32_t req_size = req.SerializedSize();
#if 1
    if (req_size > BP_KEYMASTER_SEND_BUF_SIZE) {
        ALOGE("Request too big: %u Max size: %u", req_size, BP_KEYMASTER_SEND_BUF_SIZE);
        return (keymaster_error_t)BEANPOD_KM_INVALID_INPUT_LENGTH;
    }
#endif

    ALOGI("CMD11:%x, req_size=%d, maxsize:%d", command, req_size, BP_KEYMASTER_SEND_BUF_SIZE);

    keymaster::Eraser send_buf_eraser(send_buf, BP_KEYMASTER_SEND_BUF_SIZE);
    req.Serialize(send_buf, send_buf + req_size);

    // Send it

    keymaster::Eraser recv_buf_eraser(recv_buf, BP_KEYMASTER_RECV_BUF_SIZE);
    uint32_t rsp_size = BP_KEYMASTER_RECV_BUF_SIZE;
    ALOGI("begin to call km");
    int rc = bp_keymaster_call(command, send_buf, req_size, recv_buf, &rsp_size);
    if (rc < 0) {
        // Reset the connection on tipc error
        bp_keymaster_disconnect();
        bp_keymaster_connect();
        ALOGE("invoke keymaster ta failed: %d\n", rc);
        // TODO(swillden): Distinguish permanent from transient errors and set error_ appropriately.
        return translate_error(rc);
    } else {
        ALOGE("Received %d byte response\n", rsp_size);
    }

    const uint8_t* p = recv_buf;
    if (!rsp->Deserialize(&p, p + rsp_size)) {
        ALOGE("Error deserializing response of size %d\n", (int)rsp_size);
        return KM_ERROR_UNKNOWN_ERROR;
    } else if (rsp->error != KM_ERROR_OK) {
        ALOGE("Response of size %d contained error code %d\n", (int)rsp_size, (int)rsp->error);
        return rsp->error;
    }
    return rsp->error;
}

keymaster_error_t bp_keymaster_send_no_request(uint32_t command, keymaster::KeymasterResponse* rsp) {

    //uint8_t recv_buf[BP_KEYMASTER_RECV_BUF_SIZE];
    keymaster::Eraser recv_buf_eraser(recv_buf, BP_KEYMASTER_RECV_BUF_SIZE);
    uint32_t rsp_size = BP_KEYMASTER_RECV_BUF_SIZE;
    int rc = bp_keymaster_call(command, NULL, 0, recv_buf, &rsp_size);
    if (rc < 0) {
        // Reset the connection on tipc error
        bp_keymaster_disconnect();
        bp_keymaster_connect();
        ALOGE("BeanpodKeymaster tipc error: %d\n", rc);
        // TODO(swillden): Distinguish permanent from transient errors and set error_ appropriately.
        return translate_error(rc);
    } else {
        ALOGE("BeanpodKeymaster Received %d byte response\n", rsp_size);
    }

    const uint8_t* p = recv_buf;
    if (!rsp->Deserialize(&p, p + rsp_size)) {
        ALOGE("BeanpodKeymaster Error deserializing response of size %d\n", (int)rsp_size);
        return KM_ERROR_UNKNOWN_ERROR;
    } else if (rsp->error != KM_ERROR_OK) {
        ALOGE("BeanpodKeymaster Response of size %d contained error code %d\n", (int)rsp_size, (int)rsp->error);
        return rsp->error;
    }
    return rsp->error;
}
