/**
 *   Copyright 2021 Oleh Sharuda <oleh.sharuda@gmail.com>
 *
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 */

/*!  \file
 *   \brief ADXL345 3-axis accelerometer support.
 *   \author Oleh Sharuda
 */
#pragma once

#include <time.h>
#include <memory>
#include "ekit_bus.hpp"
#include "ekit_device.hpp"
#include "tools.hpp"

/// \defgroup group_adxl350 ADXL345
/// \brief ADXL345 support
/// @{
/// \page page_adxl350
/// \tableofcontents
///
/// \section sect_adxl350_01 Work with ADXL345
///
/// Documentation to be written
/// <CHECKIT> complete documentation ...
///

#pragma pack(push, 1)

struct ADXL345Data {
    int16_t x;
    int16_t y;
    int16_t z;
};

struct ADXL345OffsetData {
    uint8_t header;
    int8_t  ofs_x;
    int8_t  ofs_y;
    int8_t  ofs_z;
};

struct ADXL345DataDbl {
    double x;
    double y;
    double z;
};

#pragma pack(pop)

struct ADXL345Confiuration {
    // POWER_CTL
    bool    link;
    bool    auto_sleep;
    bool    measure;
    bool    sleep;
    uint8_t wakeup_rate;

    // FIFO_CTL
    bool    trigger_int;
    size_t  fifo_len;
    uint8_t fifo_mode;

    // DATA_FORMAT
    bool    self_test;
    bool    three_wire_spi;
    bool    int_invert;
    bool    full_res;
    bool    justify_msb;
    uint8_t range;

    // BW_RATE
    bool    low_power;
    uint8_t rate;
};

struct ADXL345Sample {
    // The timestamp field must go the first, it is used during reading into the
    // following data member.
    struct timespec    timestamp;
    struct ADXL345Data data;
};

struct ADXL345SampleFP {
    struct timespec       timestamp;
    struct ADXL345DataDbl data;
};

/// \class ADXL345
/// \brief ADXL345 support.
class ADXL345 final : public EKitDeviceBase {
    /// \typedef super
    /// \brief Defines parent class
    typedef EKitDeviceBase  super;
    ADXL345Confiuration     adxl_config;
    double                  max_val    = 0.0;
    double                  res_scale  = 1.0;
    static constexpr double grav_accel = 9.8L;

   public:
    /// \brief No default constructor
    ADXL345()                          = delete;

    /// \brief Copy construction is forbidden
    ADXL345(const ADXL345&)            = delete;

    /// \brief Assignment is forbidden
    ADXL345& operator=(const ADXL345&) = delete;

    /// \brief Constructor to be used
    /// \param ebus - reference to shared pointer with EKitBus.
    /// \param timeout_ms - Timeout in milliseconds. Zero or negative means
    /// infinite timeout. \param name - name of the device
    ADXL345(std::shared_ptr<EKitBus>& ebus, int timeout_ms, const char* name);

    /// \brief Destructor (virtual)
    ~ADXL345() override;

    void    enable(bool enabled);
    void    configure(uint8_t rate              = BW_RATE_100HZ,
                      uint8_t watermark_samples = FIFO_CTL_SAMPLES_DEFAULT,
                      uint8_t range             = DATA_FORMAT_RANGE_2g);
    void    get_data(ADXL345Sample* data);
    size_t  get_data_len(bool& fifo_triggered);
    uint8_t get_events();
    void    clear_fifo();
    void    get_offset_data(ADXL345OffsetData& data);
    void    set_offset_data(ADXL345OffsetData& data);
    void to_double_data(const ADXL345Data& uint_data, ADXL345DataDbl& dbl_data) const;

    enum ADXL345Registers : uint8_t {
        DEVID          = 0x00,
        THRESH_TAP     = 0x1D,
        OFSX           = 0x1E,
        OFSY           = 0x1F,
        OFSZ           = 0x20,
        DUR            = 0x21,
        LATENT         = 0x22,
        WINDOW         = 0x23,
        THRESH_ACT     = 0x24,
        THRESH_INACT   = 0x25,
        TIME_INACT     = 0x26,
        ACT_INACT_CTL  = 0x27,
        THRESH_FF      = 0x28,
        TIME_FF        = 0x29,
        TAP_AXES       = 0x2A,
        ACT_TAP_STATUS = 0x2B,
        BW_RATE        = 0x2C,
        POWER_CTL      = 0x2D,
        INT_ENABLE     = 0x2E,
        INT_MAP        = 0x2F,
        INT_SOURCE     = 0x30,
        DATA_FORMAT    = 0x31,
        DATAX0         = 0x32,
        DATAX1         = 0x33,
        DATAY0         = 0x34,
        DATAY1         = 0x35,
        DATAZ0         = 0x36,
        DATAZ1         = 0x37,
        FIFO_CTL       = 0x38,
        FIFO_STATUS    = 0x39
    };

    enum ADXL345Constants : uint8_t {
        REG_ADDR_MASK            = 0b00111111,
        READ_REG_FLAG            = 0b10000000,
        MULTYBYTE_FLAG           = 0b01000000,
        DEVID_VALUE              = 0xE5,

        FIFO_CTL_SAMPLES_MASK    = 0b00011111,
        FIFO_CTL_SAMPLES_DEFAULT = 0b00001111,
        FIFO_CTL_TRIGGER         = 0b00100000,
        FIFO_CTL_MODE            = 0b11000000,
        FIFO_CTL_MODE_BYPASS     = 0b00000000,
        FIFO_CTL_MODE_FIFO       = 0b01000000,
        FIFO_CTL_MODE_STREAM     = 0b10000000,
        FIFO_CTL_MODE_TRIGGER    = 0b11000000,

