/**
 * @file driver\include\ds28exx.h
 *
 * Copyright (C) 2022
 *
 * ds28exx.h is free software: you can redistribute it and/or modify
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
#ifndef __DS28EXX_H
#define __DS28EXX_H

#ifdef __cplusplus
extern "C"
{
#endif

/*---------- includes ----------*/
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "w1_bus.h"

/*---------- macro ----------*/
#define IOCTL_DS28EXX_GET_ROMID                         (IOCTL_USER_START + 0x00)
#define IOCTL_DS28EXX_WRITE_SCRATCHPAD                  (IOCTL_USER_START + 0x01)
#define IOCTL_DS28EXX_COMPUTE_READ_PAGE_MAC             (IOCTL_USER_START + 0x02)

/*---------- type define ----------*/
enum {
    DS28EXX_NOT_ANONYMOUS_MODE = 0U,
    DS28EXX_ANONYMOUS_MODE = !DS28EXX_NOT_ANONYMOUS_MODE
};

typedef struct {
    bool (*init)(void);
    void (*deinit)(void);
} ds28exx_ops_t;

typedef struct {
    void *bus_name;
    void *bus;
    struct w1_bus_slot *slot;
    ds28exx_ops_t ops;
} ds28exx_describe_t;

union ds28exx_ioctl {
    struct {
        uint8_t *pbuf;
        uint8_t length;
    } romid;
    struct {
        uint8_t *pbuf;
        uint8_t length;
    } scratchpad;
    struct {
        bool mode;
        uint8_t page_num;
        uint8_t out_come;
        uint8_t *pbuf;
        uint8_t length;
    } page;
};

/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/

#ifdef __cplusplus
}
#endif
#endif /* __DS28EXX_H */
