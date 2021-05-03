#include "Config.h"

#include "Brigerad.h"

#include <cstring>

#define INC_PTR(var)                                                                               \
    do                                                                                             \
    {                                                                                              \
        BR_ASSERT((dataPtr + sizeof((var)) < endPtr), "Not enough data!");                         \
        dataPtr += sizeof((var));                                                                  \
    } while (0)

Config::Config(const uint8_t* data, size_t len)
{
    BR_ASSERT(data != nullptr, "Pointer is null!");
    BR_ASSERT(len >= 0, "Len is zero");

    const uint8_t* dataPtr = reinterpret_cast<const uint8_t*>(data);
    const uint8_t* endPtr  = reinterpret_cast<const uint8_t*>(data) + len;

    sn = (dataPtr[0] << 24) | (dataPtr[1] << 16) | (dataPtr[2] << 8) | (dataPtr[3]);
    INC_PTR(sn);

    btName = std::string((const char*)dataPtr);
    dataPtr += btName.size() + 1;    // std::string::size doesn't include the null-terminator.
    BR_ASSERT(dataPtr < endPtr, "Not enough data!");

    truckType = *dataPtr;
    INC_PTR(truckType);

    lastCalibDate = std::string((const char*)dataPtr);
    dataPtr +=
      lastCalibDate.size() + 1;    // std::string::size doesn't include the null-terminator.
    BR_ASSERT(dataPtr < endPtr, "Not enough data!");

    isLoraEn = *reinterpret_cast<const bool*>(dataPtr);
    INC_PTR(isLoraEn);

    isLogToFileEn = *reinterpret_cast<const bool*>(dataPtr);
    INC_PTR(isLogToFileEn);

    isDisplayEn = *reinterpret_cast<const bool*>(dataPtr);
    INC_PTR(isDisplayEn);

    isIsaacEn = *reinterpret_cast<const bool*>(dataPtr);
    INC_PTR(isIsaacEn);

    isRelayOutEn = *reinterpret_cast<const bool*>(dataPtr);
    INC_PTR(isRelayOutEn);

    isDutyBoxEn = *reinterpret_cast<const bool*>(dataPtr);
    INC_PTR(isDutyBoxEn);

    dutyBoxSpeedLimit = *dataPtr;
    INC_PTR(dutyBoxSpeedLimit);

    sensor1Type = *dataPtr;
    INC_PTR(sensor1Type);

    sensor2Type = *dataPtr;
    INC_PTR(sensor2Type);

    sensor3Type = *dataPtr;
    INC_PTR(sensor3Type);

    sensor4Type = *dataPtr;
    INC_PTR(sensor4Type);

    lineA = *dataPtr;
    INC_PTR(lineA);

    lineB = *dataPtr;
    INC_PTR(lineB);

    lineC = *dataPtr;
    INC_PTR(lineC);

    samplesToTake = *dataPtr;
    INC_PTR(samplesToTake);

    isTempSensorPresent = *reinterpret_cast<const bool*>(dataPtr);
    INC_PTR(isTempSensorPresent);

    unitType = *dataPtr;
    INC_PTR(unitType);

    language = *dataPtr;
    INC_PTR(language);

    BR_TRACE("Done loading data! {} bytes", endPtr - dataPtr);
}


size_t Config::Serialize(uint8_t** outData)
{
    uint8_t* data = new uint8_t[GetSize()];
    size_t   len  = 0;

    data[len++] = (uint8_t)(sn >> 24) & 0x000000FF;
    data[len++] = (uint8_t)(sn >> 16) & 0x000000FF;
    data[len++] = (uint8_t)(sn >> 8) & 0x000000FF;
    data[len++] = (uint8_t)sn & 0x000000FF;

    for (const auto& c : btName)
    {
        data[len++] = c;
    }
    // Add null-terminator.
    data[len++] = '\0';

    data[len++] = truckType;

    for (const auto& c : lastCalibDate)
    {
        data[len++] = c;
    }
    // Add null-terminator.
    data[len++] = '\0';

    data[len++] = isLoraEn;
    data[len++] = isLogToFileEn;
    data[len++] = isDisplayEn;
    data[len++] = isIsaacEn;
    data[len++] = isRelayOutEn;
    data[len++] = isDutyBoxEn;
    data[len++] = dutyBoxSpeedLimit;

    data[len++] = sensor1Type;
    data[len++] = sensor2Type;
    data[len++] = sensor3Type;
    data[len++] = sensor4Type;

    data[len++] = lineA;
    data[len++] = lineB;
    data[len++] = lineC;

    data[len++] = samplesToTake;
    data[len++] = isTempSensorPresent;
    data[len++] = unitType;
    data[len++] = language;

    *outData = data;
    return len;
}


size_t Config::GetSize() const
{
    // Total size in bytes.
    size_t s = sizeof(Config);

    // Remove the two strings, since we want the number of chars, not the size of std::string.
    s -= sizeof(std::string) * 2;

    // Add the number of bytes contained in each strings.
    s += btName.size();
    s += lastCalibDate.size();

    return s;
}
