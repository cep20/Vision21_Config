/**
 ******************************************************************************
 * @addtogroup Config
 * @{
 * @file    Config
 * @author  Client Microdata
 * @brief   Header for the Config module.
 *
 * @date 5/3/2021 9:19:24 AM
 *
 ******************************************************************************
 */
#ifndef _Config
#define _Config

/*****************************************************************************/
/* Includes */
#include <string>
#include <array>

/*****************************************************************************/
/* Exported defines */


/*****************************************************************************/
/* Exported macro */


/*****************************************************************************/
/* Exported types */
using TruckTypes = enum {
    TT_SemiTrailer = 0,
    TT_Train       = 1,
    TT_10Wheels    = 2,
    TT_12Wheels    = 3,
    TT_Cube        = 4
};

using SensorTypes = enum {
    ST_NotConnected = 0,
    ST_Pneumatic    = 1,
    ST_Mechanical   = 2,
    ST_Inclinometer = 3,
};

using Units = enum {
    U_Metric   = 0,
    U_Imperial = 1,
};

using Languages = enum {
    L_French  = 0,
    L_English = 1,
};

struct Config
{
    Config(const uint8_t* data, size_t len);
    Config() = default;

    size_t Serialize(uint8_t** outData);

    size_t GetSize() const;

    uint32_t    sn                  = 0;
    std::string btName              = std::string(32, '\0');
    uint8_t     truckType           = TT_SemiTrailer;
    std::string lastCalibDate       = "00-00-00";
    bool        isLoraEn            = false;
    bool        isLogToFileEn       = true;
    bool        isDisplayEn         = false;
    bool        isIsaacEn           = false;
    bool        isRelayOutEn        = false;
    bool        isDutyBoxEn         = false;
    uint8_t     dutyBoxSpeedLimit   = 12;    // Default: 12km/h.
    uint8_t     sensor1Type         = ST_NotConnected;
    uint8_t     sensor2Type         = ST_NotConnected;
    uint8_t     sensor3Type         = ST_NotConnected;
    uint8_t     sensor4Type         = ST_NotConnected;
    uint8_t     lineA               = 0x00;    // Upper 4 bits: cA, lower 4 bits: cB
    uint8_t     lineB               = 0x00;    // Upper 4 bits: cA, lower 4 bits: cB
    uint8_t     lineC               = 0x00;    // Upper 4 bits: cA, lower 4 bits: cB
    uint8_t     samplesToTake       = 8;
    bool        isTempSensorPresent = false;
    uint8_t     unitType            = U_Metric;
    uint8_t     language            = L_French;
};

/*****************************************************************************/
/* Exported functions */


/* Have a wonderful day :) */
#endif /* _Config */
/**
 * @}
 */
/****** END OF FILE ******/
