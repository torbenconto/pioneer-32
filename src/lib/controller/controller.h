/*
  Disclaimer!!!
  While this is a heavily modified version, this code was almost entirely based on https://github.com/asukiaaa/arduino-XboxSeriesXControllerESP32
*/
#pragma once

#include <NimBLEDevice.h>
#include <XboxControllerNotificationParser.h>

extern NimBLEUUID uuidServiceHid;
extern const uint16_t controllerAppearance;
extern NimBLEAdvertisedDevice* advDevice;
extern NimBLEClient* pConnectedClient;


enum class ConnectionState : uint8_t {
  Connected = 0,
  WaitingForFirstNotification = 1,
  Found = 2,
  Scanning = 3,
};

class ClientCallbacks : public NimBLEClientCallbacks {
 public:
  ConnectionState* pConnectionState;
  ClientCallbacks(ConnectionState* pConnectionState);
  void onConnect(NimBLEClient* pClient) override;
  void onDisconnect(NimBLEClient* pClient) override;
  uint32_t onPassKeyRequest() override;
  bool onConfirmPIN(uint32_t pass_key) override;
  void onAuthenticationComplete(ble_gap_conn_desc* desc) override;
};

class AdvertisedDeviceCallbacks : public NimBLEAdvertisedDeviceCallbacks {
 public:
  AdvertisedDeviceCallbacks(String strTargetDeviceAddress, ConnectionState* pConnectionState);

 private:
  NimBLEAddress* targetDeviceAddress = nullptr;
  ConnectionState* pConnectionState;
  void onResult(NimBLEAdvertisedDevice* advertisedDevice) override;
};

class Controller {
 public:
  Controller(String targetDeviceAddress = "");
  void begin();
  void onLoop();
  String buildDeviceAddressStr();
  void startScan();
  bool isWaitingForFirstNotification();
  bool isConnected();
  uint8_t getCountFailedConnection();

  XboxControllerNotificationParser xboxNotif;

 private:
  AdvertisedDeviceCallbacks* advDeviceCBs;
  ClientCallbacks* clientCBs;
  static const int deviceAddressLen = 6;
  uint8_t deviceAddressArr[deviceAddressLen];
  ConnectionState connectionState;
  uint32_t scanTime;
  uint8_t countFailedConnection;
  uint8_t retryCountInOneConnection;
  unsigned long retryIntervalMs;

  bool isScanning();
  bool connectToServer(NimBLEAdvertisedDevice* advDevice);
  bool afterConnect(NimBLEClient* pClient);
  void charaSubscribeNotification(NimBLERemoteCharacteristic* pChara);
  void notifyCB(NimBLERemoteCharacteristic* pRemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify);
  static void scanCompleteCB(NimBLEScanResults results);
};
