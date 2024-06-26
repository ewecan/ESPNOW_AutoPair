
/**
 * @file main.cpp
 * @author  Vecang
 * @brief Server 发送
 * @version 0.1
 * @date 2024-05-27
 *
 * @copyright Copyright (c) 2024
 *
 */
#include <Arduino.h>

#include "EspnowManager.h"

static EspnowManager Device(ROLE_SERVER);
static eEspnowStatus_t runMode;

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
  if (status == ESP_NOW_SEND_SUCCESS)
  {
    char mac_str[18];
    snprintf(mac_str, 18, "%02x:%02x:%02x:%02x:%02x:%02x", mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
    Serial.printf("[%s]: Send Success!\n", mac_str);
  }
}

void setup()
{
  Serial.begin(115200);

  unsigned long lastSearchTime = millis();
  while (runMode != STATUS_CONNECTED)
  {
    runMode = Device.init("uMind"); // 设置 USB适配器名称, 设备名称, 需要配对的客户端数量
    if (Device.getAlreadyPairClinentNumber() > 0 || (millis() - lastSearchTime) > 3000)
      break;
  }

  Serial.println("MAC:" + WiFi.macAddress());
  Device.setRegisterSendCB(OnDataSent); // 发送数据回调函数
}

void loop()
{

  uint8_t data[16] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};
  // 发送数据
  // Device.write(data, sizeof(data));
  // Device.writeTo(0, data, sizeof(data)

  // 发送数据 
  Device.print("Hello World"); 

  delay(1000);
}
