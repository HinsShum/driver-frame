/**
 * @file driver\inc\pwmc.h
 *
 * Copyright (C) 2022
 *
 * pwmc.h is free software: you can redistribute it and/or modify
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
#ifndef __PWMC_H
#define __PWMC_H

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
#define IOCTL_PWMC_ENABLE                                   (IOCTL_USER_START + 0x00)
#define IOCTL_PWMC_DISABLE                                  (IOCTL_USER_START + 0x01)
#define IOCTL_PWMC_GET_FREQ                                 (IOCTL_USER_START + 0x02)
#define IOCTL_PWMC_SET_FREQ                                 (IOCTL_USER_START + 0x03)
#define IOCTL_PWMC_GET_NUMBER_OF_CHANNEL                    (IOCTL_USER_START + 0x04)
#define IOCTL_PWMC_GET_DUTY                                 (IOCTL_USER_START + 0x05)
#define IOCTL_PWMC_SET_DUTY                                 (IOCTL_USER_START + 0x06)
#define IOCTL_PWMC_GET_DUTY_RAW                             (IOCTL_USER_START + 0x07)
#define IOCTL_PWMC_SET_DUTY_RAW                             (IOCTL_USER_START + 0x08)
#define IOCTL_PWMC_ENABLE_CHANNEL                           (IOCTL_USER_START + 0x09)
#define IOCTL_PWMC_DISABLE_CHANNEL                          (IOCTL_USER_START + 0x0A)
#define IOCTL_PWMC_SET_IRQ_HANDLER                          (IOCTL_USER_START + 0x0B)
#define IOCTL_PWMC_GET_DUTY_RAW_MAX                         (IOCTL_USER_START + 0x0C)

/*---------- type define ----------*/
typedef int32_t (*pwmc_irq_handler_fn)(uint32_t irq_handler, void *args, uint32_t len);

typedef struct {
    uint32_t freq;
    uint32_t number_of_channel;
    uint32_t raw_max;
    struct {
        bool (*init)(void);
        void (*deinit)(void);
        bool (*enable)(bool ctrl);
        bool (*update_duty_raw)(uint32_t channel, uint32_t raw);
        uint32_t (*get_duty_raw)(uint32_t channel);
        bool (*channel_ctrl)(uint32_t channel, bool ctrl);
        pwmc_irq_handler_fn irq_handler;
    } ops;
} pwmc_describe_t;

union pwmc_ioctl_param {
    struct {
        uint32_t channel;
        uint32_t raw;
    } duty_raw;
    struct {
        uint32_t channel;
        float duty;
    } duty;
};

struct pwmc_irq_param {
    enum {
        PWMC_IRQ_TYPE_UPDATED,
        PWMC_IRQ_TYPE_CHANNEL
    } type;
    uint32_t channel;
};

/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/

#ifdef __cplusplus
}
#endif
#endif /* __PWMC_H */
