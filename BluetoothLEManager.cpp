#include "BluetoothLEManager.h"
#include <iostream>
#include <winrt/Windows.Foundation.Collections.h>

using namespace winrt;
using namespace Windows::Devices::Bluetooth;
using namespace Windows::Devices::Bluetooth::Advertisement;
using namespace Windows::Devices::Bluetooth::GenericAttributeProfile;
using namespace Windows::Foundation;

BluetoothLEManager::BluetoothLEManager() {
    winrt::init_apartment();
}

void BluetoothLEManager::ScanForDevices() {
    watcher = BluetoothLEAdvertisementWatcher();
    watcher.Received({ this, &BluetoothLEManager::OnAdvertisementReceived });
    watcher.Start();

    std::wcout << L"Scanning for BLE devices..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(10)); // Scan for 10 seconds
    watcher.Stop();

    PrintDevices();
}

void BluetoothLEManager::ConnectToDevice(const std::wstring& deviceName) {
    for (const auto& device : devices) {
        if (device.name == deviceName) {
            connectedDevice = BluetoothLEDevice::FromIdAsync(device.id).get();
            if (connectedDevice) {
                std::wcout << L"Connected to device: " << deviceName << std::endl;
            }
            else {
                std::cerr << "Failed to connect to device." << std::endl;
            }
            break;
        }
    }
}

void BluetoothLEManager::SendData(const std::vector<uint8_t>& data) {
    if (!connectedDevice) {
        std::cerr << "No connected device." << std::endl;
        return;
    }

    // Assuming a GATT characteristic is available
    // Implement data sending logic based on the specific GATT profile
}

void BluetoothLEManager::ReceiveData() {
    if (!connectedDevice) {
        std::cerr << "No connected device." << std::endl;
        return;
    }

    // Implement data receiving logic based on the specific GATT profile
}

void BluetoothLEManager::PrintDevices() const {
    std::wcout << L"Discovered devices:" << std::endl;
    for (const auto& device : devices) {
        std::wcout << L"Name: " << device.name << L", ID: " << device.id.c_str() << L", Address: " << device.address << L", Signal Strength: " << device.signalStrength << L" dBm" << std::endl;
    }
}

void BluetoothLEManager::OnAdvertisementReceived(BluetoothLEAdvertisementWatcher const& sender, BluetoothLEAdvertisementReceivedEventArgs const& args) {
    //auto advertisement = args.Advertisement();
    //auto name = advertisement.LocalName();
    //auto id = args.BluetoothAddress();

    //DeviceInfo deviceInfo{
    //    name.empty() ? L"Unknown" : name.c_str(),
    //    hstring(std::to_wstring(id)),
    //    args.RawSignalStrengthInDBm()
    //};

    //devices.push_back(deviceInfo);



    auto advertisement = args.Advertisement();
    auto name = advertisement.LocalName();
    uint64_t address = args.BluetoothAddress();
    //std::wstring deviceName = name.empty() ? L"Unknown" : name.c_str();
    std::wstring deviceName = GetDeviceNameFromBluetoothLEDevice(args.BluetoothAddress());
    std::wstring formattedAddress = FormatBluetoothAddress(address);

    auto id = hstring(std::to_wstring(args.BluetoothAddress()));

    // 检查设备是否已经存在
    if (deviceIds.find(id) == deviceIds.end()) {
        deviceIds.insert(id); // 插入到集合中

        DeviceInfo deviceInfo{
            deviceName,
            formattedAddress,
            id,
            args.RawSignalStrengthInDBm()
        };
        devices.push_back(deviceInfo);
    }
}

std::wstring BluetoothLEManager::GetDeviceNameFromBluetoothLEDevice(uint64_t bluetoothAddress) {
    try {
        // 获取设备实例
        auto device = winrt::Windows::Devices::Bluetooth::BluetoothLEDevice::FromBluetoothAddressAsync(bluetoothAddress).get();
        if (device != nullptr) {
            // 返回设备名称
            return device.Name().c_str();
        }
    }
    catch (const std::exception& e) {
        std::wcerr << L"Error retrieving device name: " << e.what() << std::endl;
    }
    return L"Unknown";
}

std::wstring BluetoothLEManager::FormatBluetoothAddress(uint64_t address) {
    std::wstringstream stream;
    stream << std::hex << std::uppercase << std::setfill(L'0')
        << std::setw(2) << ((address >> 40) & 0xFF) << L":"
        << std::setw(2) << ((address >> 32) & 0xFF) << L":"
        << std::setw(2) << ((address >> 24) & 0xFF) << L":"
        << std::setw(2) << ((address >> 16) & 0xFF) << L":"
        << std::setw(2) << ((address >> 8) & 0xFF) << L":"
        << std::setw(2) << (address & 0xFF);
    return stream.str();
}