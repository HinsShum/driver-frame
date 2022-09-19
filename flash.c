/**
 * @file driver/flash.c
 *
 * Copyright (C) 2021
 *
 * flash.c is free software: you can redistribute it and/or modify
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
#include "flash.h"
#include "driver.h"
#include "errorno.h"
#include "options.h"

/*---------- macro ----------*/
#define TAG                                         "Flash"

/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
static int32_t flash_open(driver_t **pdrv);
static void flash_close(driver_t **pdrv);
static int32_t flash_write(driver_t **pdrv, void *buf, uint32_t addition, uint32_t len);
static int32_t flash_read(driver_t **pdrv, void *buf, uint32_t addition, uint32_t len);
static int32_t flash_ioctl(driver_t **pdrv, uint32_t cmd, void *args);

/* private ioctl functions
 */
static int32_t _ioctl_flash_erase_block(flash_describe_t *pdesc, void *args);
static int32_t _ioctl_flash_erase_chip(flash_describe_t *pdesc, void *args);
static int32_t _ioctl_flash_check_addr_is_block_start(flash_describe_t *pdesc, void *args);
static int32_t _ioctl_flash_get_info(flash_describe_t *pdesc, void *args);
static int32_t _ioctl_flash_set_callback(flash_describe_t *pdesc, void *args);
static int32_t _ioctl_flash_set_lock(flash_describe_t *pdesc, void *args);
static int32_t _ioctl_flash_set_unlock(flash_describe_t *pdesc, void *args);

/*---------- type define ----------*/
typedef int32_t (*ioctl_cb_func_t)(flash_describe_t *pdesc, void *args);
typedef struct {
    uint32_t ioctl_cmd;
    ioctl_cb_func_t cb;
} ioctl_cb_t;

/*---------- variable ----------*/
/* define flash driver
 */
DRIVER_DEFINED(flash, flash_open, flash_close, flash_write, flash_read, flash_ioctl, NULL);

/* define flash ioctl functions
 */
static ioctl_cb_t ioctl_cb_array[] = {
    {IOCTL_FLASH_ERASE_BLOCK, _ioctl_flash_erase_block},
    {IOCTL_FLASH_ERASE_CHIP, _ioctl_flash_erase_chip},
    {IOCTL_FLASH_CHECK_ADDR_IS_BLOCK_START, _ioctl_flash_check_addr_is_block_start},
    {IOCTL_FLASH_GET_INFO, _ioctl_flash_get_info},
    {IOCTL_FLASH_SET_CALLBACK, _ioctl_flash_set_callback},
    {IOCTL_FLASH_SET_LOCK, _ioctl_flash_set_lock},
    {IOCTL_FLASH_SET_UNLOCK, _ioctl_flash_set_unlock}
};

/*---------- function ----------*/
static int32_t flash_open(driver_t **pdrv)
{
    flash_describe_t *pdesc = NULL;
    int32_t retval = CY_E_WRONG_ARGS;

    assert(pdrv);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    do {
        if(!pdesc) {
            xlog_tag_error(TAG, "device has no describe field\n");
            break;
        }
        retval = CY_EOK;
        if(pdesc->ops.init) {
            if(!pdesc->ops.init()) {
                retval = CY_ERROR;
            }
        }
    } while(0);

    return retval;
}

static void flash_close(driver_t **pdrv)
{
    flash_describe_t *pdesc = NULL;

    assert(pdrv);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    if(pdesc && pdesc->ops.deinit) {
        pdesc->ops.deinit();
    }
}

static int32_t flash_write(driver_t **pdrv, void *buf, uint32_t addition, uint32_t len)
{
    flash_describe_t *pdesc = NULL;
    uint32_t actual_len = 0;

    assert(pdrv);
    assert(buf);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    if(pdesc && pdesc->ops.write) {
        actual_len = pdesc->ops.write(buf, addition, len);
    }

    return (int32_t)actual_len;
}

static int32_t flash_read(driver_t **pdrv, void *buf, uint32_t addition, uint32_t len)
{
    flash_describe_t *pdesc = NULL;
    uint32_t actual_len = 0;

    assert(pdrv);
    assert(buf);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    if(pdesc && pdesc->ops.read) {
        actual_len = pdesc->ops.read(buf, addition, len);
    }

    return (int32_t)actual_len;
}

