/**
 * @file driver\si446x.c
 *
 * Copyright (C) 2022
 *
 * si446x.c is free software: you can redistribute it and/or modify
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
#include "si446x.h"
#include "driver.h"
#include "errorno.h"
#include "misc.h"
#include "options.h"

/*---------- macro ----------*/
#define CTS                             (0xFF)
#undef BIT
#define BIT(n)                          (1UL << n)

#define TAG                             "SI446X"

/*---------- type define ----------*/
typedef enum {
    CMD_NOP = 0x00,
    CMD_PART_INFO = 0x01,
    CMD_FUNC_INFO = 0x10,
    CMD_SET_PROPERTY = 0x11,
    CMD_GET_PROPERTY = 0x12,
    CMD_GET_ADC_READING = 0x14,
    CMD_FIFO_INFO = 0x15,
    CMD_PACKET_INFO = 0x16,
    CMD_IRCAL = 0x17,
    CMD_IRCAL_MANUAL = 0x1A,
    CMD_GET_INT_STATUS = 0x20,
    CMD_GET_PH_STATUS = 0x21,
    CMD_GET_MODEM_STATUS = 0x22,
    CMD_GET_CHIP_STATUS = 0x23,
    CMD_START_TX = 0x31,
    CMD_START_RX = 0x32,
    CMD_REQUEST_DEVICE_STATE = 0x33,
    CMD_CHANGE_STATE = 0x34,
    CMD_RX_HOP = 0x36,
    CMD_TX_HOP = 0x37,
    CMD_READ_CMD_BUFF = 0x44,
    CMD_FRR_A_READ = 0x50,
    CMD_FRR_B_READ = 0x51,
    CMD_FRR_C_READ = 0x53,
    CMD_FRR_D_READ = 0x57,
    CMD_WRITE_TX_FIFO = 0x66,
    CMD_READ_RX_FIFO = 0x77
} si446x_cmd_t;

typedef enum {
    FIFO_CMD_GET_INFO = 0x00,
    FIFO_CMD_CLEAR_TX_FIFO = 0x01,
    FIFO_CMD_CLEAR_RX_FIFO = 0x02
} si446x_fifo_cmd_t;

typedef enum {
    GRP_GLOBAL = 0x00,
    GRP_INT_CTL = 0x01,
    GRP_FRR_CTL = 0x02,
    GRP_PREAMBLE = 0x10,
    GRP_SYNC = 0x11,
    GRP_PKT = 0x12,
    GRP_MODEM = 0x20,
    GRP_MODEM_CHFLT = 0x21,
    GRP_PA = 0x22,
    GRP_SYNTH = 0x23,
    GRP_MATCH = 0x30,
    GRP_FREQ = 0x40,
    GRP_RX_HOP = 0x50,
    GRP_PTI = 0xF0
} si446x_group_t;

