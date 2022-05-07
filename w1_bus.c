/**
 * @file driver\w1_bus.c
 *
 * Copyright (C) 2022
 *
 * w1_bus.c is free software: you can redistribute it and/or modify
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
#include "w1_bus.h"
#include "driver.h"
#include "device.h"
#include "errorno.h"
#include "options.h"
#include "checksum.h"
#include <string.h>

/*---------- macro ----------*/
/*---------- type define ----------*/
enum {
    LOW = 0,
    HIGH = !LOW
};

typedef int32_t (*ioctl_cb_func_t)(w1_bus_describe_t *, void *);
typedef struct {
    uint32_t ioctl;
    ioctl_cb_func_t cb;
} ioctl_cb_t;

typedef union w1_bus_ioctl *w1_bus_ioctl_t;

/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
static int32_t w1_bus_open(driver_t **pdrv);
static void w1_bus_close(driver_t **pdrv);
static int32_t w1_bus_write(driver_t **pdrv, void *buf, uint32_t nouse, uint32_t length);
static int32_t w1_bus_read(driver_t **pdrv, void *buf, uint32_t nouse, uint32_t length);
static int32_t w1_bus_ioctl(driver_t **pdrv, uint32_t ioctl, void *args);

static int32_t _ioctl_slot_config(w1_bus_describe_t *pdesc, void *args);
static int32_t _ioctl_get_romid(w1_bus_describe_t *pdesc, void *args);
static int32_t _ioctl_bus_reset(w1_bus_describe_t *pdesc, void *args);

/*---------- variable ----------*/
DRIVER_DEFINED(w1_bus, w1_bus_open, w1_bus_close, w1_bus_write, w1_bus_read, w1_bus_ioctl, NULL);
static ioctl_cb_t _ioctl_cb_array[] = {
    {IOCTL_W1_BUS_SLOT_CONFIG, _ioctl_slot_config},
    {IOCTL_W1_BUS_GET_ROMID, _ioctl_get_romid},
    {IOCTL_W1_BUS_RESET, _ioctl_bus_reset}
};

/*---------- function ----------*/
static inline bool __w1_bus_read_bit(w1_bus_describe_t *pdesc)
{
    bool retval = LOW;

    pdesc->ops.write_bit(LOW);
    __delay_us(pdesc->slot.t_rl);
    pdesc->ops.write_bit(HIGH);
    __delay_us(pdesc->slot.t_r_msp);
    retval = pdesc->ops.read_bit();
    __delay_us(pdesc->slot.t_rh);

    return retval;
}

static inline void __w1_bus_write_bit(w1_bus_describe_t *pdesc, bool bit)
{
    if(bit) {
        pdesc->ops.write_bit(LOW);
        __delay_us(pdesc->slot.t_w1l);
        pdesc->ops.write_bit(HIGH);
        __delay_us(pdesc->slot.t_w1h);
    } else {
        pdesc->ops.write_bit(LOW);
        __delay_us(pdesc->slot.t_w0l);
        pdesc->ops.write_bit(HIGH);
        __delay_us(pdesc->slot.t_w0h);
    }
}

static inline bool _w1_bus_reset(w1_bus_describe_t *pdesc)
{
    bool retval = false;

    __enter_critical();
    pdesc->ops.write_bit(LOW);
    __delay_us(pdesc->slot.t_rstl);
    pdesc->ops.write_bit(HIGH);
    __delay_us(pdesc->slot.t_rst_msp);
    retval = !pdesc->ops.read_bit();
    __delay_us(pdesc->slot.t_rsth);
    __exit_critical();

    return retval;
}

static uint8_t _w1_bus_read_byte(w1_bus_describe_t *pdesc)
{
    uint8_t retval = 0;

    __enter_critical();
    for(uint8_t i = 0; i < 8; ++i) {
        if(__w1_bus_read_bit(pdesc)) {
            retval |= (1UL << i);
        }
    }
    __exit_critical();

    return retval;
}