        DATA_FORMAT_SELF_TEST    = 0b10000000,
        DATA_FORMAT_3WIRE_SPI    = 0b01000000,
        DATA_FORMAT_INT_INVERT   = 0b00100000,
        DATA_FORMAT_FULL_RES     = 0b00001000,
        DATA_FORMAT_MSB          = 0b00000100,
        DATA_FORMAT_RANGE_MASK   = 0b00000011,
        DATA_FORMAT_RANGE_2g     = 0b00000000,
        DATA_FORMAT_RANGE_4g     = 0b00000001,
        DATA_FORMAT_RANGE_8g     = 0b00000010,
        DATA_FORMAT_RANGE_16g    = 0b00000011,

        BW_RATE_LOW_POWER        = 0b00010000,
        BW_RATE_RATE_MASK        = 0b00001111,
        BW_RATE_3200HZ           = 0b00001111,
        BW_RATE_1600HZ           = 0b00001110,
        BW_RATE_800HZ            = 0b00001101,
        BW_RATE_400HZ            = 0b00001100,
        BW_RATE_200HZ            = 0b00001011,
        BW_RATE_100HZ            = 0b00001010,
        BW_RATE_50HZ             = 0b00001001,
        BW_RATE_25HZ             = 0b00001000,
        BW_RATE_12_5HZ           = 0b00000111,
        BW_RATE_6_25HZ           = 0b00000110,
        BW_RATE_3_13HZ           = 0b00000101,
        BW_RATE_1_56HZ           = 0b00000100,
        BW_RATE_0_78HZ           = 0b00000011,
        BW_RATE_0_39HZ           = 0b00000010,
        BW_RATE_0_20HZ           = 0b00000001,
        BW_RATE_0_10HZ           = 0b00000000,

        FIFO_STATUS_ENTRIES      = 0b00111111,
        FIFO_STATUS_TRIGGER      = 0b10000000,

        POWER_CTL_LINK           = 0b00100000,
        POWER_CTL_AUTO_SLEEP     = 0b00010000,
        POWER_CTL_MEASURE        = 0b00001000,
        POWER_CTL_SLEEP          = 0b00000100,
        POWER_CTL_WAKEUP_MASK    = 0b00000011,
        POWER_CTL_WAKEUP_8HZ     = 0b00000000,
        POWER_CTL_WAKEUP_4HZ     = 0b00000001,
        POWER_CTL_WAKEUP_2HZ     = 0b00000010,
        POWER_CTL_WAKEUP_1HZ     = 0b00000011
    };

    enum ADXLEvents : uint8_t {
        ADXL_EV_DATA_READY = 0b10000000,
        ADXL_EV_SINGLE_TAP = 0b01000000,
        ADXL_EV_DOUBLE_TAP = 0b00100000,
        ADXL_EV_ACTIVITY   = 0b00010000,
        ADXL_EV_INACTIVITY = 0b00001000,
        ADXL_EV_FREE_FALL  = 0b00000100,
        ADXL_EV_WATERMARK  = 0b00000010,
        ADXL_EV_OVERRUN    = 0b00000001
    };

   private:
    EKIT_ERROR single_byte_transaction(bool          read,
                                       const uint8_t addr,
                                       uint8_t&      data,
                                       EKitTimeout&  to);

    EKIT_ERROR get_fifo_ctl_priv(size_t&      fifolen,
                                 uint8_t&     mode,
                                 bool&        trigger,
                                 EKitTimeout& to);

    EKIT_ERROR set_fifo_ctl_priv(const size_t  fifolen,
                                 const uint8_t fifo_mode,
                                 const bool    trigger_int,
                                 EKitTimeout&  to);

    EKIT_ERROR set_data_format_priv(bool         self_test,
                                    bool         three_wire_spi,
                                    bool         int_invert,
                                    bool         full_res,
                                    bool         justify_msb,
                                    uint8_t      range,
                                    EKitTimeout& to);

    EKIT_ERROR get_data_format_priv(bool&        self_test,
                                    bool&        three_wire_spi,
                                    bool&        int_invert,
                                    bool&        full_res,
                                    bool&        justify_msb,
                                    uint8_t&     range,
                                    EKitTimeout& to);

    EKIT_ERROR set_rate_priv(bool low_power, uint8_t rate, EKitTimeout& to);

    EKIT_ERROR get_rate_priv(bool& low_power, uint8_t& rate, EKitTimeout& to);

    EKIT_ERROR set_power_ctl_priv(bool         link,
                                  bool         auto_sleep,
                                  bool         measure,
                                  bool         sleep,
                                  uint8_t      wakeup_rate,
                                  EKitTimeout& to);
    EKIT_ERROR get_power_ctl_priv(bool&        link,
                                  bool&        auto_sleep,
                                  bool&        measure,
                                  bool&        sleep,
                                  uint8_t&     wakeup_rate,
                                  EKitTimeout& to);

    EKIT_ERROR check_device_id_priv(EKitTimeout& to);

    EKIT_ERROR read_config_priv(ADXL345Confiuration& config, EKitTimeout& to);

    EKIT_ERROR write_config_priv(ADXL345Confiuration& config, EKitTimeout& to);
};

/// @}