typedef enum {
    /* group global */
    GLOBAL_XO_TUNE = 0x00,
    GLOBAL_CLK_CFG = 0x01,
    GLOBAL_LOW_BATT_THRESH = 0x02,
    GLOBAL_CONFIG = 0x03,
    GLOBAL_WUT_CONFIG = 0x04,
    GLOBAL_WUT_M = 0x05,
    GLOBAL_WUT_R = 0x07,
    GLOBAL_WUT_LDC = 0x08,
    GLOBAL_WUT_CAL = 0x09,
    /* group int ctrl */
    INT_CTL_ENABLE = 0x00,
    INT_CTL_PH_ENABLE = 0x01,
    INT_CTL_MODEM_ENABLE = 0x02,
    INT_CTL_CHIP_ENABLE = 0x03,
    /* group frr ctrl */
    FRR_CTL_A_MODE = 0x00,
    FRR_CTL_B_MODE = 0x01,
    FRR_CTL_C_MODE = 0x02,
    FRR_CTL_D_MODE = 0x03,
    /* group preamble */
    PREAMBLE_TX_LENGTH = 0x00,
    PREAMBLE_CONFIG_STD_1 = 0x01,
    PREAMBLE_CONFIG_NSTD = 0x02,
    PREAMBLE_CONFIG_STD_2 = 0x03,
    PREAMBLE_CONFIG = 0x04,
    PREAMBLE_PATTERN = 0x05,
    PREAMBLE_POSTAMBLE_CONFIG = 0x09,
    PREAMBLE_POSTAMBLE_PATTERN = 0x0A,
    /* group sync */
    SYNC_CONFIG = 0x00,
    SYNC_BITS = 0x01,
    SYNC_CONFIG2 = 0x05,
    /* group pkt */
    PKT_CRC_CONFIG = 0x00,
    PKT_WHT_POLY = 0x01,
    PKT_WHT_SEED = 0x03,
    PKT_WHT_BIT_NUM = 0x05,
    PKT_CONFIG1 = 0x06,
    PKT_CONFIG2 = 0x07,
    PKT_LEN = 0x08,
    PKT_LEN_FIELD_SOURCE = 0x09,
    PKT_LEN_ADJUST = 0x0A,
    PKT_TX_THRESHOLD = 0x0B,
    PKT_RX_THRESHOLD = 0x0C,
    PKT_FIELD_1_LENGTH = 0x0D,
    PKT_FIELD_1_CONFIG = 0x0F,
    PKT_FIELD_1_CRC_CONFIG = 0x10,
    PKT_FIELD_2_LENGTH = 0x11,
    PKT_FIELD_2_CONFIG = 0x13,
    PKT_FIELD_2_CRC_CONFIG = 0x14,
    PKT_FIELD_3_LENGTH = 0x15,
    PKT_FIELD_3_CONFIG = 0x17,
    PKT_FIELD_3_CRC_CONFIG = 0x18,
    PKT_FIELD_4_LENGTH = 0x19,
    PKT_FIELD_4_CONFIG = 0x1B,
    PKT_FIELD_4_CRC_CONFIG = 0x1C,
    PKT_FIELD_5_LENGTH = 0x1D,
    PKT_FIELD_5_CONFIG = 0x1F,
    PKT_FIELD_5_CRC_CONFIG = 0x20,
    PKT_RX_FIELD_1_LENGTH = 0x21,
    PKT_RX_FIELD_1_CONFIG = 0x23,
    PKT_RX_FIELD_1_CRC_CONFIG = 0x24,
    PKT_RX_FIELD_2_LENGTH = 0x25,
    PKT_RX_FIELD_2_CONFIG = 0x27,
    PKT_RX_FIELD_2_CRC_CONFIG = 0x28,
    PKT_RX_FIELD_3_LENGTH = 0x29,
    PKT_RX_FIELD_3_CONFIG = 0x2B,
    PKT_RX_FIELD_3_CRC_CONFIG = 0x2C,
    PKT_RX_FIELD_4_LENGTH = 0x2D,
    PKT_RX_FIELD_4_CONFIG = 0x2F,
    PKT_RX_FIELD_4_CRC_CONFIG = 0x30,
    PKT_RX_FIELD_5_LENGTH = 0x31,
    PKT_RX_FIELD_5_CONFIG = 0x33,
    PKT_RX_FIELD_5_CRC_CONFIG = 0x34,
    PKT_CRC_SEED = 0x36,
    /* group modem */
    MODEM_MOD_TYPE = 0x00,
    MODEM_MAP_CONTROL = 0x01,
    MODEM_DSM_CTRL = 0x02,
    MODEM_DATA_RATE = 0x03,
    MODEM_TX_NCO_MODE = 0x06,
    MODEM_FREQ_DEV = 0x0A,
    MODEM_FREQ_OFFSET = 0x0D,
    MODEM_TX_FILTER_COEFF_8 = 0x0F,
    MODEM_TX_FILTER_COEFF_7 = 0x10,
    MODEM_TX_FILTER_COEFF_6 = 0x11,
    MODEM_TX_FILTER_COEFF_5 = 0x12,
    MODEM_TX_FILTER_COEFF_4 = 0x13,
    MODEM_TX_FILTER_COEFF_3 = 0x14,
    MODEM_TX_FILTER_COEFF_2 = 0x15,
    MODEM_TX_FILTER_COEFF_1 = 0x16,
    MODEM_TX_FILTER_COEFF_0 = 0x17,
    MODEM_TX_RAMP_DELAY = 0x18,
    MODEM_MDM_CTRL = 0x19,
    MODEM_IF_CONTROL = 0x1A,
    MODEM_IF_FREQ = 0x1B,
    MODEM_DECIMATION_CFG1 = 0x1E,
    MODEM_DECIMATION_CFG0 = 0x1F,
    MODEM_DECIMATION_CFG2 = 0x20,
    MODEM_IFPKD_THRESHOLDS = 0x21,
    MODEM_BCR_OSR = 0x22,
    MODEM_BCR_NCO_OFFSET = 0x24,
    MODEM_BCR_GAIN = 0x27,
    MODEM_BCR_GEAR = 0x29,
    MODEM_BCR_MISC1 = 0x2A,
    MODEM_BCR_MISC0 = 0x2B,
    MODEM_AFC_GEAR = 0x2C,
    MODEM_AFC_WAIT = 0x2D,
    MODEM_AFC_GAIN = 0x2E,
    MODEM_AFC_LIMITER = 0x30,
    MODEM_AFC_MISC = 0x32,
    MODEM_AFC_ZIFOFF = 0x33,
    MODEM_ADC_CTRL = 0x34,
    MODEM_AGC_CONTROL = 0x35,
    MODEM_AGC_WINDOW_SIZE = 0x38,
    MODEM_AGC_RFPD_DECAY = 0x39,
    MODEM_AGC_IFPD_DECAY = 0x3A,
    MODEM_FSK4_GAIN1 = 0x3B,
    MODEM_FSK4_GAIN0 = 0x3C,
    MODEM_FSK4_TH = 0x3D,
    MODEM_FSK4_MAP = 0x3F,
    MODEM_OOK_PDTC = 0x40,
    MODEM_OOK_BLOPK = 0x41,
    MODEM_OOK_CNT1 = 0x42,
    MODEM_OOK_MISC = 0x43,
    MODEM_RAW_CONTROL = 0x45,
    MODEM_RAW_EYE = 0x46,
    MODEM_ANT_DIV_MODE = 0x48,
    MODEM_ANT_DIV_CONTROL = 0x49,
    MODEM_RSSI_THRESH = 0x4A,
    MODEM_RSSI_JUMP_THRESH = 0x4B,
    MODEM_RSSI_CONTROL = 0x4C,
    MODEM_RSSI_CONTROL2 = 0x4D,
    MODEM_RSSI_COMP = 0x4E,
    MODEM_RAW_SEARCH2 = 0x50,
    MODEM_CLKGEN_BAND = 0x51,
    MODEM_SPIKE_DET = 0x54,
    MODEM_ONE_SHOT_AFC = 0x55,
    MODEM_RSSI_HYSTERESIS = 0x56,
    MODEM_RSSI_MUTE = 0x57,
    MODEM_FAST_RSSI_DELAY = 0x58,
    MODEM_PSM = 0x59,
    MODEM_DSA_CTRL1 = 0x5B,
    MODEM_DSA_CTRL2 = 0x5C,
    MODEM_DSA_QUAL = 0x5D,
    MODEM_DSA_RSSI = 0x5E,
    MODEM_DSA_MISC = 0x5F,
    /* group modem chflt */
    MODEM_CHFLT_RX1_CHFLT_COE = 0x00,
    MODEM_CHFLT_RX2_CHFLT_COE = 0x12,
    /* group pa */
    PA_MODE = 0x00,
    PA_PWR_LVL = 0x01,
    PA_BIAS_CLKDUTY = 0x02,
    PA_TC = 0x03,
    PA_RAMP_EX = 0x04,
    PA_RAMP_DOWN_DELAY = 0x05,
    PA_DIG_PWR_SEQ_CONFIG = 0x06,
    /* group synth */
    SYNTH_PFDCP_CPFF = 0x00,
    SYNTH_PFDCP_CPINT = 0x01,
    SYNTH_VCO_KV = 0x02,
    SYNTH_LPFILT3 = 0x03,
    SYNTH_LPFILT2 = 0x04,
    SYNTH_LPFILT1 = 0x05,
    SYNTH_LPFILT0 = 0x06,
    SYNTH_VCO_KVCAL = 0x07,
    /* group match */
    MATCH_VALUE_1 = 0x00,
    MATCH_MASK_1 = 0x01,
    MATCH_CTRL_1 = 0x02,
    MATCH_VALUE_2 = 0x03,
    MATCH_MASK_2 = 0x04,
    MATCH_CTRL_2 = 0x05,
    MATCH_VALUE_3 = 0x06,
    MATCH_MASK_3 = 0x07,
    MATCH_CTRL_3 = 0x08,
    MATCH_VALUE_4 = 0x09,
    MATCH_MASK_4 = 0x0A,
    MATCH_CTRL_4 = 0x0B,
    /* group freq */
    FREQ_CONTROL_INTE = 0x00,
    FREQ_CONTROL_FRAC = 0x01,
    FREQ_CONTROL_CHANNEL_STEP_SIZE = 0x04,
    FREQ_CONTROL_W_SIZE = 0x06,
    FREQ_CONTROL_VCOCNT_RX_ADJ = 0x07,
    /* group rx hop */
    RX_HOP_CONTROL = 0x00,
    RX_HOP_TABLE_SIZE = 0x01,
    RX_HOP_TABLE_ENTRY_0 = 0x02,
    RX_HOP_TABLE_ENTRY_1 = 0x03,
    RX_HOP_TABLE_ENTRY_2 = 0x04,
    RX_HOP_TABLE_ENTRY_3 = 0x05,
    RX_HOP_TABLE_ENTRY_4 = 0x06,
    RX_HOP_TABLE_ENTRY_5 = 0x07,
    RX_HOP_TABLE_ENTRY_6 = 0x08,
    RX_HOP_TABLE_ENTRY_7 = 0x09,
    RX_HOP_TABLE_ENTRY_8 = 0x0A,
    RX_HOP_TABLE_ENTRY_9 = 0x0B,
    RX_HOP_TABLE_ENTRY_10 = 0x0C,
    RX_HOP_TABLE_ENTRY_11 = 0x0D,
    RX_HOP_TABLE_ENTRY_12 = 0x0E,
    RX_HOP_TABLE_ENTRY_13 = 0x0F,
    RX_HOP_TABLE_ENTRY_14 = 0x10,
    RX_HOP_TABLE_ENTRY_15 = 0x11,
    RX_HOP_TABLE_ENTRY_16 = 0x12,
    RX_HOP_TABLE_ENTRY_17 = 0x13,
    RX_HOP_TABLE_ENTRY_18 = 0x14,
    RX_HOP_TABLE_ENTRY_19 = 0x15,
    RX_HOP_TABLE_ENTRY_20 = 0x16,
    RX_HOP_TABLE_ENTRY_21 = 0x17,
    RX_HOP_TABLE_ENTRY_22 = 0x18,
    RX_HOP_TABLE_ENTRY_23 = 0x19,
    RX_HOP_TABLE_ENTRY_24 = 0x1A,
    RX_HOP_TABLE_ENTRY_25 = 0x1B,
    RX_HOP_TABLE_ENTRY_26 = 0x1C,
    RX_HOP_TABLE_ENTRY_27 = 0x1D,
    RX_HOP_TABLE_ENTRY_28 = 0x1E,
    RX_HOP_TABLE_ENTRY_29 = 0x1F,
    RX_HOP_TABLE_ENTRY_30 = 0x20,
    RX_HOP_TABLE_ENTRY_31 = 0x21,
    RX_HOP_TABLE_ENTRY_32 = 0x22,
    RX_HOP_TABLE_ENTRY_33 = 0x23,
    RX_HOP_TABLE_ENTRY_34 = 0x24,
    RX_HOP_TABLE_ENTRY_35 = 0x25,
    RX_HOP_TABLE_ENTRY_36 = 0x26,
    RX_HOP_TABLE_ENTRY_37 = 0x27,
    RX_HOP_TABLE_ENTRY_38 = 0x28,
    RX_HOP_TABLE_ENTRY_39 = 0x29,
    RX_HOP_TABLE_ENTRY_40 = 0x2A,
    RX_HOP_TABLE_ENTRY_41 = 0x2B,
    RX_HOP_TABLE_ENTRY_42 = 0x2C,
    RX_HOP_TABLE_ENTRY_43 = 0x2D,
    RX_HOP_TABLE_ENTRY_44 = 0x2E,
    RX_HOP_TABLE_ENTRY_45 = 0x2F,
    RX_HOP_TABLE_ENTRY_46 = 0x30,
    RX_HOP_TABLE_ENTRY_47 = 0x31,
    RX_HOP_TABLE_ENTRY_48 = 0x32,
    RX_HOP_TABLE_ENTRY_49 = 0x33,
    RX_HOP_TABLE_ENTRY_50 = 0x34,
    RX_HOP_TABLE_ENTRY_51 = 0x35,
    RX_HOP_TABLE_ENTRY_52 = 0x36,
    RX_HOP_TABLE_ENTRY_53 = 0x37,
    RX_HOP_TABLE_ENTRY_54 = 0x38,
    RX_HOP_TABLE_ENTRY_55 = 0x39,
    RX_HOP_TABLE_ENTRY_56 = 0x3A,
    RX_HOP_TABLE_ENTRY_57 = 0x3B,
    RX_HOP_TABLE_ENTRY_58 = 0x3C,
    RX_HOP_TABLE_ENTRY_59 = 0x3D,
    RX_HOP_TABLE_ENTRY_60 = 0x3E,
    RX_HOP_TABLE_ENTRY_61 = 0x3F,
    RX_HOP_TABLE_ENTRY_62 = 0x40,
    RX_HOP_TABLE_ENTRY_63 = 0x41,
    /* group pti */
    PTI_CTL = 0x00,
    PTI_BAUD = 0x01,
    PTI_LOG_EN = 0x03
} si446x_property_t;

