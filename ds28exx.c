/**
 * @file driver\ds28exx.c
 *
 * Copyright (C) 2022
 *
 * ds28exx.c is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * @author HinsShum hinsshum@qq.com
 *
 * @encoding utf-8
 */

/*---------- includes ----------*/
#include "ds28exx.h"
#include "driver.h"
#include "device.h"
#include "options.h"
#include "errorno.h"
#include "checksum.h"

/*---------- macro ----------*/
#define TAG                                     "DS28EXX"

/*---------- type define ----------*/
typedef int32_t (*ioctl_cb_func_t)(ds28exx_describe_t *, void *);
typedef struct {
    uint32_t ioctl;
    ioctl_cb_func_t cb;
} ioctl_cb_t;
typedef union ds28exx_ioctl *ds28exx_ioctl_t;


/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
static int32_t ds28exx_open(driver_t **pdrv);
static void ds28exx_close(driver_t **pdrv);
static int32_t ds28exx_ioctl(driver_t **pdrv, uint32_t ioctl, void *args);

static int32_t _ioctl_get_romid(ds28exx_describe_t *pdesc, void *args);
static int32_t _ioctl_write_scratchpad(ds28exx_describe_t *pdesc, void *args);
static int32_t _ioctl_compute_read_page_mac(ds28exx_describe_t *pdesc, void *args);

/*---------- variable ----------*/
DRIVER_DEFINED(ds28exx, ds28exx_open, ds28exx_close, NULL, NULL, ds28exx_ioctl, NULL);
static ioctl_cb_t _ioctl_cb_array[] = {
    {IOCTL_DS28EXX_GET_ROMID, _ioctl_get_romid},
    {IOCTL_DS28EXX_WRITE_SCRATCHPAD, _ioctl_write_scratchpad},
    {IOCTL_DS28EXX_COMPUTE_READ_PAGE_MAC, _ioctl_compute_read_page_mac}
};

/*---------- function ----------*/
static int32_t ds28exx_open(driver_t **pdrv)
{
    ds28exx_describe_t *pdesc = NULL;
    int32_t retval = CY_E_WRONG_ARGS;
    void *bus = NULL;
    union w1_bus_ioctl para = {0};

    assert(pdrv);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    do {
        if(!pdesc) {
            xlog_tag_error(TAG, "driver not found any device\n");
            break;
        }
        if(pdesc->ops.init) {
            if(!pdesc->ops.init()) {
                xlog_tag_error(TAG, "initialize failure\n");
                retval = CY_ERROR;
                break;
            }
        }
        /* bind bus */
        if(NULL == (bus = device_open(pdesc->bus_name))) {
            xlog_tag_error(TAG, "not found any w1 bus\n");
            if(pdesc->ops.deinit) {
                pdesc->ops.deinit();
            }
            retval = CY_ERROR;
            break;
        }
        pdesc->bus = bus;
        if(pdesc->slot) {
            para.slot.slot = pdesc->slot;
            para.slot.speed = W1_BUS_SPEED_CUSTOMIZE;
            device_ioctl(pdesc->bus, IOCTL_W1_BUS_SLOT_CONFIG, &para);
        }
        retval = CY_EOK;
    } while(0);

    return retval;
}

static void ds28exx_close(driver_t **pdrv)
{
    ds28exx_describe_t *pdesc = NULL;

    assert(pdrv);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    if(pdesc) {
        if(pdesc->bus) {
            device_close(pdesc->bus);
            pdesc->bus = NULL;
        }
        if(pdesc->ops.deinit) {
            pdesc->ops.deinit();
        }
    }
}

static void *_ioctl_cb_func_find(uint32_t ioctl)
{
    void *cb = NULL;

    for(uint16_t i = 0; i < ARRAY_SIZE(_ioctl_cb_array); ++i) {
        if(_ioctl_cb_array[i].ioctl == ioctl) {
            cb = _ioctl_cb_array[i].cb;
            break;
        }
    }

    return cb;
}

static int32_t ds28exx_ioctl(driver_t **pdrv, uint32_t ioctl, void *args)
{
    ds28exx_describe_t *pdesc = NULL;
    int32_t retval = CY_E_WRONG_ARGS;
    ioctl_cb_func_t cb = NULL;

    assert(pdrv);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    do {
        if(pdesc == NULL) {
            xlog_tag_error(TAG, "driver not foung any device\n");
            break;
        }
        if(NULL == (cb = (ioctl_cb_func_t)_ioctl_cb_func_find(ioctl))) {
            xlog_tag_warn(TAG, "driver not support this ioctl(%08X)\n", ioctl);
            break;
        }
        retval = cb(pdesc, args);
    } while(0);

    return retval;
}

