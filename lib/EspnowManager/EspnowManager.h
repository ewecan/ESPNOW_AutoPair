/**
 * @file EspnowManager.h
 * @author  Vecang
 * @brief 实现ESPNOW 自动连接对应名称的设备
 * @version 0.2
 * @date 2024-05-14
 *
 * @copyright Copyright (c) 2024
 *
 */
#ifndef __DEV_ESPNOW_H__
#define __DEV_ESPNOW_H__

#include <esp_now.h>
#include <WiFi.h>
#include <vector>

#include "Print.h"

#define ESPNOW_CHANNEL 1
#define ESPNOW_CLIENT_NUMBER_MAX 5

typedef enum
{
    STATUS_ERROR,
    STATUS_UNCONNECTED, // 未连接
    STATUS_CONNECTED,   // 已连接
    STATUS_MAX,         // ---
} eEspnowStatus_t;

typedef enum
{
    ROLE_CLIENT, // 客户端( USB )
    ROLE_SERVER, // 服务端( 设备)
    ROLE_MAX,
} eEspnowRole_t;

class EspnowManager : public Print
{
public:
    EspnowManager(eEspnowRole_t role) { _role = role; }
    virtual ~EspnowManager(){};

    eEspnowStatus_t init(const String &deviceName);
    eEspnowStatus_t init(const String &deviceName, uint8_t needPairClinentNumber);
    eEspnowStatus_t init(const String &deviceName, const String &deviceName2, uint8_t needPairClinentNumber);
    eEspnowStatus_t init(const std::vector<String> &strs, uint8_t needPairClinentNumber);

    void setRegisterSendCB(esp_now_send_cb_t cb);
    void setRegisterRecvCB(esp_now_recv_cb_t cb);

    void setConnectFlag(bool isConnect);
    bool getPairFlag();
    bool getConnectFlag();

    size_t writeTo(uint8_t clientID, uint8_t c);
    size_t writeTo(uint8_t clientID, const uint8_t *data, size_t len);
    size_t write(uint8_t c);
    size_t write(const uint8_t *data, size_t len);

    size_t printTo(uint8_t clientID, const String &s);

    uint8_t getAlreadyPairClinentNumber();

private:
    eEspnowRole_t _role;
    bool _isConnect;
    bool _isPairUSB = false;
    uint8_t _needPairClinentNumber;
    uint8_t _alreadyPairClientNumber;
    unsigned long _lastTime;

    void ScanForSlave(const String &str);
    void ScanForSlave(const String &str, const String &str2);
    void ScanForSlave(const std::vector<String> &strs);

    esp_now_peer_info_t _slaves[ESPNOW_CLIENT_NUMBER_MAX];
};

#endif //__DEV_ESPNOW_H__