typedef enum {
    STATE_NO_CHANGE = 0x00,
    STATE_SLEEP = 0x01,
    STATE_SPI_ACTIVE = 0x02,
    STATE_READY = 0x03,
    STATE_READY2 = 0x04,
    STATE_TX_TUNE = 0x05,
    STATE_RX_TUNE = 0x06,
    STATE_TX = 0x07,
    STATE_RX = 0x08,
    STATE_RX_IDLE = 0x09
} si446x_state_t;

typedef enum {
    /* int pend */
    PH_INT_PEND = BIT(0),
    MODEM_INT_PEND = BIT(1),
    CHIP_INT_PEND = BIT(2),
    /* pd int pend */
    RX_FIFO_ALMOST_FULL_PEND = BIT(0),
    TX_FIFO_ALMOST_EMPTY_PEND = BIT(1),
    ALT_CRC_ERROR_PEND = BIT(2),
    CRC_ERROR_PEND = BIT(3),
    PACKET_RX_PEND = BIT(4),
    PACKET_SENT_PEND = BIT(5),
    FILTER_MISS_PEND = BIT(6),
    FILTER_MATCH_PEND = BIT(7),
    /* modem int pend */
    SYNC_DETECT_PEND = BIT(0),
    PREAMBLE_DETECT_PEND = BIT(1),
    INVALID_PREAMBLE_PEND = BIT(2),
    RSSI_PEND = BIT(3),
    RSSI_JUMP_PEND = BIT(4),
    INVALID_SYNC_PEND = BIT(5),
    POSTAMBLE_DETECT_PEND = BIT(6),
    RSSI_LATCH_PEND = BIT(7),
    /* chip int pend */
    WUT_PEND = BIT(0),
    LOW_BATT_PEND = BIT(1),
    CHIP_READY_PEND = BIT(2),
    CMD_ERROR_PEND = BIT(3),
    STATE_CHANGE_PEND = BIT(4),
    FIFO_UNDERFLOW_OVERFLOW_ERROR_PEND = BIT(5),
    CAL_PEND = BIT(6)
} si446x_int_pend_t;

