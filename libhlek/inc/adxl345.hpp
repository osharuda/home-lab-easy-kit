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

/// \struct ADXL345Data
/// \brief Describes acceleration measurement as native int16_t values.
struct ADXL345Data {
    int16_t x;  /// \brief Acceleration along X axis
    int16_t y;  /// \brief Acceleration along Y axis
    int16_t z;  /// \brief Acceleration along Z axis
};

/// \struct ADXL345OffsetData
/// \brief Describes acceleration offset configuration data, used to elliminate statical measurement error.
struct ADXL345OffsetData {
    uint8_t header; /// \brief Reserved.
    int8_t  ofs_x;  /// \brief Offset along X axis.
    int8_t  ofs_y;  /// \brief Offset along Y axis.
    int8_t  ofs_z;  /// \brief Offset along Z axis.
};

/// \struct ADXL345Data
/// \brief Describes acceleration measurement as floating point (double) values.
///        Values are specified m/s^2.
struct ADXL345DataDbl {
    double x; /// \brief Acceleration along X axis
    double y; /// \brief Acceleration along Y axis
    double z; /// \brief Acceleration along Z axis
};

#pragma pack(pop)

/// \struct ADXL345Confiuration
/// \brief ADXL345 configuration structure.
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
/// \struct ADXL345Sample
/// \brief Describes single ADXL345 measurement (including timestamp)
struct ADXL345Sample {
    // The timestamp field must go the first, it is used during reading into the
    // following data member.
    struct timespec    timestamp; /// \brief Timestamp
    struct ADXL345Data data;      /// \brief Acceleration data, see \ref ADXL345Data.
};

/// \struct ADXL345SampleFP
/// \brief Describes single ADXL345 measurement (including timestamp) in
///        floating point format (double).
struct ADXL345SampleFP {
    struct timespec       timestamp; /// \brief Timestamp
    struct ADXL345DataDbl data;      /// \brief Acceleration data, in floating point format; see \ref ADXL345DataDbl.
};

/// \class ADXL345
/// \brief ADXL345 support.
class ADXL345 final : public EKitDeviceBase {
    /// \typedef super
    /// \brief Defines parent class
    typedef EKitDeviceBase  super;

    /// \brief Current device configuration
    ADXL345Confiuration     adxl_config;

    /// \brief maximim value in m/s^2
    double                  max_val    = 0.0;

    /// \brief Resolution scale factor
    double                  res_scale  = 1.0;

    /// \brief Free-fall acceleration on a planet Earth.
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

    /// \brief Enables/disables adxl345 chip
    /// \param enabled - true: enable, false: disable adxl345
    void    enable(bool enabled);

    /// \brief Configures module to sample acceleration data
    /// \param rate - sample rate, must be one of the \ref ADXL345Constants::BW_RATE_XXX values.
    /// \param watermark_samples - specifies amount samples in FIFO to indicate
    ///        dangerous level (it's time to read data from FIFO). This value should
    ///        match to \ref ADXL345Constants::FIFO_CTL_SAMPLES_MASK value.
    /// \param range - range of the accelerations being measured. Specify one of
    ///        the \ref ADXL345Constants::DATA_FORMAT_RANGE_XXg values.
    void    configure(uint8_t rate              = BW_RATE_100HZ,
                      uint8_t watermark_samples = FIFO_CTL_SAMPLES_DEFAULT,
                      uint8_t range             = DATA_FORMAT_RANGE_2g);

    /// \brief Reads single ADXL345 sample from the FIFO.
    /// \param data - pointer to the \ref ADXL345Sample structure.
    void    get_data(ADXL345Sample* data);

    /// \brief Returns amount of data in FIFO
    /// \param fifo_triggered - true returned if trigger event occurred, otherwise false.
    /// \return Number of entries stored in FIFO.
    size_t  get_data_len(bool& fifo_triggered);


    /// \brief Returns bitmask with events indicating ADXL345 state
    /// \return A bit mask of the values specified bt \ref ADXLEvents enum.
    uint8_t get_events();

    /// \brief Clears FIFO.
    void    clear_fifo();

    /// \brief returns offset data
    /// \param data - reference to struct to return offset data in.
    void    get_offset_data(ADXL345OffsetData& data);

    /// \brief set offset data
    /// \param data - reference to struct with offset data to be set.
    void    set_offset_data(ADXL345OffsetData& data);

    /// \brief Converts native accelerate data format to floating point values.
    /// \param int_data - reference to data in native format (int16_t)
    /// \param dbl_data - reference to a \ref ADXL345DataDbl to store data in floating point format.
    void to_double_data(const ADXL345Data& int_data, ADXL345DataDbl& dbl_data) const;

    /// \enum ADXL345Registers
    /// \brief Describes registers avilable in ADXL345
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

    /// \enum ADXL345Constants
    /// \brief Describes various constatns used with ADXL345.
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

