/**
 * @file EspnowManager.cpp
 * @author  Vecang
 * @brief 实现ESPNOW 自动连接对应名称的设备
 * @version 0.2
 * @date 2024-05-14
 *
 * @copyright Copyright (c) 2024
 *
  unsigned long lastSearchTime = millis();
  while (runMode != STATUS_CONNECTED)
  {
    runMode = Device.init(CLIENT_USB_NAME, CLIENT_DEVICE_NAME, 2); // 设置 USB适配器名称, 设备名称, 需要配对的客户端数量
    if (Device.getAlreadyPairClinentNumber() > 0 && (millis() - lastSearchTime) > 8000)
      break;
  }
  Device.setRegisterSendCB(OnDataSent); // 发送数据回调函数

 */
#include "EspnowManager.h"
#include <vector>

eEspnowStatus_t EspnowManager::init(const String &deviceName)
{
    eEspnowStatus_t eRunStatus;
    if (_role == ROLE_SERVER)
    {
        eRunStatus = init(deviceName, 1);
    }
    else if (_role == ROLE_CLIENT)
    {
        WiFi.mode(WIFI_AP);

        String Prefix = deviceName;
        String SSID = Prefix;
        String Password = "FFFFFFFAQ";
        bool result = WiFi.softAP(SSID.c_str(), Password.c_str(), ESPNOW_CHANNEL, 0);
        if (result)
            Serial.println("ESPNOW Client: " + String(SSID));
        WiFi.disconnect();
        if (esp_now_init() != ESP_OK)
        {
            eRunStatus = STATUS_ERROR;
        }
    }
    return eRunStatus;
}

eEspnowStatus_t EspnowManager::init(const String &deviceName, uint8_t needPairClinentNumber)
{
    std::vector<String> _deviceName = {deviceName};
    return init(_deviceName, needPairClinentNumber);
}

eEspnowStatus_t EspnowManager::init(const String &deviceName1, const String &deviceName2, uint8_t needPairClinentNumber)
{
    std::vector<String> _deviceNames = {deviceName1, deviceName2};
    return init(_deviceNames, needPairClinentNumber);
}

eEspnowStatus_t EspnowManager::init(const std::vector<String> &strs, uint8_t needPairClinentNumber)
{
    eEspnowStatus_t eRunStatus = STATUS_ERROR;
    if (_role == ROLE_SERVER)
    {
        _needPairClinentNumber = needPairClinentNumber;
        WiFi.mode(WIFI_STA);
        WiFi.disconnect();
        if (esp_now_init() != ESP_OK)
        {
            Serial.println("Failed to initialize ESP-NOW");
            eRunStatus = STATUS_ERROR;
            return eRunStatus;
        }

        if (millis() - _lastTime > 1000)
        {
            // Serial.println(WiFi.macAddress());
            _lastTime = millis();
            if (_needPairClinentNumber != _alreadyPairClientNumber)
            {
                // 调用 ScanForSlave 函数并传递传入的字符串向量作为参数
                ScanForSlave(strs);
                eRunStatus = STATUS_UNCONNECTED;
            }
            else
            {
                eRunStatus = STATUS_CONNECTED;
                _isPairUSB = true;
            }
        }
    }
    return eRunStatus;
}

uint8_t EspnowManager::getAlreadyPairClinentNumber()
{
    return _alreadyPairClientNumber;
}

void EspnowManager::setRegisterSendCB(esp_now_send_cb_t cb)
{
    esp_now_register_send_cb(cb);
}

void EspnowManager::setRegisterRecvCB(esp_now_recv_cb_t cb)
{
    esp_now_register_recv_cb(cb);
}

void EspnowManager::setConnectFlag(bool isConnect)
{
    _isConnect = isConnect;
}

bool EspnowManager::getConnectFlag()
{
    return _isConnect;
}

bool EspnowManager::getPairFlag()
{
    return _isPairUSB;
}

void EspnowManager::ScanForSlave(const String &str)
{
    std::vector<String> PairName = {str};
    ScanForSlave(PairName);
}

void EspnowManager::ScanForSlave(const String &str, const String &str2)
{
    std::vector<String> PairName = {str, str2};
    ScanForSlave(PairName);
}

void EspnowManager::ScanForSlave(const std::vector<String> &strs)
{
    int8_t scanNumber = WiFi.scanNetworks();

    for (int i = 0; i < scanNumber; ++i)
    {
        // 打印找到的每个设备的 SSID 和 RSSI
        String SSID = WiFi.SSID(i);
        String BSSIDstr = WiFi.BSSIDstr(i);

        Serial.println(String(i + 1) + ": " + SSID + " [" + BSSIDstr + "]");
        delay(10);
        for (const auto &str : strs)
        {
            // 检查当前设备是否以 str 开头
            if (SSID.indexOf(str) == 0)
            {
                int mac[6];
                if (6 == sscanf(BSSIDstr.c_str(), "%x:%x:%x:%x:%x:%x", &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]))
                {
                    for (int ii = 0; ii < 6; ++ii)
                    {
                        _slaves[_alreadyPairClientNumber].peer_addr[ii] = (uint8_t)mac[ii];
                    }
                }
                _slaves[_alreadyPairClientNumber].channel = ESPNOW_CHANNEL; // 选择一个频道
                _slaves[_alreadyPairClientNumber].encrypt = false;          // 不加密

                Serial.print(">> Num" + String(_alreadyPairClientNumber) + ":");
                char macStr[18];
                sprintf(macStr, "%02x:%02x:%02x:%02x:%02x:%02x", _slaves[_alreadyPairClientNumber].peer_addr[0], _slaves[_alreadyPairClientNumber].peer_addr[1], _slaves[_alreadyPairClientNumber].peer_addr[2], _slaves[_alreadyPairClientNumber].peer_addr[3], _slaves[_alreadyPairClientNumber].peer_addr[4], _slaves[_alreadyPairClientNumber].peer_addr[5]);
                Serial.println(macStr);
                Serial.println("----------------------------------");

                // 检查peer是否存在
                bool exists = esp_now_is_peer_exist(_slaves[_alreadyPairClientNumber].peer_addr);
                if (!exists)
                {
                    esp_now_add_peer(&(_slaves[_alreadyPairClientNumber]));
                    _alreadyPairClientNumber++;
                    delay(100);
                }
            }
        }
    }
    WiFi.scanDelete();
}

size_t EspnowManager::writeTo(uint8_t clientID, uint8_t c)
{
    esp_err_t result;
    result = writeTo(clientID, c);
    return result;
}

size_t EspnowManager::writeTo(uint8_t clientID, const uint8_t *buffer, size_t size)
{
    esp_err_t result;
    if (clientID <= _alreadyPairClientNumber)
    {
        const uint8_t *peer_addr = _slaves[clientID].peer_addr;
        result = esp_now_send(peer_addr, buffer, size);
    }
    else
    {
        result = ESP_FAIL;
    }
    return result;
}

size_t EspnowManager::write(uint8_t c)
{
    esp_err_t result = write(c);
    return result;
}

size_t EspnowManager::write(const uint8_t *data, size_t len)
{
    esp_err_t result;
    for (int i = 0; i < _alreadyPairClientNumber; i++)
    {
        esp_err_t resultTemp = writeTo(i, data, len);
        result = resultTemp && result;
    }
    return result;
}

size_t EspnowManager::printTo(uint8_t clientID, const String &s)
{
    esp_err_t result;
    const uint8_t *buffer = (const uint8_t *)s.c_str();
    size_t size = s.length();
    result = writeTo(clientID, buffer, size);
    return result;
}