static int32_t _ioctl_get_romid(ds28exx_describe_t *pdesc, void *args)
{
    ds28exx_ioctl_t para = (ds28exx_ioctl_t)args;
    union w1_bus_ioctl w1 = {0};
    int32_t retval = CY_E_WRONG_ARGS;

    do {
        if(!args) {
            break;
        }
        if(!para->romid.pbuf) {
            break;
        }
        w1.romid.pbuf = para->romid.pbuf;
        w1.romid.length = para->romid.length;
        retval = device_ioctl(pdesc->bus, IOCTL_W1_BUS_GET_ROMID, &w1);
    } while(0);

    return retval;
}

static int32_t _ioctl_write_scratchpad(ds28exx_describe_t *pdesc, void *args)
{
    ds28exx_ioctl_t para = (ds28exx_ioctl_t)args;
    int32_t retval = CY_E_WRONG_ARGS;
    uint8_t buf[3] = {0};
    uint16_t crc16 = 0;

    do {
        if(!args) {
            break;
        }
        buf[0] = W1_BUS_SKIP_ROM;
        buf[1] = 0x0F;
        buf[2] = 0x00;
        retval = CY_ERROR;
        if(CY_EOK != device_ioctl(pdesc->bus, IOCTL_W1_BUS_RESET, NULL)) {
            break;
        }
        if(CY_EOK != device_write(pdesc->bus, buf, 0, ARRAY_SIZE(buf))) {
            break;
        }
        if(CY_EOK != device_read(pdesc->bus, &crc16, 0, sizeof(crc16))) {
            break;
        }
        if(crc16 != checksum_crc16_maxim(&buf[1], 2)) {
            retval = CY_E_WRONG_CRC;
            break;
        }
        if(CY_EOK != device_write(pdesc->bus, para->scratchpad.pbuf, 0, para->scratchpad.length)) {
            break;
        }
        if(CY_EOK != device_read(pdesc->bus, &crc16, 0, sizeof(crc16))) {
            break;
        }
        if(crc16 != checksum_crc16_maxim(para->scratchpad.pbuf, para->scratchpad.length)) {
            retval = CY_E_WRONG_CRC;
            break;
        }
        retval = CY_EOK;
    } while(0);

    return retval;
}

static int32_t _ioctl_compute_read_page_mac(ds28exx_describe_t *pdesc, void *args)
{
    int32_t retval = CY_E_WRONG_ARGS;
    uint16_t crc16 = 0;
    ds28exx_ioctl_t para = (ds28exx_ioctl_t)args;
    uint8_t buf[3] = {0};

    do {
        if(!args) {
            break;
        }
        buf[0] = W1_BUS_SKIP_ROM;
        buf[1] = W1_BUS_RESUME_COMMAND;
        if(para->page.mode == DS28EXX_ANONYMOUS_MODE) {
            buf[2] = para->page.page_num | 0xE0;
        } else {
            buf[2] = para->page.page_num;
        }
        retval = CY_ERROR;
        if(CY_EOK != device_ioctl(pdesc->bus, IOCTL_W1_BUS_RESET, NULL)) {
            break;
        }
        if(CY_EOK != device_write(pdesc->bus, buf, 0, ARRAY_SIZE(buf))) {
            break;
        }
        if(CY_EOK != device_read(pdesc->bus, &crc16, 0, sizeof(crc16))) {
            break;
        }
        if(crc16 != checksum_crc16_maxim(&buf[1], 2)) {
            retval = CY_E_WRONG_CRC;
            break;
        }
        __delay_ms(10);
        if(CY_EOK != device_read(pdesc->bus, &para->page.out_come, 0, sizeof(para->page.out_come))) {
            break;
        }
        if(CY_EOK != device_read(pdesc->bus, para->page.pbuf, 0, para->page.length)) {
            break;
        }
        if(CY_EOK != device_read(pdesc->bus, &crc16, 0, sizeof(crc16))) {
            break;
        }
        if(crc16 != checksum_crc16_maxim(para->page.pbuf, para->page.length)) {
            retval = CY_E_WRONG_CRC;
            break;
        }
        retval = CY_EOK;
    } while(0);

    return retval;
}
