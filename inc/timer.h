/**
 * @file driver\include\timer.h
 *
 * Copyright (C) 2021
 *
 * timer.h is free software: you can redistribute it and/or modify
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
#ifndef __DRIVER_TIMER_H
#define __DRIVER_TIMER_H

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
/**
 * @brief Get the timer frequency information.
 * @param Args is a pointer of the buffer to store the frequency information.
 * @retval If the args is null, the interface will return CY_E_WRONG_ARGS,
 *         otherwise, return CY_EOK.
 */
#define IOCTL_TIMER_GET_FREQ                        (IOCTL_USER_START + 0x00)

/**
 * @brief Set a new frequency to timer.
 * @param Args is a pointer of the new frequency
 * @retval If the args is null, the interface will return CY_E_WRONG_ARGS,
 *         otherwise, return CY_EOK.
 */
#define IOCTL_TIMER_SET_FREQ                        (IOCTL_USER_START + 0x01)

/**
 * @brief Set the irq server callback function.
 * @note If enable the timer interrupt function, when occur once interrupt,
 *       the callback function will be called once.
 * @param Args is the pointer of the callback function.
 * @retval The interface always return CY_EOK.
 */
#define IOCTL_TIMER_SET_IRQ_HANDLER                 (IOCTL_USER_START + 0x02)

/**
 * @brief Enable the timer.
 * @param Args is not useful, it can be NULL.
 * @retval If enable successfully, the interface will return CY_EOK,
 *         otherwise, return CY_ERROR.
 */
#define IOCTL_TIMER_ENABLE                          (IOCTL_USER_START + 0x03)

/**
 * @brief Disable the timer.
 * @param Args is not useful, it can be NULL.
 * @retval If disable successfully, the interface will return CY_EOK,
 *         otherwise, return CY_ERROR.
 */
#define IOCTL_TIMER_DISABLE                         (IOCTL_USER_START + 0x04)

/*---------- type define ----------*/
typedef int32_t (*timer_irq_handler_fn)(uint32_t irq_handler, void *args, uint32_t len);

typedef struct {
    uint32_t freq;
    struct {
        bool (*init)(void);
        void (*deinit)(void);
        bool (*enable)(bool ctrl);
        timer_irq_handler_fn irq_handler;
    } ops;
} timer_describe_t;

/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/

#ifdef __cplusplus
}
#endif
#endif /* __DRIVER_TIMER_H */