static int32_t _ioctl_flash_erase_block(flash_describe_t *pdesc, void *args)
{
    int32_t retval = CY_E_WRONG_ARGS;
    uint32_t *paddr = (uint32_t *)args;
    uint32_t erase_length = 0;

    do {
        if(!args) {
            xlog_tag_error(TAG, "Args is NULL, erase block functions must specify the erase address\n");
            break;
        }
        if(!pdesc->ops.erase_block) {
            xlog_tag_error(TAG, "device has no erase block ops\n");
            break;
        }
        erase_length = pdesc->ops.erase_block(*paddr);
        if(!erase_length) {
            xlog_tag_error(TAG, "Erase address(%08X) failed\n", *paddr);
            retval = CY_ERROR;
            break;
        }
        retval = (int32_t)erase_length;
        xlog_tag_message(TAG, "Erase address(%08X) block size: %dbytes\n", *paddr, erase_length);
    } while(0);

    return retval;
}

static int32_t _ioctl_flash_erase_chip(flash_describe_t *pdesc, void *args)
{
    int32_t retval = CY_E_WRONG_ARGS;

    do {
        if(!pdesc->ops.erase_chip) {
            xlog_tag_error(TAG, "device has no erase chip ops\n");
            break;
        }
        if(!pdesc->ops.erase_chip()) {
            xlog_tag_error(TAG, "Erase chip failed\n");
            retval = CY_ERROR;
        } else {
            retval = CY_EOK;
        }
    }while(0);

    return retval;
}

static int32_t _ioctl_flash_check_addr_is_block_start(flash_describe_t *pdesc, void *args)
{
    int32_t retval = CY_E_WRONG_ARGS;
    uint32_t *paddr = (uint32_t *)args;

    do {
        if(!pdesc->ops.addr_is_block_start) {
            xlog_tag_error(TAG, "device has no check ops\n");
            break;
        }
        if(!pdesc->ops.addr_is_block_start(*paddr)) {
            retval = CY_ERROR;
        } else {
            retval = CY_EOK;
        }
    } while(0);

    return retval;
}

static int32_t _ioctl_flash_get_info(flash_describe_t *pdesc, void *args)
{
    int32_t retval = CY_E_WRONG_ARGS;
    flash_info_t *pinfo = (flash_info_t *)args;

    do {
        if(!args) {
            xlog_tag_error(TAG, "Args is NULL, no memory to store flash information\n");
            break;
        }
        pinfo->start = pdesc->start;
        pinfo->end = pdesc->end;
        pinfo->block_size = pdesc->block_size;
        retval = CY_EOK;
    } while(0);

    return retval;
}

static int32_t _ioctl_flash_set_callback(flash_describe_t *pdesc, void *args)
{
    void (*cb)(void) = (void (*)(void))args;

    pdesc->ops.cb = cb;


    return CY_EOK;
}

static int32_t _ioctl_flash_set_lock(flash_describe_t *pdesc, void *args)
{
    void (*cb)(void) = (void (*)(void))args;

    pdesc->ops.lock = cb;

    return CY_EOK;
}

static int32_t _ioctl_flash_set_unlock(flash_describe_t *pdesc, void *args)
{
    void (*cb)(void) = (void (*)(void))args;

    pdesc->ops.unlock = cb;

    return CY_EOK;
}

static ioctl_cb_func_t _ioctl_cb_func_find(uint32_t ioctl_cmd)
{
    ioctl_cb_func_t cb = NULL;

    for(uint32_t i = 0; i < ARRAY_SIZE(ioctl_cb_array); ++i) {
        if(ioctl_cb_array[i].ioctl_cmd == ioctl_cmd) {
            cb = ioctl_cb_array[i].cb;
            break;
        }
    }

    return cb;
}

static int32_t flash_ioctl(driver_t **pdrv, uint32_t cmd, void *args)
{
    flash_describe_t *pdesc = NULL;
    int32_t retval = CY_E_WRONG_ARGS;
    ioctl_cb_func_t cb = NULL;

    assert(pdrv);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    do {
        if(!pdesc) {
            xlog_tag_error(TAG, "device has no describe field\n");
            break;
        }
        if(NULL == (cb = _ioctl_cb_func_find(cmd))) {
            xlog_tag_error(TAG, "driver not support this command(%08X)\n", cmd);
            break;
        }
        retval = cb(pdesc, args);
    } while(0);

    return retval;
}