typedef enum {
    /* tx complete state */
    TX_COMPLETE_STATE_NO_CHANGE = 0x00,
    TX_COMPLETE_STATE_SLEEP = 0x10,
    TX_COMPLETE_STATE_SPI_ACTIVE = 0x20,
    TX_COMPLETE_STATE_READY = 0x30,
    TX_COMPLETE_STATE_READY2 = 0x40,
    TX_COMPLETE_STATE_TX_TUNE = 0x50,
    TX_COMPLETE_STATE_RX_TUNE = 0x60,
    TX_COMPLETE_STATE_TX = 0x70,
    TX_COMPLETE_STATE_RX = 0x80,
    /* update */
    UPDATE_TX_PARA_ENTER_TX_MODE = 0x00,
    UPDATE_TX_PARA_ONLY = 0x80,
    /* retransmit */
    RETRANSMIT_DISABLE = 0x00,
    RETRANSMIT_ENABLE = 0x04,
    /* start */
    TX_START_IMMEDIATELY = 0x00,
    TX_START_WUT = 0x01
} si446x_tx_condition_t;

typedef enum {
    /* update */
    UPDATE_RX_PARA_ENTER_RX_MODE = 0x00,
    UPDATE_RX_PARA_ONLY = 0x80,
    /* start */
    RX_START_IMMEDIATELY = 0x00,
    RX_START_WUT = 0x01
} si446x_rx_condition_t;

typedef int32_t (*ioctl_cb_func_t)(si446x_describe_t *pdesc, void *args);
typedef struct {
    uint32_t cmd;
    ioctl_cb_func_t cb;
} ioctl_cb_t;

/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
static int32_t si446x_open(driver_t **pdrv);
static void si446x_close(driver_t **pdrv);
static int32_t si446x_write(driver_t **pdrv, void *pbuf, uint32_t type, uint32_t length);
static int32_t si446x_read(driver_t **pdrv, void *buf, uint32_t nouse, uint32_t length);
static int32_t si446x_ioctl(driver_t **pdrv, uint32_t cmd, void *args);
static int32_t si446x_irq_handler(driver_t **pdrv, uint32_t irq_handler, void *args, uint32_t length);

/* private ioctl function */
static int32_t _ioctl_reinitialize(si446x_describe_t *pdesc, void *args);
static int32_t _ioctl_set_receive_variable_max_length(si446x_describe_t *pdesc, void *args);
static int32_t _ioctl_set_receive_fixed_length(si446x_describe_t *pdesc, void *args);
static int32_t _ioctl_start_receiving(si446x_describe_t *pdesc, void *args);
static int32_t _ioctl_get_received_bytes(si446x_describe_t *pdesc, void *args);
static int32_t _ioctl_clear_receiver_fifo(si446x_describe_t *pdesc, void *args);
static int32_t _ioctl_clear_transmiter_fifo(si446x_describe_t *pdesc, void *args);
static int32_t _ioctl_interrupt_handling(si446x_describe_t *pdesc, void *args);
static int32_t _ioctl_set_evt_cb(si446x_describe_t *pdesc, void *args);
static int32_t _ioctl_set_interrup_handler(si446x_describe_t *pdesc, void *args);
static int32_t _ioctl_get_part_info(si446x_describe_t *pdesc, void *args);

/*---------- variable ----------*/
DRIVER_DEFINED(si446x, si446x_open, si446x_close, si446x_write, si446x_read, si446x_ioctl, si446x_irq_handler);
static ioctl_cb_t _ioctl_cb_tables[] = {
    {IOCTL_SI446X_REINITIALIZE, _ioctl_reinitialize},
    {IOCTL_SI446X_SET_RECEIVE_VARIABLE_MAX_LENGTH, _ioctl_set_receive_variable_max_length},
    {IOCTL_SI446X_SET_RECEIVE_FIXED_LENGTH, _ioctl_set_receive_fixed_length},
    {IOCTL_SI446X_START_RECEIVING, _ioctl_start_receiving},
    {IOCTL_SI446X_GET_RECEIVED_BYTES, _ioctl_get_received_bytes},
    {IOCTL_SI446X_CLEAR_RECEIVER_FIFO, _ioctl_clear_receiver_fifo},
    {IOCTL_SI446X_CLEAR_TRANSMITER_FIFO, _ioctl_clear_transmiter_fifo},
    {IOCTL_SI446X_INTERRUPT_HANDLING, _ioctl_interrupt_handling},
    {IOCTL_SI446X_SET_EVT_CALLBACK, _ioctl_set_evt_cb},
    {IOCTL_SI446X_SET_IRQ_HANDLER, _ioctl_set_interrup_handler},
    {IOCTL_SI446X_GET_PART_INFO, _ioctl_get_part_info}
};

/*---------- function ----------*/
static void _evt_cb_default(si446x_evt_t evt)
{
}

static bool _wait_cts(si446x_describe_t *pdesc)
{
    bool retval = false;
    uint8_t cts = 0;

    for(uint16_t i = 0; i < 100; ++i) {
        pdesc->ops.select(true);
        pdesc->ops.xfer(CMD_READ_CMD_BUFF);
        cts = pdesc->ops.xfer(CTS);
        pdesc->ops.select(false);
        if(cts == CTS) {
            retval = true;
            break;
        }
        __delay_us(100);
    }

    return retval;
}

static inline void _write_command(si446x_describe_t *pdesc, const uint8_t *pbuf, uint8_t length)
{
    pdesc->ops.select(true);
    for(uint8_t i = 0; i < length; ++i) {
        pdesc->ops.xfer(pbuf[i]);
    }
    pdesc->ops.select(false);
}

static bool _get_resp(si446x_describe_t *pdesc, uint8_t *out, uint8_t size)
{
    bool retval = false;
    uint8_t cts = 0;

    do {
        for(uint8_t i = 0; i < 100; ++i) {
            pdesc->ops.select(true);
            pdesc->ops.xfer(CMD_READ_CMD_BUFF);
            cts = pdesc->ops.xfer(CTS);
            if(cts == CTS) {
                retval = true;
                break;
            }
            pdesc->ops.select(false);
            __delay_us(100);
        }
        if(retval == false) {
            break;
        }
        for(uint8_t i = 0; i < size; ++i) {
            out[i] = pdesc->ops.xfer(CTS);
        }
        pdesc->ops.select(false);
    } while(0);

    return retval;
}

static bool _hw_reset(si446x_describe_t *pdesc)
{
    pdesc->ops.shutdown(true);
    __delay_ms(50);
    pdesc->ops.shutdown(false);
    __delay_ms(50);

    return _wait_cts(pdesc);
}

