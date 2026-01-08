#include <SPI.h>
#include <LoRa.h>


const int csPin = 5;         
const int resetPin = 4;      
const int irqPin = 2;        
const int sck = 14;
const int miso = 12;
const int mosi = 13;


int X = 0; // 成功接收計數
const int totalExpected = 10000;


void setup() {
 Serial.begin(115200);
 while (!Serial);


 Serial.println("--- LoRa Receiver (Ground) ---");


 SPI.begin(sck, miso, mosi, csPin);
 LoRa.setPins(csPin, resetPin, irqPin);


 // 參數必須與發送端完全一致
 if (!LoRa.begin(433E6)) {
   Serial.println("Starting LoRa failed!");
   while (1);
 }


 LoRa.setSpreadingFactor(10);
 LoRa.setSignalBandwidth(125E3);
 LoRa.setCodingRate4(5);
 LoRa.setSyncWord(0xF1);
 LoRa.setPreambleLength(8);


 Serial.println("Ready to receive...");
}


void loop() {
 int packetSize = LoRa.parsePacket();
 if (packetSize) {
   String receivedData = "";
   while (LoRa.available()) {
     receivedData += (char)LoRa.read();
   }


   X++; // 累加接收次數
  
   // 獲取信號強度 (RSSI) 與 信噪比 (SNR)，這對 10 公里測試很重要
   int rssi = LoRa.packetRssi();
   float snr = LoRa.packetSnr();


   Serial.print("Rx: [");
   Serial.print(receivedData);
   Serial.print("] | Total: ");
   Serial.print(X);
   Serial.print(" | RSSI: ");
   Serial.print(rssi);
   Serial.print(" | SNR: ");
   Serial.println(snr);


   // 達到 10000 個或每收到 100 個時更新統計
   if (X % 100 == 0 || receivedData == "10000") {
     printStatistics();
   }
 }
}


void printStatistics() {
 // 丟失率計算公式
 float lossRate = (1.0 - ((float)X / (float)totalExpected)) * 100.0;
  Serial.println("\n--- Test Report ---");
 Serial.print("Received Packets (X): "); Serial.println(X);
 Serial.print("Packet Loss Rate: "); Serial.print(lossRate); Serial.println("%");
  if (lossRate <= 20.0) {
   Serial.println("Result: VERIFIED (Pass)");
 } else {
   Serial.println("Result: FAILED (Loss too high)");
 }
 Serial.println("-------------------\n");
}