static void _w1_bus_write_byte(w1_bus_describe_t *pdesc, uint8_t byte)
{
    __enter_critical();
    for(uint8_t i = 0; i < 8; ++i) {
        __w1_bus_write_bit(pdesc, !!((byte >> i) & 0x01));
    }
    __exit_critical();
}

static inline void _w1_bus_read(w1_bus_describe_t *pdesc, uint8_t *bytes, uint16_t length)
{
    for(uint16_t i = 0; i < length; ++i) {
        bytes[i] = _w1_bus_read_byte(pdesc);
    }
}

static inline void _w1_bus_write(w1_bus_describe_t *pdesc, const uint8_t *bytes, const uint16_t length)
{
    for(uint16_t i = 0; i < length; ++i) {
        _w1_bus_write_byte(pdesc, bytes[i]);
    }
}

static inline void _set_standard_speed(w1_bus_describe_t *pdesc)
{
    pdesc->slot.t_rstl = 500;
    pdesc->slot.t_rst_msp = 70;
    pdesc->slot.t_rsth = 430;
    pdesc->slot.t_rl = 6;
    pdesc->slot.t_r_msp = 9;
    pdesc->slot.t_rh = 55;
    pdesc->slot.t_w0l = 60;
    pdesc->slot.t_w0h = 10;
    pdesc->slot.t_w1l = 6;
    pdesc->slot.t_w1h = 64;
}

static inline void _set_overdriver_speed(w1_bus_describe_t *pdesc)
{
    pdesc->slot.t_rstl = 70;
    pdesc->slot.t_rst_msp = 9;
    pdesc->slot.t_rsth = 40;
    pdesc->slot.t_rl = 1;
    pdesc->slot.t_r_msp = 1;
    pdesc->slot.t_rh = 7;
    pdesc->slot.t_w0l = 8;
    pdesc->slot.t_w0h = 3;
    pdesc->slot.t_w1l = 1;
    pdesc->slot.t_w1h = 8;
}

static int32_t _ops_check(w1_bus_describe_t *pdesc)
{
    int32_t retval = CY_EOK;

    if(pdesc->ops.write_bit == NULL || pdesc->ops.read_bit == NULL) {
        retval = CY_E_POINT_NONE;
        __debug_error("W1 bus ops not bind\n");
    }

    return retval;
}

static int32_t w1_bus_open(driver_t **pdrv)
{
    w1_bus_describe_t *pdesc = NULL;
    int32_t retval = CY_E_WRONG_ARGS;
    
    assert(pdrv);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    do {
        if(!pdesc) {
            __debug_error("W1 bus has no describe field\n");
            break;
        }
        if(CY_EOK != (retval = _ops_check(pdesc))) {
            break;
        }
        if(pdesc->ops.init) {
            if(!pdesc->ops.init()) {
                retval = CY_ERROR;
                break;
            }
        }
        /* release bus */
        pdesc->ops.write_bit(HIGH);
        /* set slot */
        if(pdesc->speed == W1_BUS_SPEED_STANDARD) {
            _set_standard_speed(pdesc);
        } else if(pdesc->speed == W1_BUS_SPEED_OVERDRIVER) {
            _set_overdriver_speed(pdesc);
        }
        retval = CY_EOK;
    } while(0);

    return retval;
}

static void w1_bus_close(driver_t **pdrv)
{
    w1_bus_describe_t *pdesc = NULL;

    assert(pdrv);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    if(pdesc && pdesc->ops.deinit) {
        pdesc->ops.deinit();
    }
}

static int32_t w1_bus_write(driver_t **pdrv, void *buf, uint32_t nouse, uint32_t length)
{
    w1_bus_describe_t *pdesc = NULL;
    int32_t retval = CY_E_WRONG_ARGS;

    assert(pdrv);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    do {
        if(!pdesc) {
            __debug_error("W1 bus not found any device\n");
            break;
        }
        if(!buf) {
            break;
        }
        _w1_bus_write(pdesc, buf, length);
        retval = CY_EOK;
    } while(0);

    return retval;
}