static bool _get_part_info(si446x_describe_t *pdesc)
{
    bool retval = false;
    uint8_t buf[8] = {0};

    buf[0] = CMD_PART_INFO;
    _write_command(pdesc, buf, 1);
    retval = _get_resp(pdesc, buf, ARRAY_SIZE(buf));
    if(retval) {
        /* buf[0]: chip revision
         * buf[1..2]: part number
         * buf[3]: part build
         * buf[4..5]: chip id
         * buf[6]: customer id
         * buf[7]: rom id
         */
        pdesc->part_info.chip_revision = buf[0];
        pdesc->part_info.part_number = ((uint16_t)buf[1] << 8) | buf[2];
        pdesc->part_info.part_build = buf[3];
        pdesc->part_info.chip_id = ((uint16_t)buf[4] << 8) | buf[5];
        pdesc->part_info.customer_id = buf[6];
        pdesc->part_info.rom_id = buf[7];
    }

    return retval;
}

static bool _get_int_status(si446x_describe_t *pdesc, uint8_t ph_clr_pend, uint8_t mode_clr_pend, uint8_t chip_clr_pend)
{
    uint8_t buf[4] = {0};

    buf[0] = CMD_GET_INT_STATUS;
    buf[1] = ph_clr_pend;
    buf[2] = mode_clr_pend;
    buf[3] = chip_clr_pend;
    _write_command(pdesc, buf, ARRAY_SIZE(buf));
    
    return _get_resp(pdesc, (void *)&pdesc->resp.int_status, sizeof(pdesc->resp.int_status));
}

static bool _set_property(si446x_describe_t *pdesc, uint8_t group, uint8_t start, const uint8_t *pbuf, uint8_t length)
{
    bool retval = false;

    if(length <= 0x0C) {
        pdesc->ops.select(true);
        pdesc->ops.xfer(CMD_SET_PROPERTY);
        pdesc->ops.xfer(group);
        pdesc->ops.xfer(length);
        pdesc->ops.xfer(start);
        for(uint8_t i = 0; i < length; ++i) {
            pdesc->ops.xfer(pbuf[i]);
        }
        pdesc->ops.select(false);
        retval = _wait_cts(pdesc);
    }

    return retval;
}

static bool _fifo_info(si446x_describe_t *pdesc, si446x_fifo_cmd_t cmd)
{
    uint8_t buf[2] = {0};

    buf[0] = CMD_FIFO_INFO;
    buf[1] = cmd;
    _write_command(pdesc, buf, ARRAY_SIZE(buf));

    return _get_resp(pdesc, (void *)&pdesc->resp.fifo_info, sizeof(pdesc->resp.fifo_info));
}

static inline bool _get_fifo_info(si446x_describe_t *pdesc)
{
    return _fifo_info(pdesc, FIFO_CMD_GET_INFO);
}

static inline bool _clear_receiver_fifo(si446x_describe_t *pdesc)
{
    return _fifo_info(pdesc, FIFO_CMD_CLEAR_RX_FIFO);
}

static inline bool _clear_transmiter_fifo(si446x_describe_t *pdesc)
{
    return _fifo_info(pdesc, FIFO_CMD_CLEAR_TX_FIFO);
}

static bool _start_rx(si446x_describe_t *pdesc, uint8_t channel, uint8_t condition, uint16_t recv_length)
{
    uint8_t buf[8] = {0};

    /* buf[0]: command
     * buf[1]: channel
     * buf[2]: condition
     * buf[3..4]: rx length
     * buf[5]: next state when rx timeout
     * buf[6]: next state when rx valid
     * buf[7]: next state when rx invalid
     */
    buf[0] = CMD_START_RX;
    buf[1] = channel;
    buf[2] = condition;
    buf[3] = (recv_length >> 8) & 0x1F;
    buf[4] = recv_length & 0xFF;
    buf[5] = STATE_NO_CHANGE;
    buf[6] = STATE_READY;
    buf[7] = STATE_RX;
    _write_command(pdesc, buf, ARRAY_SIZE(buf));

    return _wait_cts(pdesc);
}

static bool _start_tx(si446x_describe_t *pdesc, uint8_t channel, uint8_t condition, uint16_t trans_length)
{
    uint8_t buf[7] = {0};

    /* buf[0]: command
     * buf[1]: channel
     * buf[2]: condition
     * buf[3..4]: tx length
     * buf[5]: tx delay
     * buf[6]: repeat num
     */
    buf[0] = CMD_START_TX;
    buf[1] = channel;
    buf[2] = condition;
    buf[3] = (trans_length >> 8) & 0x1F;
    buf[4] = trans_length & 0xFF;
    buf[5] = 0x00;
    buf[6] = 0x00;
    _write_command(pdesc, buf, ARRAY_SIZE(buf));

    return _wait_cts(pdesc);
}

static bool _change_state(si446x_describe_t *pdesc, si446x_state_t next_state)
{
    uint8_t buf[2] = {0};

    buf[0] = CMD_CHANGE_STATE;
    buf[1] = next_state;
    _write_command(pdesc, buf, ARRAY_SIZE(buf));

    return _wait_cts(pdesc);
}

static void _get_received_data(si446x_describe_t *pdesc, uint8_t *pbuf, uint16_t length)
{
    pdesc->ops.select(true);
    pdesc->ops.xfer(CMD_READ_RX_FIFO);
    for(uint16_t i = 0; i < length; ++i) {
        pbuf[i] = pdesc->ops.xfer(CTS);
    }
    pdesc->ops.select(false);
}

