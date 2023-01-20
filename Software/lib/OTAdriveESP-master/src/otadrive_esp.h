#ifndef _OTADRIVE_ESP_H_
#define _OTADRIVE_ESP_H_

#include <Arduino.h>
#include <FS.h>

#ifdef ESP8266
#include <ESP8266httpUpdate.h>
#include <LittleFS.h>
#define updateObj ESPhttpUpdate
#define OTA_FILE_SYS LittleFS
#elif defined(ESP32)
#include <HTTPUpdate.h>
#include <SPIFFS.h>
#define updateObj httpUpdate
#define OTA_FILE_SYS SPIFFS
#endif

#ifndef OTADRIVE_URL
#define OTADRIVE_URL "http://otadrive.com/deviceapi/"
#endif

class otadrive_ota;
class updateInfo;

enum class update_result : int
{
    ConnectError = 0,
    DeviceUnauthorized = 401,
    AlreadyUpToDate = 304,
    NoFirmwareExists = 404,
    Success = 200
};

class updateInfo : public Printable
{
    const char *code_str() const;
    String old_version;

public:
    bool available;
    String version;
    int size;
    update_result code;

    virtual size_t printTo(Print &p) const;
    String toString() const;

    friend class otadrive_ota;
};

class device_status
{
    float battery_voltage;
    bool power_plugged;
    double latitude;
    double longitude;
};

class otadrive_ota
{
private:
    uint32_t tickTimestamp = 0;
    const uint16_t TIMEOUT_MS = 10000;
    String ProductKey;
    String Version;

    String cutLine(String &str);
    String baseParams();
    bool download(String url, File *file, String *outStr);
    update_result head(String url, String &resultStr, const char *reqHdrs[1], uint8_t reqHdrsCount);
    String file_md5(File &f);
    String downloadResourceList();

    static void updateFirmwareProgress(int progress, int totalt);

public:
    typedef std::function<void(size_t, size_t)> THandlerFunction_Progress;
    FS *fileObj;

    otadrive_ota();
    void setInfo(String ApiKey, String Version);
    String getChipId();

    bool sendAlive();

    updateInfo updateFirmware(bool reboot = true);
    updateInfo updateFirmwareInfo();
    void onUpdateFirmwareProgress(THandlerFunction_Progress fn);

    bool syncResources();
    void setFileSystem(FS *fileObj);
    String getConfigs();

    bool timeTick(uint16_t seconds);

private:
    static THandlerFunction_Progress _progress_callback;
};

extern otadrive_ota OTADRIVE;

#define otd_pre "[OTAdrive]"
#ifdef ESP8266
#if ARDUHAL_LOG_LEVEL >= 4
#define otd_log_d(format, ...) Serial.printf("[D]" otd_pre format "\n", ##__VA_ARGS__)
#else
#define otd_log_d(format, ...)
#endif
#if ARDUHAL_LOG_LEVEL >= 3
#define otd_log_i(format, ...) Serial.printf("[I]" otd_pre format "\n", ##__VA_ARGS__)
#else
#define otd_log_i(format, ...)
#endif
#if ARDUHAL_LOG_LEVEL >= 1
#define otd_log_e(format, ...) Serial.printf("[E]" otd_pre format "\n", ##__VA_ARGS__)
#else
#define otd_log_e(format, ...)
#endif
// end ESP8266
#elif defined(ESP32)
#define otd_log_d(format, ...) log_d(format, ##__VA_ARGS__)
#define otd_log_i(format, ...) log_i(format, ##__VA_ARGS__)
#define otd_log_e(format, ...) log_e(format, ##__VA_ARGS__)
// end ESP32
#endif

#endif