static int32_t w1_bus_read(driver_t **pdrv, void *buf, uint32_t nouse, uint32_t length)
{
    w1_bus_describe_t *pdesc = NULL;
    int32_t retval = CY_E_WRONG_ARGS;

    assert(pdrv);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    do {
        if(!pdesc) {
            __debug_error("W1 bus not found any device\n");
            break;
        }
        if(!buf) {
            break;
        }
        _w1_bus_read(pdesc, buf, length);
        retval = CY_EOK;
    } while(0);

    return retval;
}

static void *_ioctl_cb_find(uint32_t ioctl)
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

static int32_t w1_bus_ioctl(driver_t **pdrv, uint32_t ioctl, void *args)
{
    w1_bus_describe_t *pdesc = NULL;
    int32_t retval = CY_E_WRONG_ARGS;
    ioctl_cb_func_t cb = NULL;

    assert(pdrv);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    do {
        if(!pdesc) {
            __debug_error("W1 bus not found any device\n");
            break;
        }
        if(NULL == (cb = (ioctl_cb_func_t)_ioctl_cb_find(ioctl))) {
            __debug_warn("W1 bus not support this ioctl(%08X)\n", ioctl);
            break;
        }
        retval = cb(pdesc, args);
    } while(0);

    return retval;
}

static int32_t _ioctl_slot_config(w1_bus_describe_t *pdesc, void *args)
{
    w1_bus_ioctl_t para = (w1_bus_ioctl_t)args;
    int32_t retval = CY_E_WRONG_ARGS;
    
    do {
        if(!args) {
            break;
        }
        if(para->slot.speed == W1_BUS_SPEED_STANDARD) {
            _set_standard_speed(pdesc);
            retval = CY_EOK;
            break;
        }
        if(para->slot.speed == W1_BUS_SPEED_OVERDRIVER) {
            _set_overdriver_speed(pdesc);
            retval = CY_EOK;
            break;
        }
        if(para->slot.speed == W1_BUS_SPEED_CUSTOMIZE) {
            if(para->slot.slot == NULL) {
                break;
            }
            pdesc->slot = *para->slot.slot;
            retval = CY_EOK;
            break;
        }
        __debug_warn("W1 bus not support this speed mode: %d\n", para->slot.speed);
    } while(0);

    return retval;
}

static int32_t _ioctl_get_romid(w1_bus_describe_t *pdesc, void *args)
{
    w1_bus_ioctl_t para = (w1_bus_ioctl_t)args;
    int32_t retval = CY_E_WRONG_ARGS;
    uint8_t romid[8] = {0};
    uint8_t command = W1_BUS_READ_ROM;

    do {
        if(!args) {
            break;
        }
        if(!para->romid.pbuf) {
            break;
        }
        /* get romid */
        if(_w1_bus_reset(pdesc) != true) {
            __debug_warn("W1 bus reset failure\n");
            retval = CY_ERROR;
            break;
        }
        _w1_bus_write(pdesc, &command, sizeof(command));
        __delay_us(10);
        _w1_bus_read(pdesc, romid, ARRAY_SIZE(romid));
        /* check crc */
        if(checksum_crc8_maxim(romid, ARRAY_SIZE(romid)) != 0) {
            __debug_warn("W1 bus get rom id, but crc error\n");
            retval = CY_E_WRONG_CRC;
            break;
        }
        if(para->romid.length > ARRAY_SIZE(romid)) {
            para->romid.length = ARRAY_SIZE(romid);
        }
        memcpy(para->romid.pbuf, romid, para->romid.length);
        retval = CY_EOK;
    } while(0);

    return retval;
}

static int32_t _ioctl_bus_reset(w1_bus_describe_t *pdesc, void *args)
{
    int32_t retval = CY_EOK;

    if(_w1_bus_reset(pdesc) != true) {
        __debug_warn("W1 bus reset failure\n");
        retval = CY_ERROR;
    }

    return retval;
}
