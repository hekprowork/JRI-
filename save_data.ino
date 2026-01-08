#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP3XX.h>
#include <ICM_20948.h>
#include <TinyGPS++.h>
#include <LoRa.h>

// 腳位定義
#define PIN_IMU_SDA 26
#define PIN_IMU_SCL 25
#define PIN_BARO_SDA 21
#define PIN_BARO_SCL 22

#define PIN_SPI_SCK 14
#define PIN_SPI_MISO 12
#define PIN_SPI_MOSI 13
#define PIN_SD_CS 18
#define PIN_LORA_CS 5
#define PIN_FLASH_CS 23

#define PIN_GPS_RX 17
#define PIN_GPS_TX 16
#define PIN_SW_ON 15

// 物件實例化
Adafruit_BMP3XX bmp;
ICM_20948_I2C myICM;
TinyGPSPlus gps;
HardwareSerial SerialGPS(2);

// 全域變數
File dataFile;
unsigned long lastLogTime = 0;
const int logInterval = 100; // 100ms

void setup() {
  Serial.begin(115200);
  
  // 1. 初始化 SW IC (啟動後端電源)
  pinMode(PIN_SW_ON, OUTPUT);
  digitalWrite(PIN_SW_ON, HIGH);
  delay(500);

  // 2. 初始化 I2C (兩組)
  Wire.begin(PIN_IMU_SDA, PIN_IMU_SCL);     // Wire 用於 IMU
  Wire1.begin(PIN_BARO_SDA, PIN_BARO_SCL);  // Wire1 用於 Barometer

  // 3. 初始化感測器
  if (!bmp.begin_I2C(0x77, &Wire1)) { 
    Serial.println("Could not find BMP388!");
  }
  myICM.begin(Wire, 1); // 0x69 AD0 High

  // 4. 初始化 SPI 設備
  pinMode(PIN_LORA_CS, OUTPUT);
  pinMode(PIN_FLASH_CS, OUTPUT);
  digitalWrite(PIN_LORA_CS, HIGH); // 預設不選取
  digitalWrite(PIN_FLASH_CS, HIGH);

  if (!SD.begin(PIN_SD_CS)) {
    Serial.println("SD Card failed!");
  } else {
    dataFile = SD.open("/flight_log.csv", FILE_WRITE);
    if (dataFile) {
      dataFile.println("Time,AccX,AccY,AccZ,GyroX,GyroY,GyroZ,Pressure,Alt,Lat,Lng,Sats");
      dataFile.close();
    }
  }

  // 5. 初始化 GPS 與 LoRa
  SerialGPS.begin(9600, SERIAL_8N1, PIN_GPS_RX, PIN_GPS_TX);
  LoRa.setPins(PIN_LORA_CS, -1, -1); // 這裡僅測試連接性
  if (!LoRa.begin(433E6)) {
    Serial.println("LoRa failed!");
  }

  Serial.println("Initialization complete. Waiting for 30s convergence...");
}

void loop() {
  // 持續讀取 GPS 串口
  while (SerialGPS.available() > 0) {
    gps.encode(SerialGPS.read());
  }

  // 每 100ms 執行一次數據存儲
  if (millis() - lastLogTime >= logInterval) {
    lastLogTime = millis();
    saveData();
  }
}

void saveData() {
  // 讀取 IMU
  myICM.getAGMT();
  
  // 讀取氣壓計
  if (!bmp.performReading()) return;

  // 寫入 SD 卡
  dataFile = SD.open("/flight_log.csv", FILE_APPEND);
  if (dataFile) {
    dataFile.print(millis()); dataFile.print(",");
    
    // 加速度 (mg)
    dataFile.print(myICM.accX()); dataFile.print(",");
    dataFile.print(myICM.accY()); dataFile.print(",");
    dataFile.print(myICM.accZ()); dataFile.print(",");
    
    // 角速度 (dps)
    dataFile.print(myICM.gyrX()); dataFile.print(",");
    dataFile.print(myICM.gyrY()); dataFile.print(",");
    dataFile.print(myICM.gyrZ()); dataFile.print(",");
    
    // 氣壓與高度
    dataFile.print(bmp.pressure / 100.0); dataFile.print(",");
    dataFile.print(bmp.readAltitude(1013.25)); dataFile.print(","); // 請根據當地氣壓修正
    
    // GPS
    dataFile.print(gps.location.lat(), 6); dataFile.print(",");
    dataFile.print(gps.location.lng(), 6); dataFile.print(",");
    dataFile.print(gps.satellites.value());
    
    dataFile.println();
    dataFile.close(); 
  }
}