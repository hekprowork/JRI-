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
const int intervalMs = 500; // 為了 SF10 的穩定性，間隔設為 500ms


void setup() {
 Serial.begin(115200);
 while (!Serial);


 Serial.println("--- LoRa Sender (Avionics) ---");


 // 初始化自定義 SPI 引腳
 SPI.begin(sck, miso, mosi, csPin);
 LoRa.setPins(csPin, resetPin, irqPin);


 // 1. 載波頻率 (Frequency)
 if (!LoRa.begin(433E6)) {
   Serial.println("Starting LoRa failed! Check wiring.");
   while (1);
 }


 // 設定優化參數
 LoRa.setSpreadingFactor(10);           // 2. 擴頻因子
 LoRa.setSignalBandwidth(125E3);        // 3. 頻寬
 LoRa.setCodingRate4(5);                // 4. 編碼率
 LoRa.setTxPower(20);                   // 5. 發射功率
 LoRa.setSyncWord(0xF1);                // 6. 同步字
 LoRa.setPreambleLength(8);             // 7. 前導碼


 Serial.println("LoRa Initialized with Long Range Parameters.");
 delay(2000); // 準備時間
}


void loop() {
 for (int i = 1; i <= totalPackets; i++) {
   Serial.print("Sending packet: ");
   Serial.println(i);


   // 開始發送封包
   LoRa.beginPacket();
   LoRa.print(i);
   LoRa.endPacket();


   delay(intervalMs); // 8. 建議發送間隔
 }


 Serial.println("Mission Complete: 10000 packets sent.");
 while (1); // 停止發送
}
