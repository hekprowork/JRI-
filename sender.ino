#include <SPI.h>
#include <LoRa.h>

// 引腳定義
const int csPin = 5;          
const int resetPin = 4;       
const int irqPin = 2;         
const int sck = 14;
const int miso = 12;
const int mosi = 13;

const int totalPackets = 10000;
const int intervalMs = 500; 

// 定義測試資料長度
const int PAYLOAD_SIZE = 32;

void setup() {
  Serial.begin(115200);
  while (!Serial);

  Serial.println("--- LoRa Radio Test: 32-Byte Payload Sender ---");

  // 初始化自定義 SPI 引腳
  SPI.begin(sck, miso, mosi, csPin);
  LoRa.setPins(csPin, resetPin, irqPin);

  // 初始化 LoRa (433MHz)
  if (!LoRa.begin(433E6)) {
    Serial.println("Starting LoRa failed! Check wiring.");
    while (1);
  }

  // 無線電參數設定
  LoRa.setSpreadingFactor(10);           
  LoRa.setSignalBandwidth(125E3);        
  LoRa.setCodingRate4(5);                
  LoRa.setTxPower(20);                   
  LoRa.setSyncWord(0xF1);                
  LoRa.setPreambleLength(8);             

  Serial.println("LoRa Configured. Ready for stress test.");
  delay(2000); 
}

void loop() {
  uint8_t testPayload[PAYLOAD_SIZE];

  for (int i = 1; i <= totalPackets; i++) {
    // 1. 初始化 Buffer (填充 0 或特定測試字元)
    memset(testPayload, 0xAA, PAYLOAD_SIZE); // 使用 0xAA (10101010) 有助於觀察訊號翻轉

    // 2. 將封包序號寫入前 4 個 Byte (Big Endian)
    testPayload[0] = (i >> 24) & 0xFF;
    testPayload[1] = (i >> 16) & 0xFF;
    testPayload[2] = (i >> 8) & 0xFF;
    testPayload[3] = i & 0xFF;

    // 3. 執行傳送
    Serial.print("Sending Packet ID: ");
    Serial.print(i);
    Serial.print(" [32 Bytes]... ");

    long startTime = millis();
    
    LoRa.beginPacket();
    LoRa.write(testPayload, PAYLOAD_SIZE); // 傳送固定 32 byte
    LoRa.endPacket();

    long airTime = millis() - startTime;
    Serial.print("Done. Airtime: ");
    Serial.print(airTime);
    Serial.println("ms");

    delay(intervalMs); 
  }

  Serial.println("Mission Complete: 10000 packets sent.");
  while (1); 
}
