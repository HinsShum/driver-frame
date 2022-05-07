/**
 * @file driver\include\w1_bus.h
 *
 * Copyright (C) 2022
 *
 * w1_bus.h is free software: you can redistribute it and/or modify
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
#ifndef __W1_BUS_H
#define __W1_BUS_H

#ifdef __cplusplus
extern "C"
{
#endif

/*---------- includes ----------*/
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "device.h"

/*---------- macro ----------*/
#define IOCTL_W1_BUS_SLOT_CONFIG                (IOCTL_USER_START + 0x00)
#define IOCTL_W1_BUS_GET_ROMID                  (IOCTL_USER_START + 0x01)
#define IOCTL_W1_BUS_RESET                      (IOCTL_USER_START + 0x02)

/*---------- type define ----------*/
enum {
    W1_BUS_SPEED_STANDARD,
    W1_BUS_SPEED_OVERDRIVER,
    W1_BUS_SPEED_CUSTOMIZE
};

enum {
    W1_BUS_READ_ROM = 0x33,
    W1_BUS_OVERDRIVER_SKIP_ROM = 0x3C,
    W1_BUS_MATCH_ROM = 0x55,
    W1_BUS_OVERDIRVER_MATCH_ROM = 0x69,
    W1_BUS_RESUME_COMMAND = 0xA5,
    W1_BUS_SKIP_ROM = 0xCC,
    W1_BUS_SEARCH_ROM = 0xF0,
};

struct w1_bus_slot {
    uint16_t t_rstl;                    /*<< 1-Wire reset low time */
    uint16_t t_rst_msp;                 /*<< 1-Wire reset presence detect sample time */
    uint16_t t_rsth;                    /*<< 1-Wire reset high time after sample */
    uint16_t t_rl;                      /*<< 1-wire read low time */
    uint16_t t_r_msp;                   /*<< 1-Wire read presence detect sample time */
    uint16_t t_rh;                      /*<< 1-Wire read high time after sample */
    uint16_t t_w0l;                     /*<< 1-Wire write 0 low time */
    uint16_t t_w0h;                     /*<< 1-Wire write 0 high time */
    uint16_t t_w1l;                     /*<< 1-Wire write 1 low time */
    uint16_t t_w1h;                     /*<< 1-Wire write 1 high time */
};

typedef struct {
    bool (*init)(void);
    void (*deinit)(void);
    void (*write_bit)(bool bit);
    bool (*read_bit)(void);
} w1_bus_ops_t;

typedef struct {
    uint32_t speed;
    struct w1_bus_slot slot;
    w1_bus_ops_t ops;
} w1_bus_describe_t;

union w1_bus_ioctl {
    struct {
        uint8_t *pbuf;
        uint8_t length;
    } romid;
    struct {
        struct w1_bus_slot *slot;
        uint32_t speed;
    } slot;
};

/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/

#ifdef __cplusplus
}
#endif
#endif /* __W1_BUS_H */
