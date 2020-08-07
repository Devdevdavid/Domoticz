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

char * get_device_address(char * str, DeviceAddress address)
{
  char * strOld = str;

  for (byte index = 0; index < sizeof(address); index++) {
    str += sprintf(str, "%02X ", address[index]);
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
  if (sensorCount == 0) {
    log_warn("No sensor found");

    // Set both sensors as unused
    _unset(STATUS_TEMP, STATUS_TEMP_1_CONN | STATUS_TEMP_1_CONN);
  } else if (sensorCount == 1) {
    memcpy(sensorAddrList[0], addressList[0], sizeof(DeviceAddress));

    // Set sensor 2 as unused
    _set(STATUS_TEMP, STATUS_TEMP_1_CONN);
    _unset(STATUS_TEMP, STATUS_TEMP_2_CONN);
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

    // Set both sensors as connected
    _set(STATUS_TEMP, STATUS_TEMP_1_CONN | STATUS_TEMP_1_CONN);
  } else {
    log_error("Unsupported sensor number: %d", sensorCount);
  }
}

/**
 *  Read the value of the specified sensor and send it
 **/
void manage_sensor(uint8_t deviceIndex, DeviceAddress deviceAddress){
  float degreesValue = sensors.getTempC(deviceAddress);

  if (degreesValue == DEVICE_DISCONNECTED_C) {
    switch(deviceIndex) {
      case 0: _set(STATUS_TEMP, STATUS_TEMP_1_FAULT); break;
      case 1: _set(STATUS_TEMP, STATUS_TEMP_2_FAULT); break;
      default: return;
    }
    log_error("Sensor %d: error getting temperature", deviceIndex);
    return;
  }

  _unset(STATUS_TEMP, (deviceIndex == 0) ? STATUS_TEMP_1_FAULT : STATUS_TEMP_2_FAULT);

  // Save data
  sensorValue[deviceIndex] = degreesValue;

  // Log
  log_info("Sensor %d is %.2f°C", deviceIndex, degreesValue);
}

float temp_get_value(uint8_t deviceIndex)
{
  if (deviceIndex >= TEMP_MAX_SENSOR_SUPPORTED) {
    return DEVICE_DISCONNECTED_C;
  }

  return sensorValue[deviceIndex];
}

byte temp_get_nb_sensor(void)
{
  return (byte) sensorCount;
}

void temp_init(void)
{
  char strTemp[20];
  log_info("Locating devices... ");
  get_sensor_address();

  sensors.begin();

  for (byte sensorIndex = 0; sensorIndex < sensorCount; sensorIndex++) {
    // Print device name and address
    log_info("Device [%d/%d]: %s", sensorIndex + 1, sensorCount, get_device_address(strTemp, sensorAddrList[sensorIndex]));

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