/**
  * @file   temp.cpp
  * @brief  Get temperature of OneWire sensors
  * @author David DEVANT
  * @date   12/08/2018
  */

#include <OneWire.h>
#include <DallasTemperature.h>

#include "global.hpp"
#include "temp.hpp"
#include "domoticz/domoticz.hpp"
#include "status_led/status_led.hpp"

#ifdef MODULE_TEMPERATURE

extern uint32_t tick;
uint32_t tempNextMesureTick = 0;

OneWire oneWire(TEMP_1_WIRE_PIN);
DallasTemperature sensors(&oneWire);
// Stockage des adresses des thermometres
byte sensorCount = 0;
DeviceAddress sensorAddrList[TEMP_MAX_SENSOR_SUPPORTED] = {};

float sensorValue[TEMP_MAX_SENSOR_SUPPORTED] = {};

char * sensor_addr_to_string(char * str, DeviceAddress address)
{
  char * strOld = str;

  for (byte index = 0; index < sizeof(address); index++) {
    if (index > 0) {
      str += sprintf(str, "-");
    }
    str += sprintf(str, "%02X", address[index]);
  }
  *str = '\0';

  return strOld;
}

/**
 * @brief Search for new addresses on the OneWire bus
 * Results are saved in addressList
 * @param maxLen: maximum number of address to find
 * @return The number of address found
 */
byte get_onewire_address(DeviceAddress addressList[], byte maxLen)
{
  byte index = 0;

  while (true) {
    if (oneWire.search(addressList[index]) != 1) {
      oneWire.reset_search();
      break;
    } else {
      // Count this new sensor
      ++index;
      // Do we have reach the limit ?
      if (index >= maxLen) {
        break;
      }
    }
    delay(100);
  }

  return index;
}

/**
 * @brief Write the thermometers address into sensorAddrList
 * @note It can write 1 or 2 addresses
 */
void get_sensor_address(void)
{
  DeviceAddress addressList[TEMP_MAX_SENSOR_SUPPORTED];

  sensorCount = get_onewire_address(addressList, TEMP_MAX_SENSOR_SUPPORTED);

  // ONLY FOR DEBUG
  // sensorCount = 2;
  // for (byte index = 0; index < sizeof(DeviceAddress); index++) {
  //   addressList[0][index] = 2 * index;
  //   addressList[1][index] = 10 * index;
  // }

  if (sensorCount == 0) {
    log_warn("No sensor found");
  } else if (sensorCount == 1) {
    memcpy(sensorAddrList[0], addressList[0], sizeof(DeviceAddress));
  }
  else if (sensorCount == 2) {
    // The addresses smaller goes in first position
    for (byte index = 0; index < sizeof(DeviceAddress); index++) {
      if (addressList[0][index] > addressList[1][index]) {
        memcpy(sensorAddrList[0], addressList[0], sizeof(DeviceAddress));
        memcpy(sensorAddrList[1], addressList[1], sizeof(DeviceAddress));
      } else if (addressList[0][index] < addressList[1][index]) {
        memcpy(sensorAddrList[0], addressList[1], sizeof(DeviceAddress));
        memcpy(sensorAddrList[1], addressList[0], sizeof(DeviceAddress));
      } else {
        continue;
      }
      break; // If we copied, leave loop
    }
  } else {
    log_error("Unsupported sensor number: %d", sensorCount);
  }
}

/**
 *  Read the value of the specified sensor and send it
 **/
void manage_sensor(uint8_t deviceIndex, DeviceAddress deviceAddress) {

  // ONLY FOR DEBUG
  // sensorValue[deviceIndex] = (deviceIndex == 0) ? 14.0 : 28.1;
  // return;

  float degreesValue = sensors.getTempC(deviceAddress);

  if (degreesValue == DEVICE_DISCONNECTED_C) {
    _set(STATUS_TEMP, STATUS_TEMP_1_FAULT << deviceIndex);
    log_error("Sensor %d: error getting temperature", deviceIndex);
    return;
  }

  _unset(STATUS_TEMP, STATUS_TEMP_1_FAULT << deviceIndex);

  // Save data
  sensorValue[deviceIndex] = degreesValue;

  // Log
  log_info("Sensor %d is %.2f°C", deviceIndex, degreesValue);
}

/**
 * @brief Get the last value read for the specified sensor
 *
 * @param deviceIndex The index of the sensor
 * @return Sensor value in celcius or DEVICE_DISCONNECTED_C
 */
float temp_get_value(uint8_t deviceIndex)
{
  if (deviceIndex >= TEMP_MAX_SENSOR_SUPPORTED) {
    return DEVICE_DISCONNECTED_C;
  }

  return sensorValue[deviceIndex];
}

/**
 * @brief Get the address of the sensor into a string
 *
 * @param str string to fill
 * @param deviceIndex The index of the sensor
 *
 * @return The input string
 */
char * temp_get_address(char * str, uint8_t deviceIndex)
{
  if (deviceIndex >= TEMP_MAX_SENSOR_SUPPORTED) {
    str[0] = '\0';
  } else {
    sensor_addr_to_string(str, sensorAddrList[deviceIndex]);
  }
  return str;
}

uint8_t temp_get_nb_sensor(void)
{
  return (uint8_t) sensorCount;
}

void temp_init(void)
{
  char strTemp[3 * 8 + 1];
  log_info("Locating OneWire devices... ");
  get_sensor_address();

  sensors.begin();

  for (byte sensorIndex = 0; sensorIndex < sensorCount; sensorIndex++) {
    // Print device name and address
    log_info("Device [%d/%d]: %s", sensorIndex + 1, sensorCount, sensor_addr_to_string(strTemp, sensorAddrList[sensorIndex]));

    // Vérifie si les capteurs sont connectés | check and report if sensors are conneted
    if (!sensors.getAddress(sensorAddrList[sensorIndex], sensorIndex)) {
      log_error("Unable to find address for Device %d", sensorIndex + 1);
      continue;
    }
    // set the resolution to 9 bit per device
    sensors.setResolution(sensorAddrList[sensorIndex], TEMP_SENSOR_RESOLUTION);

    // On vérifie que le capteur est correctement configuré | Check that sensor is correctly configured
    log_info("Resolution: %d bits", sensors.getResolution(sensorAddrList[sensorIndex]));
  }
}

void temp_main(void)
{
  if (tick > tempNextMesureTick) {
    tempNextMesureTick = tick + TEMP_POLLING_PERIOD_MS;

    if (sensorCount > 0) {
      sensors.requestTemperatures();

      // print the device information
      for (byte sensorIndex = 0; sensorIndex < sensorCount; sensorIndex++) {
        manage_sensor(sensorIndex, sensorAddrList[sensorIndex]);
      }
    }
  }
}

#endif