    /// \enum ADXLEvents
    /// \brief Specify possible events (bit mask) to be returned by \ref ADXL345::get_events method.
    enum ADXLEvents : uint8_t {
        /// \brief Indicates data is ready is ready to be read from FIFO
        ADXL_EV_DATA_READY = 0b10000000,

        /// \brief The SINGLE_TAP bit is set when a single acceleration event that is greater than the value in the THRESH_TAP register.
        ADXL_EV_SINGLE_TAP = 0b01000000,

        /// \brief The DOUBLE_TAP bit is set when two acceleration events that are greater than the value in the THRESH_TAP register.
        ADXL_EV_DOUBLE_TAP = 0b00100000,

        /// \brief The activity bit is set when acceleration greater than the value stored in the THRESH_ACT register.
        ADXL_EV_ACTIVITY   = 0b00010000,

        /// \brief The inactivity bit is set when acceleration of less than the value stored in the THRESH_INACT register.
        ADXL_EV_INACTIVITY = 0b00001000,

        /// \brief The FREE_FALL bit is set when acceleration of less than the value stored in the THRESH_FF register.
        ADXL_EV_FREE_FALL  = 0b00000100,

        /// \brief The watermark bit is set when the number of samples in FIFO equals the value stored in the samples bits.
        ADXL_EV_WATERMARK  = 0b00000010,

        /// \brief The overrun bit is set when new data replaces unread data.
        ADXL_EV_OVERRUN    = 0b00000001
    };

   private:

    /// \brief Does single byte transaction to/from single single register
    /// \param read - true to read data from device register, false to write data
    ///        into device register
    /// \param addr - register address. One of the \ref ADXL345Registers values.
    /// \param data - reference to a byte with data to be read or written.
    /// \param timeout - timeout object for timeout control.
    /// \return Corresponding \ref EKIT_ERROR code.
    EKIT_ERROR single_byte_transaction(bool          read,
                                       const uint8_t addr,
                                       uint8_t&      data,
                                       EKitTimeout&  to);

    /// \brief Returns FIFO control mode information
    /// \param fifolen - specify how many FIFO entries will triggere corresponding event (depends on mode).
    /// \param mode - current mode selected, must be one of the \ref ADXL345Constants::FIFO_CTL_MODE_XXX constants.
    /// \param trigger - true links the trigger event of trigger mode to INT1, otherwise links to INT2.
    /// \param timeout - timeout object for timeout control.
    /// \return Corresponding \ref EKIT_ERROR code.
    EKIT_ERROR get_fifo_ctl_priv(size_t&      fifolen,
                                 uint8_t&     mode,
                                 bool&        trigger,
                                 EKitTimeout& to);

    /// \brief Set FIFO control mode
    /// \param fifolen - specify how many FIFO entries will triggere corresponding event (depends on mode).
    /// \param mode - current mode selected, must be one of the \ref ADXL345Constants::FIFO_CTL_MODE_XXX constants.
    /// \param trigger - true links the trigger event of trigger mode to INT1, otherwise links to INT2.
    /// \param timeout - timeout object for timeout control.
    /// \return Corresponding \ref EKIT_ERROR code.
    EKIT_ERROR set_fifo_ctl_priv(const size_t  fifolen,
                                 const uint8_t fifo_mode,
                                 const bool    trigger_int,
                                 EKitTimeout&  to);

    /// \brief Sets data format
    /// \param self_test - true to perform self test, otherwise false.
    /// \param three_wire_spi - true to use 3 wire spi, otherwise false (4 wire SPI).
    /// \param int_invert - true to set interrupts to active high, otherwise interrupts will be active low.
    /// \param full_res - if true the device is in full resolution mode, where
    ///        the output resolution increases with the g range set by the range
    ///        bits to maintain a 4 mg/LSB scale factor. False instructs the device
    ///        to work in 10-bit mode, and the range bits determine the maximum g
    ///        range and scale factor.
    /// \param justify_msb - True selects left-justified (MSB) mode, and false
    ///        selects right-justified mode with sign extension.
    /// \param range - Specifies measurement range. Use one of these constants
    ///        \ref ADXL345Constants::DATA_FORMAT_RANGE_XXg
    /// \param timeout - timeout object for timeout control.
    /// \return Corresponding \ref EKIT_ERROR code.
    EKIT_ERROR set_data_format_priv(bool         self_test,
                                    bool         three_wire_spi,
                                    bool         int_invert,
                                    bool         full_res,
                                    bool         justify_msb,
                                    uint8_t      range,
                                    EKitTimeout& to);

