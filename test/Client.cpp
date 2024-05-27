/**
 * @file main.cpp
 * @author  Vecang
 * @brief Client 接收
 * @version 0.1
 * @date 2024-05-27
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#include <Arduino.h>

#include "EspnowManager.h"

static EspnowManager Device(ROLE_CLIENT);
static eEspnowStatus_t runMode;

void OnDataRecv(const uint8_t *mac_addr, const uint8_t *data, int data_len)
{
  char mac_str[18];
  snprintf(mac_str, 18, "%02x:%02x:%02x:%02x:%02x:%02x", mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.printf("\n[%s]:", mac_str);
  Serial.write(data, data_len);
}

void setup()
{
  Serial.begin(115200);

  unsigned long lastSearchTime = millis();
  while (runMode != STATUS_CONNECTED)
  {
    runMode = Device.init("uMind"); // 设置 USB适配器名称, 设备名称, 需要配对的客户端数量
  }

  Serial.println("MAC:" + WiFi.macAddress());
  Device.setRegisterRecvCB(OnDataRecv);
}

void loop()
{
}
