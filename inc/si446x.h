/**
 * @file driver\inc\si446x.h
 *
 * Copyright (C) 2022
 *
 * si446x.h is free software: you can redistribute it and/or modify
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
#ifndef __SI446X_H
#define __SI446X_H

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
 * @brief Reinitialize the si446x, it will reset the si446x,
 *        then get si446x part info, write configure data and
 *        clear all int pend.
 * @param Args is not used, it can be NULL.
 * @retval If reinitialize si446x successful, the interface will
 *         return CY_EOK, otherwise will return CY_ERROR.
 */
#define IOCTL_SI446X_REINITIALIZE                           (IOCTL_USER_START + 0x00)

/**
 * @brief Configure si446x to receive variable packet.
 * @param Args type is uint16_t. It allows the si446x to receive
 *        variable packet up to a maximum length of args value.
 *        Args value are not allowed to exceed 63.
 * @retval If configure success, it will return CY_EOK, otherwise
 *         return CY_ERROR.
 */
#define IOCTL_SI446X_SET_RECEIVE_VARIABLE_MAX_LENGTH        (IOCTL_USER_START + 0x01)

/**
 * @brief Configure si446x to receive fixed packet.
 * @param Args type is uint16_t. It allows the si446x to receive
 *        fixed packet which length is args value.
 *        Args value are not allowed to exceed 64.
 * @retval If configure success, it will return CY_EOK, otherwise
 *         return CY_ERROR.
 */
#define IOCTL_SI446X_SET_RECEIVE_FIXED_LENGTH               (IOCTL_USER_START + 0x02)

/**
 * @brief Configure SI446x to start receiving packet.
 * @param Args is not used, it can be NULL.
 * @retval If configure success, it will return CY_EOK, otherwise
 *         return CY_ERROR.
 */
#define IOCTL_SI446X_START_RECEIVING                        (IOCTL_USER_START + 0x03)

/**
 * @brief Get bytes of SI446x received.
 * @param Args type is pointer of uint16_t. It will store the bytes of
 *        SI446x received.
 * @retval If get bytes success, it will return CY_EOK, otherwise
 *         return CY_ERROR.
 */
#define IOCTL_SI446X_GET_RECEIVED_BYTES                     (IOCTL_USER_START + 0x04)

/**
 * @brief Clear SI446x receiver fifo, drop all data received.
 * @param Args is not used, it can be NULL.
 * @retval If clear success, it will return CY_EOK, otherwise
 *         return CY_ERROR.
 */
#define IOCTL_SI446X_CLEAR_RECEIVER_FIFO                    (IOCTL_USER_START + 0x05)

/**
 * @brief Clear SI446x transmiter fifo, drop all untransmitted data.
 * @param Args is not used, it can be NULL.
 * @retval If clear success, it will return CY_EOK, otherwise
 *         return CY_ERROR.
 */
#define IOCTL_SI446X_CLEAR_TRANSMITER_FIFO                  (IOCTL_USER_START + 0x06)

/**
 * @brief According to the interrupt pend bits to execute the event callback function.
 * @param Args is not used, it can be NULL.
 * @retval If get interrupt pend bits success, it will return CY_EOK, otherwise
 *         return CY_ERROR.
 */
#define IOCTL_SI446X_INTERRUPT_HANDLING                     (IOCTL_USER_START + 0x07)

/**
 * @brief Set interrupt event callback.
 * @param Args type is pointer of `void (*)(si446x_evt_t)`.
 * @retval If set callback success, it will return CY_EOK, otherwise
 *         return CY_ERROR.
 */
#define IOCTL_SI446X_SET_EVT_CALLBACK                       (IOCTL_USER_START + 0x08)

/**
 * @brief Set interrupt handler for SI446x nirq pin.
 * @param Args type is pointer of `int32_t (*)(uint32_t, void *, uint32_t)`.
 * @retval If set handler success, it will return CY_EOK, otherwise
 *         return CY_ERROR.
 */
#define IOCTL_SI446X_SET_IRQ_HANDLER                        (IOCTL_USER_START + 0x09)

/**
 * @brief Get SI446x part information.
 * @param Args type is pointer of si446x_part_info_t.
 * @retval If get success, it will return CY_EOK, otherwise
 *         return CY_ERROR or CY_E_WRONG_ARGS.
 */
#define IOCTL_SI446X_GET_PART_INFO                          (IOCTL_USER_START + 0x0A)

