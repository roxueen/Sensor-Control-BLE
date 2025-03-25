#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <ArduinoJson.h>

#define LED 2
#define trig 5
#define echo 18
#define ldrPin 34

BLEServer *pServer = NULL;
BLECharacteristic *pCharacteristic = NULL;
bool deviceConnected = false;
const char *serviceUUID = "4fafc201-1fb5-459e-8fcc-c5c9c331914b";
const char *characteristicUUID = "beb5483e-36e1-4688-b7f5-ea07361b26a8";

class MyServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer *pServer) {
        deviceConnected = true;
    }
    void onDisconnect(BLEServer *pServer) {
        deviceConnected = false;
    }
};

void setup() {
    Serial.begin(115200);
    pinMode(LED, OUTPUT);
    pinMode(trig, OUTPUT);
    pinMode(echo, INPUT);
    pinMode(ldrPin, INPUT);
    
    BLEDevice::init("ESP32 BLE Sensor");
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());
    
    BLEService *pService = pServer->createService(serviceUUID);
    pCharacteristic = pService->createCharacteristic(
        characteristicUUID,
        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
    );
    pService->start();
    
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(serviceUUID);
    pAdvertising->setScanResponse(false);
    pAdvertising->setMinPreferred(0x06);
    pAdvertising->setMinPreferred(0x12);
    BLEDevice::startAdvertising();
    Serial.println("BLE Server started");
}

float readDistance() {
    digitalWrite(trig, LOW);
    delayMicroseconds(2);
    digitalWrite(trig, HIGH);
    delayMicroseconds(10);
    digitalWrite(trig, LOW);
    long duration = pulseIn(echo, HIGH);
    return duration * 0.034 / 2;
}

int readLight() {
    return analogRead(ldrPin);
}

void loop() {
    if (deviceConnected) {
        float distance = readDistance();
        int light = readLight();
        
        StaticJsonDocument<200> jsonDoc;
        jsonDoc["distance"] = distance;
        jsonDoc["light"] = light;
        
        char buffer[200];
        serializeJson(jsonDoc, buffer);
        pCharacteristic->setValue(buffer);
        pCharacteristic->notify();
        Serial.println(buffer);
    }
    delay(1000);
}
