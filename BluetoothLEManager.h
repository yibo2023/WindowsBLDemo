#ifndef BLUETOOTHLEMANAGER_H
#define BLUETOOTHLEMANAGER_H

#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Devices.Bluetooth.h>
#include <winrt/Windows.Devices.Bluetooth.Advertisement.h>
#include <winrt/Windows.Devices.Bluetooth.GenericAttributeProfile.h>
#include <winrt/Windows.Storage.Streams.h>
#include <unordered_set>
#include <iostream>
#include <string>
#include <vector>

class BluetoothLEManager {
public:
    BluetoothLEManager();
    ~BluetoothLEManager() = default;

    void ScanForDevices();
    void ConnectToDevice(const std::wstring& deviceName);
    void DisconnectFromDevice();  // 新增的断开连接功能声明
    void SendData(const std::vector<uint8_t>& data);
    void ReceiveData();
    void PrintDevices() const;

private:
    struct DeviceInfo {
        std::wstring name;
        std::wstring address;
        winrt::hstring id;
        int16_t signalStrength;
        uint64_t bluetoothAddress;
    };

    std::vector<DeviceInfo> devices;
    std::unordered_set<winrt::hstring> deviceIds; // 用于去重的集合
    winrt::Windows::Devices::Bluetooth::BluetoothLEDevice connectedDevice{ nullptr };
    winrt::Windows::Devices::Bluetooth::Advertisement::BluetoothLEAdvertisementWatcher watcher{ nullptr };

    void OnAdvertisementReceived(winrt::Windows::Devices::Bluetooth::Advertisement::BluetoothLEAdvertisementWatcher const& sender,
        winrt::Windows::Devices::Bluetooth::Advertisement::BluetoothLEAdvertisementReceivedEventArgs const& args);
    std::wstring GetDeviceNameFromBluetoothLEDevice(uint64_t bluetoothAddress);
    std::wstring FormatBluetoothAddress(uint64_t address);
    void OnCharacteristicValueChanged(winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattCharacteristic const& sender,
        winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattValueChangedEventArgs const& args);
};

#endif // BLUETOOTHLEMANAGER_H