static bool _receiver_configure(si446x_describe_t *pdesc)
{
    bool retval = false;
    uint8_t buf[2] = {0};

    do {
        if(pdesc->configure.receiver.variable_length_enabled) {
            /* variable length
             *  set pkt_len to 0x02(field1 containing the length but not put in rx fifo, field2 as variable length field)
             *  set field1 length to 0x01
             *  set field2 length to pdesc->configure.receiver.length.variable_max_length
             *  set field3 length to 0x00
             */
            if(pdesc->configure.receiver.length.variable_max_length > 8191) {
                break;
            }
            buf[0] = 0x02;
            if(_set_property(pdesc, GRP_PKT, PKT_LEN, buf, 1) != true) {
                break;
            }
            buf[0] = 0x00;
            buf[1] = 0x01;
            if(_set_property(pdesc, GRP_PKT, PKT_FIELD_1_LENGTH, buf, ARRAY_SIZE(buf)) != true) {
                break;
            }
            buf[0] = (pdesc->configure.receiver.length.variable_max_length >> 8) & 0xFF;
            buf[1] = pdesc->configure.receiver.length.variable_max_length & 0xFF;
            if(_set_property(pdesc, GRP_PKT, PKT_FIELD_2_LENGTH, buf, ARRAY_SIZE(buf)) != true) {
                break;
            }
            buf[0] = 0x00;
            buf[1] = 0x00;
            retval = _set_property(pdesc, GRP_PKT, PKT_FIELD_3_LENGTH, buf, ARRAY_SIZE(buf));
        } else {
            /* fixed length 
             *  set pkt_len to 0x00(disable variable packet mode)
             *  set field1 length to pdesc->configure.receiver.lengt.fixed_length
             *  set field2 length to 0x00
             */
            if(pdesc->configure.receiver.length.fixed_length > 64) {
                break;
            }
            buf[0] = 0x00;
            if(_set_property(pdesc, GRP_PKT, PKT_LEN, buf, 1) != true) {
                break;
            }
            buf[0] = (pdesc->configure.receiver.length.fixed_length >> 8) & 0xFF;
            buf[1] = pdesc->configure.receiver.length.fixed_length & 0xFF;
            if(_set_property(pdesc, GRP_PKT, PKT_FIELD_1_LENGTH, buf, ARRAY_SIZE(buf)) != true) {
                break;
            }
            buf[0] = 0x00;
            buf[1] = 0x00;
            retval = _set_property(pdesc, GRP_PKT, PKT_FIELD_2_LENGTH, buf, ARRAY_SIZE(buf));
        }
    } while(0);

    return retval;
}

static bool _reinitialize(si446x_describe_t *pdesc)
{
    bool retval = false;
    const uint8_t *configure_data = pdesc->configure.data;

    do {
        if(pdesc->configure.data == NULL) {
            xlog_tag_error(TAG, "No configure data exit, initialize si446x failure\n");
            break;
        }
        if(_hw_reset(pdesc) != true) {
            xlog_tag_error(TAG, "Reset SI446x failure\n");
            break;
        }
        if(_get_part_info(pdesc) != true) {
            xlog_tag_error(TAG, "Get SI446x part info failure during initialize sequence\n");
            break;
        }
        while(configure_data[0] != 0x00) {
            _write_command(pdesc, &configure_data[1], configure_data[0]);
            retval = _wait_cts(pdesc);
            if(retval == false) {
                xlog_tag_error(TAG, "Configure SI446x failure\n");
                break;
            }
            configure_data += configure_data[0] + 1;
        }
        if(retval != true) {
            break;
        }
        /* clear all int pend */
        if(_get_int_status(pdesc, 0, 0, 0) != true) {
            xlog_tag_error(TAG, "Clear SI446x int pend failure during initialize sequence\n");
            break;
        }
        if(_receiver_configure(pdesc) != true) {
            xlog_tag_error(TAG, "Configure receiver failure during initialize sequence\n");
            break;
        }
        retval = true;
    } while(0);

    return retval;
}

static int32_t si446x_open(driver_t **pdrv)
{
    int32_t retval = CY_E_WRONG_ARGS;
    si446x_describe_t *pdesc = NULL;

    assert(pdrv);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    do {
        if(!pdesc) {
            xlog_tag_error(TAG, "driver not found any device\n");
            break;
        }
        if(pdesc->ops.init) {
            if(pdesc->ops.init() != true) {
                xlog_tag_error(TAG, "board support package code initialize failure\n");
                retval = CY_ERROR;
                break;
            }
        }
        /* unselect si446x */
        pdesc->ops.select(false);
        /* set si446x shutdown */
        pdesc->ops.shutdown(true);
        /* initialize si446x */
        if(_reinitialize(pdesc) != true) {
            if(pdesc->ops.deinit) {
                pdesc->ops.deinit();
            }
            retval = CY_ERROR;
            break;
        }
        pdesc->ops.evt_cb = _evt_cb_default;
        /* echo si446x part info */
        xlog_tag_info(TAG, "SI446x part info:\n");
        xlog_tag_info(TAG, "\tchip revision: %02X\n", pdesc->part_info.chip_revision);
        xlog_tag_info(TAG, "\tpart number: %04X\n", pdesc->part_info.part_number);
        xlog_tag_info(TAG, "\tpart build: %02X\n", pdesc->part_info.part_build);
        xlog_tag_info(TAG, "\tchip id: %04X\n", pdesc->part_info.chip_id);
        xlog_tag_info(TAG, "\tcustomer id: %02X\n", pdesc->part_info.customer_id);
        xlog_tag_info(TAG, "\trom id: %02X\n", pdesc->part_info.rom_id);
        retval = CY_EOK;
    } while(0);

    return retval;
}

static void si446x_close(driver_t **pdrv)
{
    si446x_describe_t *pdesc = NULL;

    assert(pdrv);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    if(pdesc) {
        if(pdesc->ops.deinit) {
            pdesc->ops.deinit();
        }
    }
}

static bool _write_variable_length(si446x_describe_t *pdesc, const uint8_t *pbuf, uint8_t length)
{
    bool retval = false;

    do {
        if(length > 63) {
            break;
        }
        if(_clear_transmiter_fifo(pdesc) != true) {
            break;
        }
        /* exit rx state */
        if(_change_state(pdesc, STATE_READY) != true) {
            break;
        }
        if(_get_int_status(pdesc, 0, 0, 0) != true) {
            break;
        }
        pdesc->ops.select(true);
        pdesc->ops.xfer(CMD_WRITE_TX_FIFO);
        /* set length to field1 */
        pdesc->ops.xfer(length);
        /* set packet to field2 */
        for(uint8_t i = 0; i < length; ++i) {
            pdesc->ops.xfer(pbuf[i]);
        }
        pdesc->ops.select(false);
        retval = _start_tx(pdesc, 0, TX_COMPLETE_STATE_RX | UPDATE_TX_PARA_ENTER_TX_MODE |
                           RETRANSMIT_DISABLE | TX_START_IMMEDIATELY, length + 1);
    } while(0);

    return retval;
}

static bool _write_fixed_length(si446x_describe_t *pdesc, const uint8_t *pbuf, uint8_t length)
{
    bool retval = false;

    do {
        if(length > 64) {
            break;
        }
        if(_clear_transmiter_fifo(pdesc) != true) {
            break;
        }
        /* exit rx state */
        if(_change_state(pdesc, STATE_READY) != true) {
            break;
        }
        if(_get_int_status(pdesc, 0, 0, 0) != true) {
            break;
        }
        pdesc->ops.select(true);
        pdesc->ops.xfer(CMD_WRITE_TX_FIFO);
        /* set packet to field1 */
        for(uint8_t i = 0; i < length; ++i) {
            pdesc->ops.xfer(pbuf[i]);
        }
        pdesc->ops.select(false);
        retval = _start_tx(pdesc, 0, TX_COMPLETE_STATE_RX | UPDATE_TX_PARA_ENTER_TX_MODE |
                           RETRANSMIT_DISABLE | TX_START_IMMEDIATELY, length);
    } while(0);

    return retval;
}

