#include "qemu.h"
#include "log.h"

void test_handler(const void *data, size_t len)
{
    KERN_LOG("QEMU", APP_LOG_LEVEL_INFO, "I got these %d bytes from qemu:", len);
    debug_write((const unsigned char*)data, len);
    debug_write("\n", 1);
}

const QemuEndpoint qemu_endpoints[] =
{
    {
        .protocol = QemuProtocol_Tests,
        .handler = test_handler
    },
    { .handler = NULL }
};
