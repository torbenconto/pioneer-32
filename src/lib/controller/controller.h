// credit to https://github.com/asukiaaa this is entirely based on one of his libraries for xbox controller handling
#pragma once

#include <NimBLEDevice.h>
#include <XboxControllerNotificationParser.h>

static NimBLEUUID uuidServiceHid("1812");

static NimBLEAdvertisedDevice* advDevice;
static NimBLEClient* pConnectedClient = nullptr;

static const uint16_t controllerAppearance = 964;

enum class ConnectionState : uint8_t {
  Connected = 0,
  WaitingForFirstNotification = 1,
  Found = 2,
  Scanning = 3,
};

class ClientCallbacks : public NimBLEClientCallbacks {
 public:
  ConnectionState* pConnectionState;
  ClientCallbacks(ConnectionState* pConnectionState) : pConnectionState(pConnectionState) {}

  void onConnect(NimBLEClient* pClient) override {
    *pConnectionState = ConnectionState::WaitingForFirstNotification;
  }

  void onDisconnect(NimBLEClient* pClient) override {
    *pConnectionState = ConnectionState::Scanning;
    pConnectedClient = nullptr;
  }

  uint32_t onPassKeyRequest() override { return 0; }
  bool onConfirmPIN(uint32_t pass_key) override { return true; }
  void onAuthenticationComplete(ble_gap_conn_desc* desc) override {
    if (!desc->sec_state.encrypted) {
      NimBLEDevice::getClientByID(desc->conn_handle)->disconnect();
    }
  }
};

class AdvertisedDeviceCallbacks : public NimBLEAdvertisedDeviceCallbacks {
 public:
  AdvertisedDeviceCallbacks(String strTargetDeviceAddress, ConnectionState* pConnectionState)
      : pConnectionState(pConnectionState) {
    if (!strTargetDeviceAddress.isEmpty()) {
      targetDeviceAddress = new NimBLEAddress(strTargetDeviceAddress.c_str());
    }
  }

 private:
  NimBLEAddress* targetDeviceAddress = nullptr;
  ConnectionState* pConnectionState;

  void onResult(NimBLEAdvertisedDevice* advertisedDevice) override {
    if ((targetDeviceAddress != nullptr && advertisedDevice->getAddress().equals(*targetDeviceAddress)) ||
        (targetDeviceAddress == nullptr &&
         advertisedDevice->getAppearance() == controllerAppearance &&
         advertisedDevice->getServiceUUID().equals(uuidServiceHid))) {
      *pConnectionState = ConnectionState::Found;
      advDevice = advertisedDevice;
    }
  }
};

class Controller {
 public:
  Controller(String targetDeviceAddress = "") {
    advDeviceCBs = new AdvertisedDeviceCallbacks(targetDeviceAddress, &connectionState);
    clientCBs = new ClientCallbacks(&connectionState);
  }

  AdvertisedDeviceCallbacks* advDeviceCBs;
  ClientCallbacks* clientCBs;
  static const int deviceAddressLen = 6;
  uint8_t deviceAddressArr[deviceAddressLen];

  void begin() {
    NimBLEDevice::init("");
    NimBLEDevice::setOwnAddrType(BLE_OWN_ADDR_PUBLIC);
    NimBLEDevice::setSecurityAuth(true, false, false);
  }

  void onLoop() {
    if (!isConnected()) {
      if (advDevice != nullptr) {
        auto connectionResult = connectToServer(advDevice);
        if (!connectionResult || !isConnected()) {
          NimBLEDevice::deleteBond(advDevice->getAddress());
          connectionState = ConnectionState::Scanning;
        } else {
          connectionState = ConnectionState::Connected;
        }
        advDevice = nullptr;
      } else if (!isScanning()) {
        startScan();
      }
    }
  }

  String buildDeviceAddressStr() {
    char buffer[18];
    snprintf(buffer, sizeof(buffer), "%02x:%02x:%02x:%02x:%02x:%02x",
             deviceAddressArr[5], deviceAddressArr[4], deviceAddressArr[3],
             deviceAddressArr[2], deviceAddressArr[1], deviceAddressArr[0]);
    return String(buffer);
  }

  void startScan() {
    connectionState = ConnectionState::Scanning;
    auto pScan = NimBLEDevice::getScan();
    pScan->setDuplicateFilter(false);
    pScan->setAdvertisedDeviceCallbacks(advDeviceCBs);
    pScan->setInterval(97);
    pScan->setWindow(97);
    pScan->start(scanTime, &Controller::scanCompleteCB, false);
  }

  XboxControllerNotificationParser xboxNotif;

  bool isWaitingForFirstNotification() {
    return connectionState == ConnectionState::WaitingForFirstNotification;
  }
  bool isConnected() {
    return connectionState == ConnectionState::WaitingForFirstNotification ||
           connectionState == ConnectionState::Connected;
  }
  uint8_t getCountFailedConnection() { return countFailedConnection; }

 private:
  ConnectionState connectionState = ConnectionState::Scanning;
  uint32_t scanTime = 4;
  uint8_t countFailedConnection = 0;
  uint8_t retryCountInOneConnection = 3;
  unsigned long retryIntervalMs = 100;

  bool isScanning() { return NimBLEDevice::getScan()->isScanning(); }

  bool connectToServer(NimBLEAdvertisedDevice* advDevice) {
    NimBLEClient* pClient = NimBLEDevice::getClientByPeerAddress(advDevice->getAddress());
    if (!pClient) {
      if (NimBLEDevice::getClientListSize() >= NIMBLE_MAX_CONNECTIONS) {
        return false;
      }
      pClient = NimBLEDevice::createClient();
      pClient->setClientCallbacks(clientCBs, true);
      pClient->connect(advDevice, true);
    }

    int retryCount = retryCountInOneConnection;
    while (!pClient->isConnected()) {
      if (retryCount <= 0) {
        return false;
      }
      delay(retryIntervalMs);
      pClient->connect(true);
      --retryCount;
    }

    bool result = afterConnect(pClient);
    if (!result) {
      return result;
    }
    pConnectedClient = pClient;
    return true;
  }

  bool afterConnect(NimBLEClient* pClient) {
    memcpy(deviceAddressArr, pClient->getPeerAddress().getNative(), deviceAddressLen);
    for (auto pService : *pClient->getServices(true)) {
      auto sUuid = pService->getUUID();
      if (sUuid.equals(uuidServiceHid)) {
        for (auto pChara : *pService->getCharacteristics(true)) {
          charaHandle(pChara);
          charaSubscribeNotification(pChara);
        }
      }
    }
    return true;
  }

  void charaHandle(NimBLERemoteCharacteristic* pChara) {
    if (pChara->canRead()) {
      auto str = pChara->readValue();
    }
  }

  void charaSubscribeNotification(NimBLERemoteCharacteristic* pChara) {
    if (pChara->canNotify()) {
      if (pChara->subscribe(true, std::bind(&Controller::notifyCB, this, std::placeholders::_1,
                                             std::placeholders::_2, std::placeholders::_3,
                                             std::placeholders::_4), true)) {
      }
    }
  }

  void notifyCB(NimBLERemoteCharacteristic* pRemoteCharacteristic, uint8_t* pData, size_t length,
                bool isNotify) {
    auto sUuid = pRemoteCharacteristic->getRemoteService()->getUUID();
    if (sUuid.equals(uuidServiceHid)) {
      xboxNotif.update(pData, length);
    }
  }

  static void scanCompleteCB(NimBLEScanResults results) {
  }
};