static int32_t si446x_write(driver_t **pdrv, void *pbuf, uint32_t type, uint32_t length)
{
    int32_t retval = CY_E_WRONG_ARGS;
    si446x_describe_t *pdesc = NULL;

    assert(pdrv);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    do {
        if(pdesc == NULL) {
            xlog_tag_error(TAG, "driver not found any describe field\n");
            break;
        }
        if(pbuf == NULL || length == 0) {
            xlog_tag_error(TAG, "Buf can not be NULL or length can not be zero when try to write data to SI446x\n");
            break;
        }
        retval = CY_ERROR;
        if(type == SI446X_TRANS_TYPE_VARIABLE_LENGTH) {
            if(_write_variable_length(pdesc, pbuf, (length & 0xFF))) {
                retval = CY_EOK;
            }
        } else if(type == SI446X_TRANS_TYPE_FIXED_LENGTH) {
            if(_write_fixed_length(pdesc, pbuf, (length & 0xFF))) {
                retval = CY_EOK;
            }
        }
    } while(0);

    return retval;
}

static int32_t si446x_read(driver_t **pdrv, void *buf, uint32_t nouse, uint32_t length)
{
    int32_t retval = CY_E_WRONG_ARGS;
    si446x_describe_t *pdesc = NULL;

    assert(pdrv);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    do {
        if(pdesc == NULL) {
            xlog_tag_error(TAG, "driver not found any describe field\n");
            break;
        }
        if(buf == NULL || length == 0) {
            xlog_tag_error(TAG, "Buf can not be NULL or length can not be zero when try to read data from SI446x\n");
            break;
        }
        _get_received_data(pdesc, buf, (uint16_t)length);
        retval = CY_EOK;
    } while(0);

    return retval;
}

static int32_t _ioctl_reinitialize(si446x_describe_t *pdesc, void *args)
{
    int32_t retval = CY_ERROR;

    if(_reinitialize(pdesc)) {
        retval = CY_EOK;
    }

    return retval;
}

static int32_t _ioctl_set_receive_variable_max_length(si446x_describe_t *pdesc, void *args)
{
    uint16_t length = 0;
    int32_t retval = CY_ERROR;
    si446x_configure_t old_cfg = pdesc->configure;

    do {
        if(args == NULL) {
            break;
        }
        length = *(uint16_t *)args;
        pdesc->configure.receiver.variable_length_enabled = true;
        pdesc->configure.receiver.length.variable_max_length = length;
        if(_receiver_configure(pdesc) != true) {
            pdesc->configure = old_cfg;
            /* recover */
            _receiver_configure(pdesc);
            break;
        }
        retval = CY_EOK;
    } while(0);

    return retval;
}

static int32_t _ioctl_set_receive_fixed_length(si446x_describe_t *pdesc, void *args)
{
    uint16_t length = 0;
    int32_t retval = CY_ERROR;
    si446x_configure_t old_cfg = pdesc->configure;

    do {
        if(args == NULL) {
            break;
        }
        length = *(uint16_t *)args;
        pdesc->configure.receiver.variable_length_enabled = false;
        pdesc->configure.receiver.length.fixed_length = length;
        if(_receiver_configure(pdesc) != true) {
            pdesc->configure = old_cfg;
            /* recover */
            _receiver_configure(pdesc);
            break;
        }
        retval = CY_EOK;
    } while(0);

    return retval;
}

static int32_t _ioctl_start_receiving(si446x_describe_t *pdesc, void *args)
{
    int32_t retval = CY_ERROR;

    do {
        if(_get_int_status(pdesc, 0, 0, 0) != true) {
            xlog_tag_error(TAG, "Clear int pend failure before start receive\n");
            break;
        }
        if(_clear_receiver_fifo(pdesc) != true) {
            xlog_tag_error(TAG, "Reset receiver fifo failure before start receive\n");
            break;
        }
        /* start rx immediately(condition is 0x00) */
        if(_start_rx(pdesc, 0, UPDATE_RX_PARA_ENTER_RX_MODE | RX_START_IMMEDIATELY, 0) != true) {
            xlog_tag_error(TAG, "Start SI446x to receive failure\n");
            break;
        }
        retval = CY_EOK;
    } while(0);

    return retval;
}

static int32_t _ioctl_get_received_bytes(si446x_describe_t *pdesc, void *args)
{
    int32_t retval = CY_ERROR;
    uint16_t *plength = (uint16_t *)args;

    do {
        if(args == NULL) {
            break;
        }
        if(_get_fifo_info(pdesc) != true) {
            xlog_tag_error(TAG, "Get received bytes failure\n");
            break;
        }
        *plength = pdesc->resp.fifo_info.rx_fifo_count;
        retval = CY_EOK;
    } while(0);

    return retval;
}

static int32_t _ioctl_clear_receiver_fifo(si446x_describe_t *pdesc, void *args)
{
    int32_t retval = CY_EOK;

    if(_clear_receiver_fifo(pdesc) != true) {
        xlog_tag_error(TAG, "Clear SI446x receiver fifo failure\n");
        retval = CY_ERROR;
    }

    return retval;
}

static int32_t _ioctl_clear_transmiter_fifo(si446x_describe_t *pdesc, void *args)
{
    int32_t retval = CY_EOK;

    if(_clear_transmiter_fifo(pdesc) != true) {
        xlog_tag_error(TAG, "Clear SI446x transmiter fifo failure\n");
        retval = CY_ERROR;
    }

    return retval;
}

static inline void __ph_pend_handling(si446x_describe_t *pdesc, uint8_t ph_pend)
{
    if(ph_pend & RX_FIFO_ALMOST_FULL_PEND) {
        pdesc->ops.evt_cb(SI446X_EVT_RX_FIFO_ALMOST_FULL);
    }
    if(ph_pend & TX_FIFO_ALMOST_EMPTY_PEND) {
        pdesc->ops.evt_cb(SI446X_EVT_TX_FIFO_ALMOST_FULL);
    }
    if(ph_pend & ALT_CRC_ERROR_PEND) {
        pdesc->ops.evt_cb(SI446X_EVT_ALT_CRC_ERROR);
    }
    if(ph_pend & CRC_ERROR_PEND) {
        pdesc->ops.evt_cb(SI446X_EVT_CRC_ERROR);
    }
    if(ph_pend & PACKET_RX_PEND) {
        pdesc->ops.evt_cb(SI446X_EVT_PACKET_RX);
    }
    if(ph_pend & PACKET_SENT_PEND) {
        pdesc->ops.evt_cb(SI446X_EVT_PACKET_SENT);
    }
    if(ph_pend & FILTER_MISS_PEND) {
        pdesc->ops.evt_cb(SI446X_EVT_FILTER_MISS);
    }
    if(ph_pend & FILTER_MATCH_PEND) {
        pdesc->ops.evt_cb(SI446X_EVT_FILTER_MATCH);
    }
}

