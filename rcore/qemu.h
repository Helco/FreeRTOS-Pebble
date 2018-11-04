#pragma once
#include "platform.h"

#define QEMU_HEADER_SIGNATURE 0xFEED
#define QEMU_FOOTER_SIGNATURE 0xBEEF
#define QEMU_MAX_DATA_LEN 2048

typedef struct
{
    uint16_t signature; // QEMU_HEADER_SIGNATURE
    uint16_t protocol;  // one of QemuProtocol
    uint16_t len;       // number of bytes that follow (not including this header or footer)
} QemuCommChannelHeader;

typedef struct
{
    uint16_t signature; // QEMU_FOOTER_SIGNATURE
} QemuCommChannelFooter;

enum
{
    // Pebble standard
    QemuProtocol_SPP = 1,
    QemuProtocol_Tap = 2,
    QemuProtocol_BluetoothConnection = 3,
    QemuProtocol_Compass = 4,
    QemuProtocol_Battery = 5,
    QemuProtocol_Accel = 6,
    QemuProtocol_Vibration = 7,
    QemuProtocol_Button = 8,

    // Rebble custom
    QemuProtocol_Tests = 100
};

typedef void (*QemuEndpointHandler)(const void *data, size_t len);
typedef struct
{
    uint16_t protocol;
    QemuEndpointHandler handler;
} QemuEndpoint;

uint8_t qemu_init(void);
void qemu_send_packet(const void *buffer, size_t len);
