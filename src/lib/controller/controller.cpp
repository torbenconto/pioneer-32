/*
  Disclaimer!!!
  While this is a heavily modified version, this code was almost entirely based on https://github.com/asukiaaa/arduino-XboxSeriesXControllerESP32
*/
#include "controller.h"

NimBLEUUID uuidServiceHid("1812");
const uint16_t controllerAppearance = 964;
NimBLEAdvertisedDevice* advDevice = nullptr;
NimBLEClient* pConnectedClient = nullptr;

ClientCallbacks::ClientCallbacks(ConnectionState* pConnectionState) : pConnectionState(pConnectionState) {}

void ClientCallbacks::onConnect(NimBLEClient* pClient) {
  *pConnectionState = ConnectionState::WaitingForFirstNotification;
}

void ClientCallbacks::onDisconnect(NimBLEClient* pClient) {
  *pConnectionState = ConnectionState::Scanning;
  pConnectedClient = nullptr;
}

uint32_t ClientCallbacks::onPassKeyRequest() { return 0; }

bool ClientCallbacks::onConfirmPIN(uint32_t pass_key) { return true; }

void ClientCallbacks::onAuthenticationComplete(ble_gap_conn_desc* desc) {
  if (!desc->sec_state.encrypted) {
    NimBLEDevice::getClientByID(desc->conn_handle)->disconnect();
  }
}

AdvertisedDeviceCallbacks::AdvertisedDeviceCallbacks(String strTargetDeviceAddress, ConnectionState* pConnectionState)
    : pConnectionState(pConnectionState) {
  if (!strTargetDeviceAddress.isEmpty()) {
    targetDeviceAddress = new NimBLEAddress(strTargetDeviceAddress.c_str());
  }
}

void AdvertisedDeviceCallbacks::onResult(NimBLEAdvertisedDevice* advertisedDevice) {
  if ((targetDeviceAddress != nullptr && advertisedDevice->getAddress().equals(*targetDeviceAddress)) ||
      (targetDeviceAddress == nullptr &&
       advertisedDevice->getAppearance() == controllerAppearance &&
       advertisedDevice->getServiceUUID().equals(uuidServiceHid))) {
    *pConnectionState = ConnectionState::Found;
    advDevice = advertisedDevice;
  }
}

Controller::Controller(String targetDeviceAddress) {
  advDeviceCBs = new AdvertisedDeviceCallbacks(targetDeviceAddress, &connectionState);
  clientCBs = new ClientCallbacks(&connectionState);
  connectionState = ConnectionState::Scanning;
  scanTime = 4;
  countFailedConnection = 0;
  retryCountInOneConnection = 3;
  retryIntervalMs = 100;
}

void Controller::begin() {
  NimBLEDevice::init("");
  NimBLEDevice::setOwnAddrType(BLE_OWN_ADDR_PUBLIC);
  NimBLEDevice::setSecurityAuth(true, false, false);
}

void Controller::onLoop() {
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

String Controller::buildDeviceAddressStr() {
  char buffer[18];
  snprintf(buffer, sizeof(buffer), "%02x:%02x:%02x:%02x:%02x:%02x",
           deviceAddressArr[5], deviceAddressArr[4], deviceAddressArr[3],
           deviceAddressArr[2], deviceAddressArr[1], deviceAddressArr[0]);
  return String(buffer);
}

void Controller::startScan() {
  connectionState = ConnectionState::Scanning;
  auto pScan = NimBLEDevice::getScan();
  pScan->setDuplicateFilter(false);
  pScan->setAdvertisedDeviceCallbacks(advDeviceCBs);
  pScan->setInterval(97);
  pScan->setWindow(97);
  pScan->start(scanTime, &Controller::scanCompleteCB, false);
}

bool Controller::isWaitingForFirstNotification() {
  return connectionState == ConnectionState::WaitingForFirstNotification;
}

bool Controller::isConnected() {
  return connectionState == ConnectionState::WaitingForFirstNotification ||
         connectionState == ConnectionState::Connected;
}

uint8_t Controller::getCountFailedConnection() { return countFailedConnection; }

bool Controller::isScanning() { return NimBLEDevice::getScan()->isScanning(); }

bool Controller::connectToServer(NimBLEAdvertisedDevice* advDevice) {
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

bool Controller::afterConnect(NimBLEClient* pClient) {
  memcpy(deviceAddressArr, pClient->getPeerAddress().getNative(), deviceAddressLen);
  for (auto pService : *pClient->getServices(true)) {
    if (pService->getUUID().equals(uuidServiceHid)) {
      for (auto pChara : *pService->getCharacteristics(true)) {
        charaSubscribeNotification(pChara);
      }
    }
  }
  return true;
}

void Controller::charaSubscribeNotification(NimBLERemoteCharacteristic* pChara) {
  if (pChara->canNotify()) {
    pChara->subscribe(true, std::bind(&Controller::notifyCB, this, std::placeholders::_1,
                                       std::placeholders::_2, std::placeholders::_3,
                                       std::placeholders::_4), true);
  }
}

void Controller::notifyCB(NimBLERemoteCharacteristic* pRemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
  if (pRemoteCharacteristic->getRemoteService()->getUUID().equals(uuidServiceHid)) {
    xboxNotif.update(pData, length);
  }
}

void Controller::scanCompleteCB(NimBLEScanResults results) {}
