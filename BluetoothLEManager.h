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
    void DisconnectFromDevice();  // �����ĶϿ����ӹ�������
    void SendData(const std::vector<uint8_t>& data, uint32_t serviceIndex = 0, uint32_t characteristicIndex = 0);
    void ReceiveData(uint32_t serviceIndex = 0, uint32_t characteristicIndex = 0);
    const std::vector<uint8_t>& GetReceivedData() const;
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
    std::vector<uint8_t> m_vReceivedData;  // �洢���յ�������
    std::unordered_set<winrt::hstring> deviceIds; // ����ȥ�صļ���
    winrt::Windows::Devices::Bluetooth::BluetoothLEDevice m_connectedDevice{ nullptr };
    winrt::Windows::Devices::Bluetooth::Advertisement::BluetoothLEAdvertisementWatcher watcher{ nullptr };

    void OnAdvertisementReceived(winrt::Windows::Devices::Bluetooth::Advertisement::BluetoothLEAdvertisementWatcher const& sender,
        winrt::Windows::Devices::Bluetooth::Advertisement::BluetoothLEAdvertisementReceivedEventArgs const& args);
    std::wstring GetDeviceNameFromBluetoothLEDevice(uint64_t bluetoothAddress);
    std::wstring FormatBluetoothAddress(uint64_t address);
    void OnCharacteristicValueChanged(winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattCharacteristic const& sender,
        winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattValueChangedEventArgs const& args);

    // ������������ȡָ�������� GATT ���������
    winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattDeviceService GetGattService(uint32_t index);
    winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattCharacteristic GetGattCharacteristic(
        const winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattDeviceService& service, uint32_t index);
};

#endif // BLUETOOTHLEMANAGER_H
