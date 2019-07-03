 #include <Arduino.h>
/*
    Video: https://www.youtube.com/watch?v=oCMOYS71NIU
    Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleNotify.cpp
    Ported to Arduino ESP32 by Evandro Copercini
    updated by chegewara

   Create a BLE server that, once we receive a connection, will send periodic notifications.
   The service advertises itself as: 4fafc201-1fb5-459e-8fcc-c5c9c331914b
   And has a characteristic of: beb5483e-36e1-4688-b7f5-ea07361b26a8

   The design of creating the BLE server is:
   1. Create a BLE Server
   2. Create a BLE Service
   3. Create a BLE Characteristic on the Service
   4. Create a BLE Descriptor on the characteristic
   5. Start the service.
   6. Start advertising.

   A connect hander associated with the server starts a background task that performs notification
   every couple of seconds.
*/
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;
uint32_t value = 0;

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"


class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      BLEDevice::startAdvertising();
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};



void setup() {
  Serial.begin(9600);

  // Create the BLE Device
  BLEDevice::init("ESP32-MultipleClient");

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY |
                      BLECharacteristic::PROPERTY_BROADCAST|
                      BLECharacteristic::PROPERTY_INDICATE
                    );

  // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.descriptor.gatt.client_characteristic_configuration.xml
  // Create a BLE Descriptor
  pCharacteristic->addDescriptor(new BLE2902());

  // Start the service
  pService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0);  // set value to 0x00 to not advertise this parameter
  BLEDevice::startAdvertising();
  Serial.println("Waiting a client connection to notify...");
}

void loop() {
    // notify changed value
    if (deviceConnected) {
        int read= hallRead();
        pCharacteristic->setValue(read);
        pCharacteristic->notify();
        //value++;
        Serial.println(read);
        delay(3000); // bluetooth stack will go into congestion, if too many packets are sent, in 6 hours test i was able to go as low as 3ms
    }
    // disconnecting
    if (!deviceConnected && oldDeviceConnected) {
        delay(500); // give the bluetooth stack the chance to get things ready
        pServer->startAdvertising(); // restart advertising
        Serial.println("start advertising");
        oldDeviceConnected = deviceConnected;
    }
    // connecting
    if (deviceConnected && !oldDeviceConnected) {
        // do stuff here on connecting
        oldDeviceConnected = deviceConnected;
    }
}


 // #include <BLEDevice.h>
// #include <BLEServer.h>
// #include <BLEUtils.h>
// #include <BLE2902.h>

// BLECharacteristic *pCharacteristic;
// bool deviceConnected = false;
// float txValue = 0;
// const int readPin = 32; // Use GPIO number. See ESP32 board pinouts
// const int LED = 2; // Could be different depending on the dev board. I used the DOIT ESP32 dev board.

// //std::string rxValue; // Could also make this a global var to access it in loop()

// // See the following for generating UUIDs:
// // https://www.uuidgenerator.net/

// #define SERVICE_UUID           "480d821e-4cc7-4846-a0c3-21011c224564" // UART service UUID
// #define CHARACTERISTIC_UUID_RX "e3b9b0ec-abde-4e6c-8788-cc4e8d5cb1f2"
// #define CHARACTERISTIC_UUID_TX "52b5f497-c3af-42b6-b917-7a4f4cb0c6e1"

// class MyServerCallbacks: public BLEServerCallbacks {
//     void onConnect(BLEServer* pServer) {
//       deviceConnected = true;
//     };

//     void onDisconnect(BLEServer* pServer) {
//       deviceConnected = false;
//     }
// };

// class MyCallbacks: public BLECharacteristicCallbacks {
//     void onWrite(BLECharacteristic *pCharacteristic) {
//       std::string rxValue = pCharacteristic->getValue();

//       if (rxValue.length() > 0) {
//         Serial.println("*********");
//         Serial.print("Received Value: ");

//         for (int i = 0; i < rxValue.length(); i++) {
//           Serial.print(rxValue[i]);
//         }

//         Serial.println();

//         // Do stuff based on the command received from the app
//         if (rxValue.find("A") != -1) { 
//           Serial.println("Turning ON!");
//           digitalWrite(LED, HIGH);
//         }
//         else if (rxValue.find("B") != -1) {
//           Serial.println("Turning OFF!");
//           digitalWrite(LED, LOW);
//         }

//         Serial.println();
//         Serial.println("*********");
//       }
//     }
// };

// void setup() {
//   Serial.begin(9600);

//   pinMode(LED, OUTPUT);

//   // Create the BLE Device
//   BLEDevice::init("ESP32 UART Test"); // Give it a name

//   // Create the BLE Server
//   BLEServer *pServer = BLEDevice::createServer();
//   pServer->setCallbacks(new MyServerCallbacks());

//   // Create the BLE Service
//   BLEService *pService = pServer->createService(SERVICE_UUID);

//   // Create a BLE Characteristic
//   pCharacteristic = pService->createCharacteristic(
//                       CHARACTERISTIC_UUID_TX,
//                       BLECharacteristic::PROPERTY_NOTIFY
//                     );
                      
//   pCharacteristic->addDescriptor(new BLE2902());

//   BLECharacteristic *pCharacteristic = pService->createCharacteristic(
//                                          CHARACTERISTIC_UUID_RX,
//                                          BLECharacteristic::PROPERTY_WRITE
//                                        );

//   pCharacteristic->setCallbacks(new MyCallbacks());

//   // Start the service
//   pService->start();

//   // Start advertising
//   pServer->getAdvertising()->start();
//   Serial.println("Waiting a client connection to notify...");
// }

// void loop() {
//   if (deviceConnected) {
//     // Fabricate some arbitrary junk for now...
//     txValue = analogRead(readPin) / 3.456; // This could be an actual sensor reading!

//     // Let's convert the value to a char array:
//     char txString[8]; // make sure this is big enuffz
//     dtostrf(txValue, 1, 2, txString); // float_val, min_width, digits_after_decimal, char_buffer
    
// //    pCharacteristic->setValue(&txValue, 1); // To send the integer value
// //    pCharacteristic->setValue("Hello!"); // Sending a test message
//     pCharacteristic->setValue(txString);
    
//     pCharacteristic->notify(); // Send the value to the app!
//     Serial.print("*** Sent Value: ");
//     Serial.print(txString);
//     Serial.println(" ***");

//     // You can add the rxValue checks down here instead
//     // if you set "rxValue" as a global var at the top!
//     // Note you will have to delete "std::string" declaration
//     // of "rxValue" in the callback function.
// //    if (rxValue.find("A") != -1) { 
// //      Serial.println("Turning ON!");
// //      digitalWrite(LED, HIGH);
// //    }
// //    else if (rxValue.find("B") != -1) {
// //      Serial.println("Turning OFF!");
// //      digitalWrite(LED, LOW);
// //    }
//   }
//   delay(1000);
// }