    /// \brief Returns data format
    /// \param self_test - Reference to self-test value: true indicates self test, otherwise false.
    /// \param three_wire_spi - Reference to SPI mode: true indicates 3 wire spi, otherwise false (4 wire SPI).
    /// \param full_res - Reference to full resolution mode: if true the device is in full resolution mode, where
    ///        the output resolution increases with the g range set by the range
    ///        bits to maintain a 4 mg/LSB scale factor. False instructs the device
    ///        to work in 10-bit mode, and the range bits determine the maximum g
    ///        range and scale factor.
    /// \param justify_msb - Reference to justification mode value: true selects left-justified (MSB) mode, and false
    ///        selects right-justified mode with sign extension.
    /// \param range - Reference to measured value range: One of these constants are returned:
    ///        \ref ADXL345Constants::DATA_FORMAT_RANGE_XXg
    /// \param timeout - timeout object for timeout control.
    /// \return Corresponding \ref EKIT_ERROR code.
    EKIT_ERROR get_data_format_priv(bool&        self_test,
                                    bool&        three_wire_spi,
                                    bool&        int_invert,
                                    bool&        full_res,
                                    bool&        justify_msb,
                                    uint8_t&     range,
                                    EKitTimeout& to);

    /// \brief Sets data flow rate
    /// \param low_power - false selects normal operation, true selects reduced power operation, which has
    ///                    somewhat higher noise
    /// \param rate - data flow rate, specify one of the values: \ref ADXL345Constants::BW_RATE_XXXHz
    /// \param timeout - timeout object for timeout control.
    /// \return Corresponding \ref EKIT_ERROR code.
    EKIT_ERROR set_rate_priv(bool low_power, uint8_t rate, EKitTimeout& to);

    /// \brief Returns data flow rate
    /// \param low_power - Reference to low power value: false selects normal operation,
    ///        true selects reduced power operation, which has somewhat higher noise.
    /// \param rate - Reference to data flow rate, returned one of the values: \ref ADXL345Constants::BW_RATE_XXXHz
    /// \param timeout - timeout object for timeout control.
    /// \return Corresponding \ref EKIT_ERROR code.
    EKIT_ERROR get_rate_priv(bool& low_power, uint8_t& rate, EKitTimeout& to);

    /// \brief Set power control
    /// \param link - True with both the activity and inactivity functions enabled
    ///        delays the start of the activity function until inactivity is detected.
    /// \param auto_sleep - If the link bit is set and true is specified enables
    ///        the auto-sleep functionality.
    /// \param measure - False places the part into standby mode, and true places
    ///        the part into measurement mode.
    /// \param sleep - false puts the part into the normal mode of operation,
    ///        and true places the part into sleep mode.
    /// \param wakeup_rate - control the frequency of readings in sleep mode. One
    ///        of these values must be specified \ref ADXL345Constants::POWER_CTL_WAKEUP_XHZ.
    /// \param timeout - timeout object for timeout control.
    /// \return Corresponding \ref EKIT_ERROR code.
    EKIT_ERROR set_power_ctl_priv(bool         link,
                                  bool         auto_sleep,
                                  bool         measure,
                                  bool         sleep,
                                  uint8_t      wakeup_rate,
                                  EKitTimeout& to);

    /// \brief Returns current power control
    /// \param link - Reference to link value: true with both the activity and
    ///        inactivity functions enabled delays the start of the activity
    ///        function until inactivity is detected.
    /// \param auto_sleep - Reference to auto sleep value: if the link bit is
    ///        set and true is specified enables the auto-sleep functionality.
    /// \param measure - Reference to measure value: false places the part into
    ///        standby mode, and true places the part into measurement mode.
    /// \param sleep - Reference to sleep value: false puts the part into the
    ///        normal mode of operation, and true places the part into sleep mode.
    /// \param wakeup_rate - Reference to wakeup rate control value: One
    ///        of these values are returned \ref ADXL345Constants::POWER_CTL_WAKEUP_XHZ.
    /// \param timeout - timeout object for timeout control.
    /// \return Corresponding \ref EKIT_ERROR code.
    EKIT_ERROR get_power_ctl_priv(bool&        link,
                                  bool&        auto_sleep,
                                  bool&        measure,
                                  bool&        sleep,
                                  uint8_t&     wakeup_rate,
                                  EKitTimeout& to);

    /// \brief Verifies device id
    /// \param timeout - timeout object for timeout control.
    /// \return Corresponding \ref EKIT_ERROR code.
    EKIT_ERROR check_device_id_priv(EKitTimeout& to);

    /// \brief Reads device configuration
    /// \param config - reference to configuration to be read.
    /// \param timeout - timeout object for timeout control.
    /// \return Corresponding \ref EKIT_ERROR code.
    EKIT_ERROR read_config_priv(ADXL345Confiuration& config, EKitTimeout& to);

    /// \brief Sets device configuration
    /// \param config - reference to configuration to be set.
    /// \param timeout - timeout object for timeout control.
    /// \return Corresponding \ref EKIT_ERROR code.
    EKIT_ERROR write_config_priv(ADXL345Confiuration& config, EKitTimeout& to);
};

/// @}
