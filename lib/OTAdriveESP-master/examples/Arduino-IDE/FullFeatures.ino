#include <Arduino.h>
#include <otadrive_esp.h>

#define APIKEY "c0af643b-4f90-4905-9807-db8be5164cde"
#define LED 2

void update();
void listDir(fs::FS &fs, const char *dirname, uint8_t levels);
void onUpdateProgress(int progress, int totalt);

#ifdef ESP8266
#define FILESYS LittleFS
#elif defined(ESP32)
#define FILESYS SPIFFS
#endif

void setup()
{
  Serial.begin(115200);

  pinMode(LED, OUTPUT);
  WiFi.begin("*", "*");

  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    digitalWrite(2, HIGH);
    delay(100);
    digitalWrite(2, LOW);
    delay(400);
  }

  Serial.println(WiFi.localIP());

  // initialize FileSystem
#ifdef ESP8266
  if (!LittleFS.begin())
  {
    Serial.println("LittleFS Mount Failed");
    LittleFS.format();
    return;
  }
#elif defined(ESP32)
  if (!SPIFFS.begin(true))
  {
    Serial.println("SPIFFS Mount Failed");
    return;
  }
#endif
  Serial.println("File system Mounted");

  OTADRIVE.setInfo(APIKEY, "2.5.5");

  OTADRIVE.onUpdateFirmwareProgress(onUpdateProgress);
}

void loop()
{
  delay(1000);
  if (WiFi.status() == WL_CONNECTED)
  {
    // Every 30 seconds
    if(OTADRIVE.timeTick(30))
    {
      digitalWrite(2, HIGH);
      delay(100);
      digitalWrite(2, LOW);
      delay(400);
      Serial.println("\n\nloop");

      // retrive firmware info from OTAdrive server
      updateInfo inf = OTADRIVE.updateFirmwareInfo();
      Serial.printf("\nfirmware info: %s ,%dBytes\n%s\n",
                    inf.version.c_str(), inf.size, inf.available ? "New version available" : "No newer version");
      // update firmware if newer available
      if (inf.available)
        OTADRIVE.updateFirmware();

      // sync local files with OTAdrive server
      OTADRIVE.syncResources();
      // list local files to serial port
      listDir(OTA_FILE_SYS, "/", 0);

      // get configuration of device
      String c = OTADRIVE.getConfigs();
      Serial.printf("\nconfiguration: %s\n", c.c_str());
    }
  }
}

void onUpdateProgress(int progress, int totalt)
{
  static int last = 0;
  int progressPercent = (100 * progress) / totalt;
  Serial.print("*");
  if (last != progressPercent && progressPercent % 10 == 0)
  {
    //print every 10%
    Serial.printf("%d", progressPercent);
  }
  last = progressPercent;
}

void listDir(fs::FS &fs, const char *dirname, uint8_t levels)
{
  Serial.printf("Listing directory: %s\r\n", dirname);

  File root = fs.open(dirname, "r");
  if (!root)
  {
    Serial.println("- failed to open directory");
    return;
  }
  if (!root.isDirectory())
  {
    Serial.println(" - not a directory");
    return;
  }

  File file = root.openNextFile();
  while (file)
  {
    if (file.isDirectory())
    {
      Serial.print("  DIR : ");
      Serial.println(file.name());
      if (levels)
      {
        listDir(fs, file.name(), levels - 1);
      }
    }
    else
    {
      Serial.print("  FILE: ");
      Serial.print(file.name());
      Serial.print("\tSIZE: ");
      Serial.println(file.size());
    }
    file = root.openNextFile();
  }
}