/*---------- type define ----------*/
typedef enum {
    SI446X_EVT_RX_FIFO_ALMOST_FULL,
    SI446X_EVT_TX_FIFO_ALMOST_FULL,
    SI446X_EVT_ALT_CRC_ERROR,
    SI446X_EVT_CRC_ERROR,
    SI446X_EVT_PACKET_RX,
    SI446X_EVT_PACKET_SENT,
    SI446X_EVT_FILTER_MISS,
    SI446X_EVT_FILTER_MATCH,
    SI446X_EVT_SYNC_DETECTED,
    SI446X_EVT_PREAMBLE_DETECTED,
    SI446X_EVT_INVALID_PERAMBLE,
    SI446X_EVT_RSSI,
    SI446X_EVT_RSSI_JUMP,
    SI446X_EVT_INVALID_SYNC,
    SI446X_EVT_POSTAMBLE_DETECTED,
    SI446X_EVT_RSSI_LATCH,
    SI446X_EVT_WUT,
    SI446X_EVT_LOW_BATT,
    SI446X_EVT_CHIP_READY,
    SI446X_EVT_CMD_ERROR,
    SI446X_EVT_STATE_CHANGE,
    SI446X_EVT_UNDERFLOW_OVERFLOW_ERROR,
    SI446X_EVT_CAL
} si446x_evt_t;

typedef enum {
    SI446X_TRANS_TYPE_VARIABLE_LENGTH,
    SI446X_TRANS_TYPE_FIXED_LENGTH
} si446x_transmite_type_t;

typedef struct {
    /**
     * @brief Initialize the board support package code(some peripherals, 
     *         e.g. spi, cs gpio and sdn gpio)
     * @retval Return the initialize result, success is true, failure is false
     */
    bool (*init)(void);
    /**
     * @brief Deinitialize the board support package code(some peripherals,
     *         e.g. spi, cs gpio and sdn gpio)
     * @retval None
     */
    void (*deinit)(void);
    /**
     * @brief Control the device enter or exit shutdown mode
     * @param shutdown If ture, control the device to enter shutdown mode, 
     *                 otherwise, exit shutdown mode
     * @retval None
     */
    void (*shutdown)(bool shutdown);
    /**
     * @brief Select the device for communication
     * @param select If true, select the device for communication,
     *               otherwise, unselect
     * @retval None
     */
    void (*select)(bool select);
    /**
     * @brief Two-way transmission of data
     * @param data The data to be sent to the slave
     * @retval The data to be sent to the master
     */
    uint8_t (*xfer)(uint8_t data);
    /**
     * @brief SI446x event callback
     * @param evt reference @si446x_evt_t.
     * @retval None
     */
    void (*evt_cb)(si446x_evt_t evt);
    /**
     * @brief SI446x interrupt handler for NIRQ pin.
     * @param irq irq handler pointer.
     * @param args not used.
     * @param length not used.
     * @retval CY_EOK.
     */
    int32_t (*irq_handler)(uint32_t irq, void *args, uint32_t length);
} si446x_ops_t;

typedef struct {
    uint8_t chip_revision;      /*<< chip mask revision */
    uint8_t part_build;         /*<< part build number */
    uint8_t customer_id;        /*<< customer identity */
    uint8_t rom_id;             /*<< rom identity */
    uint16_t part_number;       /*<< e.g. si4461 will be 0x4461 */
    uint16_t chip_id;           /*<< chip identity */
} si446x_part_info_t;

typedef union {
    struct {
        uint8_t int_pend;
        uint8_t int_status;
        uint8_t ph_pend;
        uint8_t ph_status;
        uint8_t modem_pend;
        uint8_t modem_status;
        uint8_t chip_pend;
        uint8_t chip_status;
    } int_status;
    struct {
        uint8_t rx_fifo_count;
        uint8_t tx_fifo_space;
    } fifo_info;
} si446x_resp_t;

typedef struct {
    const uint8_t *data;                    /*<< si446x reg configure data array */
    struct {
        bool variable_length_enabled;       /*<< flag for variable length */
        union {
            uint16_t variable_max_length;   /*<< field2 max length and field1 length will be set to 0x01 */
            uint16_t fixed_length;          /*<< field1 fixed length */
        } length;
    } receiver;
} si446x_configure_t;

typedef struct {
    si446x_configure_t configure;
    si446x_part_info_t part_info;
    si446x_resp_t resp;
    si446x_ops_t ops;
} si446x_describe_t;

/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/

#ifdef __cplusplus
}
#endif
#endif /* __SI446X_H */
