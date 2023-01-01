/*
 * Copyright (C) 2016 The Android Open Source Project
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
#pragma once

#include <stdint.h>
#include <microtrust/interface/storage.h>

int storage_file_delete(struct storage_msg *msg,
                        const void *req, size_t req_len);

int storage_file_open(struct storage_msg *msg,
                      const void *req, size_t req_len);

int storage_file_close(struct storage_msg *msg,
                       const void *req, size_t req_len);

int storage_file_write(struct storage_msg *msg,
                       const void *req, size_t req_len);

int storage_file_read(struct storage_msg *msg,
                      const void *req, size_t req_len);

int storage_file_get_size(struct storage_msg *msg,
                          const void *req, size_t req_len);

int storage_file_set_size(struct storage_msg *msg,
                          const void *req, size_t req_len);

int storage_init(const char *dirname);

int storage_persistent_init(const char *dirname);

int storage_proinfo_init(const char *nodename);

int storage_init_ok(struct storage_msg *msg,
                       const void *r, size_t req_len);

int storage_init_persist_ok(struct storage_msg *msg,
                      const void *r, size_t req_len);

int storage_init_proinfo_ok(struct storage_msg *msg,
                     const void *r, size_t req_len);

int storage_sync_checkpoint(void);
