#pragma once
#include "platform.h"

void hw_qemu_init(void);
int hw_qemu_has_data(void);
size_t hw_qemu_read(void *buffer, size_t max_len);
size_t hw_qemu_write(const void *buffer, size_t len);
