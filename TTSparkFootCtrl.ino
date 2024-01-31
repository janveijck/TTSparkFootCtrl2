#include <NimBLEDevice.h>
#include <Arduino.h>
#include "Spark.h"

//#define CLASSIC

#define MIDI_SERVICE_UUID         "03b80e5a-ede8-4b33-a751-6ce34ec4c700" // MIDI Service UUID
#define MIDI_CHARACTERISTIC_UUID  "7772e5db-3868-4112-a1a9-f2669d106bf3" // MIDI Characteristic UUID
//#define MIDI_ADDRESS "08:29:78:37:7A:AD" 
#define MIDI_ADDRESS "0D:8C:83:A3:84:AB"
// #define MIDI_ADDRESS "11:61:7A:B5:E5:2F" //Tonino's Footctrl address

NimBLEClient* pClient;
NimBLERemoteService* pMidiService;
NimBLERemoteCharacteristic* pMidiCharacteristic;
void connectFootCtrl();
int mode = 0;
int presetmode = 0;
int effcomp = 0;
int effdelay = 0;
int effreverb = 0;
int effdrive = 0;

class MyCallbacks : public NimBLEClientCallbacks {
    void onConnect(NimBLEClient* pClient) {
        Serial.println("Connected to BLE MIDI device");
    }

    void onDisconnect(NimBLEClient* pClient) {
        Serial.println("Disconnected from BLE MIDI device");

        ESP.restart();
       
        Serial.println("Setup complete.");
        }
};

void notifyCallback(NimBLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
    if (isNotify) {

      int midi_chan, midi_cmd = 0,midi_data; bool onoff;
      
      midi_cmd = pData[2] ;
      midi_data = pData[3] % 4;
        
      Serial.print("Received MIDI Message: ");     
      for (size_t i = 0; i < length; i++) {
          Serial.print(pData[i], HEX);
          Serial.print(" ");
      }
      Serial.println();
      Serial.print("Midi_data: ");
      Serial.println(midi_data);
      Serial.println();
      Serial.print("Midi_command: ");
      Serial.print(midi_cmd);
      Serial.println();
      Serial.print("presetmode: ");
      Serial.println(presetmode);
      
      if (midi_cmd == 0) {
        Serial.println();
        Serial.println("Startup phase so presets");
      }
      else if (midi_cmd == 176) {
        if (presetmode == 0){
          presetmode = 1;
          Serial.print("Preset mode changed to:");
          Serial.println(presetmode);
        }
        else {
          presetmode = 0;
        }
        Serial.println("Time to toggle between presets and effects");
        Serial.print("Preset mode changed to:");
        Serial.println(presetmode);
      }
      else {
        Serial.println("Stay in the current mode (either presets or effects)");
      }

      //Code for hardware presets based on the presetmode toggle
      if (midi_cmd != 176){
        if (presetmode == 0){
          //hardware presets
          switch (midi_data) {
              case 0:              change_hardware_preset(0);Serial.println("change_hardware_preset 1");break;                 
              case 1:              change_hardware_preset(1);Serial.println("change_hardware_preset 2");break;                  
              case 2:              change_hardware_preset(2);Serial.println("change_hardware_preset 3");break;                
              case 3:              change_hardware_preset(3);Serial.println("change_hardware_preset 4");break;  
          }
        }
        else {
          //effect toggles
          switch (midi_data) {
            //case 0:              change_drive_toggle();Serial.println("change_drive_toggle 1");break;     
            //case 1:              change_mod_toggle();Serial.println("change_mod_toggle 2");break;                          
            //case 2:              change_delay_toggle();Serial.println("change_delay_toggle 3");break;                     
            //case 3:              change_reverb_toggle();Serial.println("change_reverb_toggle 4");break;     
            
            //Toggle Drive
            case 0:              
              Serial.println("change_drive_toggle");
              if (effdrive == 0) {
                effdrive = 1;
                change_drive_onoff(1);
              }
              else {
                effdrive = 0;
                change_drive_onoff(0);
              }
              break;      

            //Toggle Delay
            case 1:              
              Serial.println("change_delay_toggle");
              if (effdelay == 0) {
                effdelay = 1;
                change_delay_onoff(1);
              }
              else {
                effdelay = 0;
                change_delay_onoff(0);
              }
              break;

            //Toggle Reverb
            case 2:              
              Serial.println("change_reverb_toggle");
              if (effreverb == 0) {
                effreverb = 1;
                change_reverb_onoff(1);
              }
              else {
                effreverb = 0;
                change_reverb_onoff(0);
              }
              break;
            //Toggle Compression
            case 3:              
              Serial.println("change_compression_toggle");
              if (effcomp == 0) {
                effcomp = 1;
                change_comp_onoff(1);
              }
              else {
                effdrive = 0;
                change_comp_onoff(0);
              }
              break;   
          } 
        }
      }
      update_ui();
    }
          
        
}

void connectFootCtrl() {

  NimBLEDevice::init("ESP32 MIDI Client");
    pClient = NimBLEDevice::createClient();

    MyCallbacks* pCallbacks = new MyCallbacks();
    pClient->setClientCallbacks(pCallbacks);

    Serial.println("Connecting to BLE MIDI device...");
    NimBLEAddress serverAddress(MIDI_ADDRESS); // Replace with the address of your BLE MIDI device
    pClient->connect(serverAddress, true);

    Serial.println("Getting MIDI service...");
    pMidiService = pClient->getService(NimBLEUUID(MIDI_SERVICE_UUID));

    if (pMidiService) {
        Serial.println("Getting MIDI characteristic...");
        pMidiCharacteristic = pMidiService->getCharacteristic(NimBLEUUID(MIDI_CHARACTERISTIC_UUID));

        if (pMidiCharacteristic) {
            Serial.println("Registering notify callback...");
            pMidiCharacteristic->registerForNotify(notifyCallback);
        } else {
            Serial.println("Failed to get MIDI characteristic!");
            ESP.restart();
        }
    } else {
        Serial.println("Failed to get MIDI service!");
    }
}

void setup() {
    //setCpuFrequencyMhz(80);
    Serial.begin(115200);
    Serial.println("Starting setup...");
    mode = 0;

    DEBUG("Spark connecting ...");
    while (!spark_state_tracker_start()) {  // set up data to track Spark and app state, if it fails to find the Spark it will return false
      DEBUG("No Spark found - perhaps sleep?");// think about a deep sleep here if needed
    }
    DEBUG("Spark found and connected - starting");
    spark_state_tracker_start();
    connectFootCtrl();


    Serial.println("Setup complete.");
}

void loop() {
    // Nothing to do in the loop for this basic example.
  
}