static inline void __modem_pend_handling(si446x_describe_t *pdesc, uint8_t modem_pend)
{
    if(modem_pend & SYNC_DETECT_PEND) {
        pdesc->ops.evt_cb(SI446X_EVT_SYNC_DETECTED);
    }
    if(modem_pend & PREAMBLE_DETECT_PEND) {
        pdesc->ops.evt_cb(SI446X_EVT_PREAMBLE_DETECTED);
    }
    if(modem_pend & INVALID_PREAMBLE_PEND) {
        pdesc->ops.evt_cb(SI446X_EVT_INVALID_PERAMBLE);
    }
    if(modem_pend & RSSI_PEND) {
        pdesc->ops.evt_cb(SI446X_EVT_RSSI);
    }
    if(modem_pend & RSSI_JUMP_PEND) {
        pdesc->ops.evt_cb(SI446X_EVT_RSSI_JUMP);
    }
    if(modem_pend & INVALID_SYNC_PEND) {
        pdesc->ops.evt_cb(SI446X_EVT_INVALID_SYNC);
    }
    if(modem_pend & POSTAMBLE_DETECT_PEND) {
        pdesc->ops.evt_cb(SI446X_EVT_POSTAMBLE_DETECTED);
    }
    if(modem_pend & RSSI_LATCH_PEND) {
        pdesc->ops.evt_cb(SI446X_EVT_RSSI_LATCH);
    }
}

static inline void __chip_pend_handling(si446x_describe_t *pdesc, uint8_t chip_pend)
{
    if(chip_pend & WUT_PEND) {
        pdesc->ops.evt_cb(SI446X_EVT_WUT);
    }
    if(chip_pend & LOW_BATT_PEND) {
        pdesc->ops.evt_cb(SI446X_EVT_LOW_BATT);
    }
    if(chip_pend & CHIP_READY_PEND) {
        pdesc->ops.evt_cb(SI446X_EVT_CHIP_READY);
    }
    if(chip_pend & CMD_ERROR_PEND) {
        pdesc->ops.evt_cb(SI446X_EVT_CMD_ERROR);
    }
    if(chip_pend & STATE_CHANGE_PEND) {
        pdesc->ops.evt_cb(SI446X_EVT_STATE_CHANGE);
    }
    if(chip_pend & FIFO_UNDERFLOW_OVERFLOW_ERROR_PEND) {
        pdesc->ops.evt_cb(SI446X_EVT_UNDERFLOW_OVERFLOW_ERROR);
    }
    if(chip_pend & CAL_PEND) {
        pdesc->ops.evt_cb(SI446X_EVT_CAL);
    }
}

static int32_t _ioctl_interrupt_handling(si446x_describe_t *pdesc, void *args)
{
    int32_t retval = CY_ERROR;
    si446x_resp_t resp = {0};

    do {
        if(_get_int_status(pdesc, 0, 0, 0) != true) {
            xlog_tag_error(TAG, "Get interrupt pend bits failure during interrupt handling\n");
            break;
        }
        retval = CY_EOK;
        resp = pdesc->resp;
        /* ph int pend handling */
        if(resp.int_status.int_pend & PH_INT_PEND) {
            __ph_pend_handling(pdesc, resp.int_status.ph_pend);
        }
        /* modem int pend handling */
        if(resp.int_status.int_pend & MODEM_INT_PEND) {
            __modem_pend_handling(pdesc, resp.int_status.modem_pend);
        }
        /* chip int pend handling */
        if(resp.int_status.int_pend & CHIP_INT_PEND) {
            __chip_pend_handling(pdesc, resp.int_status.chip_pend);
        }
    } while(0);

    return retval;
}

static int32_t _ioctl_set_evt_cb(si446x_describe_t *pdesc, void *args)
{
    void (*cb)(si446x_evt_t) = (void (*)(si446x_evt_t))args;

    if(cb) {
        pdesc->ops.evt_cb = cb;
    } else {
        pdesc->ops.evt_cb = _evt_cb_default;
    }

    return CY_EOK;
}

static int32_t _ioctl_set_interrup_handler(si446x_describe_t *pdesc, void *args)
{
    int32_t (*irq)(uint32_t, void *, uint32_t) = (int32_t (*)(uint32_t, void *, uint32_t))args;

    pdesc->ops.irq_handler = irq;

    return CY_EOK;
}

static int32_t _ioctl_get_part_info(si446x_describe_t *pdesc, void *args)
{
    int32_t retval = CY_E_WRONG_ARGS;
    si446x_part_info_t *part_info = (si446x_part_info_t *)args;

    if(args) {
        *part_info = pdesc->part_info;
        retval = CY_EOK;
    }

    return retval;
}

static ioctl_cb_func_t _ioctl_cb_func_find(uint32_t cmd)
{
    ioctl_cb_func_t cb = NULL;

    for(uint32_t i = 0; i < ARRAY_SIZE(_ioctl_cb_tables); ++i) {
        if(cmd == _ioctl_cb_tables[i].cmd) {
            cb = _ioctl_cb_tables[i].cb;
            break;
        }
    }

    return cb;
}

static int32_t si446x_ioctl(driver_t **pdrv, uint32_t cmd, void *args)
{
    int32_t retval = CY_E_WRONG_ARGS;
    si446x_describe_t *pdesc = NULL;
    ioctl_cb_func_t cb = NULL;

    assert(pdrv);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    do {
        if(pdesc == NULL) {
            xlog_tag_error(TAG, "driver not found any describe field\n");
            break;
        }
        cb = _ioctl_cb_func_find(cmd);
        if(cb == NULL) {
            xlog_tag_warn(TAG, "driver not support this ioctl cmd(%08X)\n", cmd);
            break;
        }
        retval = cb(pdesc, args);
    } while(0);

    return retval;
}

static int32_t si446x_irq_handler(driver_t **pdrv, uint32_t irq_handler, void *args, uint32_t length)
{
    si446x_describe_t *pdesc = NULL;
    int32_t retval = CY_E_WRONG_ARGS;

    assert(pdrv);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    do {
        if(pdesc == NULL) {
            break;
        }
        if(pdesc->ops.irq_handler) {
            retval = pdesc->ops.irq_handler(irq_handler, args, length);
        }
    } while(0);

    return retval;
}
