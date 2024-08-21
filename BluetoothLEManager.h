#ifndef BLUETOOTHLEMANAGER_H
#define BLUETOOTHLEMANAGER_H

#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Devices.Bluetooth.h>
#include <winrt/Windows.Devices.Bluetooth.Advertisement.h>
#include <winrt/Windows.Devices.Bluetooth.GenericAttributeProfile.h>
#include <unordered_set>  // 添加这个头文件
#include <string>
#include <vector>

class BluetoothLEManager {
public:
    BluetoothLEManager();
    ~BluetoothLEManager() = default;

    void ScanForDevices();
    void ConnectToDevice(const std::wstring& deviceName);
    void SendData(const std::vector<uint8_t>& data);
    void ReceiveData();
    void PrintDevices() const;

private:
    struct DeviceInfo {
        std::wstring name;
        std::wstring address;
        winrt::hstring id;
        int16_t signalStrength;
    };

    std::vector<DeviceInfo> devices;
    std::unordered_set<winrt::hstring> deviceIds; // 用于去重的集合
    winrt::Windows::Devices::Bluetooth::BluetoothLEDevice connectedDevice{ nullptr };
    winrt::Windows::Devices::Bluetooth::Advertisement::BluetoothLEAdvertisementWatcher watcher{ nullptr };

    void OnAdvertisementReceived(winrt::Windows::Devices::Bluetooth::Advertisement::BluetoothLEAdvertisementWatcher const& sender,
        winrt::Windows::Devices::Bluetooth::Advertisement::BluetoothLEAdvertisementReceivedEventArgs const& args);
    std::wstring GetDeviceNameFromBluetoothLEDevice(uint64_t bluetoothAddress);
    std::wstring FormatBluetoothAddress(uint64_t address);
};

#endif // BLUETOOTHLEMANAGER_H
