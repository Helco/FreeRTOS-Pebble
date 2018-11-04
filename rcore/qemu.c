#include "qemu.h"
#include "task.h"
#include "log.h"
#include "rebbleos.h"

extern const QemuEndpoint qemu_endpoints[];

#define STACK_SZ_QEMU configMINIMAL_STACK_SIZE + 600

static TaskHandle_t _qemu_task;
static StackType_t _qemu_task_stack[STACK_SZ_QEMU];
static StaticTask_t _qemu_task_buf;
static uint8_t _qemu_packet_buffer[QEMU_MAX_DATA_LEN];

static void _qemu_thread(void *pvParameters);
static void _qemu_handle_packet();

uint8_t qemu_init(void)
{
    hw_qemu_init();
    _qemu_task = xTaskCreateStatic(_qemu_thread,
                                   "QEMU", STACK_SZ_QEMU, NULL,
                                   tskIDLE_PRIORITY + 3UL,
                                   _qemu_task_stack, &_qemu_task_buf);
    return INIT_RESP_OK;
}

static QemuEndpointHandler _qemu_find_endpoint_handler(uint16_t protocol)
{
    const QemuEndpoint *endpoint = qemu_endpoints;
    while (endpoint->handler != NULL)
    {
        if (endpoint->protocol == protocol)
            return endpoint->handler;
    }
    return NULL;
}

static void _qemu_thread(void *pvParameters)
{
    for (;;)
    {
        if (hw_qemu_has_data())
            _qemu_handle_packet();
        vTaskDelay(pdMS_TO_TICKS(16));
    }
}

static void _qemu_read_header(QemuCommChannelHeader *header)
{
    hw_qemu_read(_qemu_packet_buffer, sizeof(QemuCommChannelHeader));
    const QemuCommChannelHeader *raw_header = (const QemuCommChannelHeader*)_qemu_packet_buffer;
    header->signature = ntohs(raw_header->signature);
    header->protocol = ntohs(raw_header->protocol);
    header->len = ntohs(raw_header->len);
}

static void _qemu_read_footer(QemuCommChannelFooter *footer)
{
    hw_qemu_read(_qemu_packet_buffer, sizeof(QemuCommChannelFooter));
    const QemuCommChannelFooter *raw_footer = (const QemuCommChannelFooter*)_qemu_packet_buffer;
    footer->signature = ntohs(raw_footer->signature);
}

static void _qemu_handle_packet(void)
{
    QemuCommChannelHeader header;
    _qemu_read_header(&header);
    if (header.signature != QEMU_HEADER_SIGNATURE)
    {
        KERN_LOG("QEMU", APP_LOG_LEVEL_ERROR, "Invalid header signature: %x", header.signature);
        return;
    }
    if (header.len > QEMU_MAX_DATA_LEN)
    {
        KERN_LOG("QEMU", APP_LOG_LEVEL_ERROR, "Invalid packet size: %d", header.len);
        return;
    }
    QemuEndpointHandler handler = _qemu_find_endpoint_handler(header.protocol);
    if (handler == NULL)
    {
        KERN_LOG("QEMU", APP_LOG_LEVEL_ERROR, "Unknown protocol: %d", header.protocol);
    }

    size_t len = header.len;
    hw_qemu_read(_qemu_packet_buffer, len);
    handler(_qemu_packet_buffer, len);

    const QemuCommChannelFooter footer;
    _qemu_read_footer(&footer);
    if (footer.signature != QEMU_FOOTER_SIGNATURE)
    {
        KERN_LOG("QEMU", APP_LOG_LEVEL_ERROR, "Invalid footer signature: %x", footer.signature);
        return;
    }
}
