#include "BluetoothLEManager.h"
#include <iostream>
#include <winrt/Windows.Foundation.Collections.h>

using namespace winrt;
using namespace Windows::Devices::Bluetooth;
using namespace Windows::Devices::Bluetooth::Advertisement;
using namespace Windows::Devices::Bluetooth::GenericAttributeProfile;
using namespace Windows::Storage::Streams;
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
            if (device.id.empty()) {
                std::cerr << "Device ID is empty." << std::endl;
                return;
            }

            try {
                // 使用设备 Address 创建 BluetoothLEDevice 实例
                connectedDevice = winrt::Windows::Devices::Bluetooth::BluetoothLEDevice::FromBluetoothAddressAsync(device.bluetoothAddress).get();

                if (connectedDevice) {
                    std::wcout << L"Connected to device: " << deviceName << std::endl;
                }
                else {
                    std::cerr << "Failed to connect to device." << std::endl;
                }
            }
            catch (const winrt::hresult_error& e) {
                // 捕获并输出详细的异常信息
                std::wcerr << L"Exception caught: " << e.message().c_str() << std::endl;
                std::wcerr << L"HResult: 0x" << std::hex << e.code() << std::endl;
            }

            break;
        }
    }
}

void BluetoothLEManager::DisconnectFromDevice() {
    if (connectedDevice) {
        connectedDevice.Close();
        connectedDevice = nullptr;
        std::wcout << L"Disconnected from device." << std::endl;
    }
    else {
        std::cerr << "No connected device to disconnect." << std::endl;
    }
}

void BluetoothLEManager::SendData(const std::vector<uint8_t>& data) {
    if (!connectedDevice) {
        std::cerr << "No connected device." << std::endl;
        return;
    }

    // 获取设备的 GATT 服务
    auto servicesResult = connectedDevice.GetGattServicesAsync().get();
    if (servicesResult.Status() != GattCommunicationStatus::Success) {
        std::cerr << "Failed to get GATT services." << std::endl;
        return;
    }

    auto services = servicesResult.Services();
    if (services.Size() == 0) {
        std::cerr << "No GATT services found." << std::endl;
        return;
    }

    // 假设选择第一个服务
    auto service = services.GetAt(3);

    // 获取 GATT 特征
    auto characteristicsResult = service.GetCharacteristicsAsync().get();
    if (characteristicsResult.Status() != GattCommunicationStatus::Success) {
        std::cerr << "Failed to get characteristics." << std::endl;
        return;
    }

    auto characteristics = characteristicsResult.Characteristics();
    if (characteristics.Size() == 0) {
        std::cerr << "No GATT characteristics found." << std::endl;
        return;
    }

    // 假设选择第一个特征
    auto characteristic = characteristics.GetAt(0);

    // 创建数据流
    auto writer = winrt::Windows::Storage::Streams::DataWriter();
    writer.WriteBytes(winrt::array_view<uint8_t const>(data));

    // 发送数据
    auto result = characteristic.WriteValueAsync(writer.DetachBuffer()).get();

    if (result == GattCommunicationStatus::Success) {
        std::cout << "Data sent successfully." << std::endl;
    }
    else {
        std::cerr << "Failed to send data." << std::endl;
    }
}

void BluetoothLEManager::ReceiveData() {
    if (!connectedDevice) {
        std::cerr << "No connected device." << std::endl;
        return;
    }

    // 获取设备的 GATT 服务
    auto servicesResult = connectedDevice.GetGattServicesAsync().get();
    if (servicesResult.Status() != GattCommunicationStatus::Success) {
        std::cerr << "Failed to get GATT services." << std::endl;
        return;
    }

    auto services = servicesResult.Services();
    if (services.Size() == 0) {
        std::cerr << "No GATT services found." << std::endl;
        return;
    }

    // 假设选择第一个服务
    auto service = services.GetAt(3);

    // 获取 GATT 特征
    auto characteristicsResult = service.GetCharacteristicsAsync().get();
    if (characteristicsResult.Status() != GattCommunicationStatus::Success) {
        std::cerr << "Failed to get characteristics." << std::endl;
        return;
    }

    auto characteristics = characteristicsResult.Characteristics();
    if (characteristics.Size() == 0) {
        std::cerr << "No GATT characteristics found." << std::endl;
        return;
    }

    // 假设选择第一个特征
    auto characteristic = characteristics.GetAt(0);

    // 注册通知事件处理程序
    characteristic.ValueChanged({ this, &BluetoothLEManager::OnCharacteristicValueChanged });

    // 启用通知
    auto result = characteristic.WriteClientCharacteristicConfigurationDescriptorAsync(GattClientCharacteristicConfigurationDescriptorValue::Notify).get();

    if (result == GattCommunicationStatus::Success) {
        std::cout << "Notifications enabled." << std::endl;
    }
    else {
        std::cerr << "Failed to enable notifications." << std::endl;
    }
}

void BluetoothLEManager::OnCharacteristicValueChanged(GattCharacteristic const& sender, GattValueChangedEventArgs const& args) {
    auto reader = winrt::Windows::Storage::Streams::DataReader::FromBuffer(args.CharacteristicValue());
    std::vector<uint8_t> data(reader.UnconsumedBufferLength());
    reader.ReadBytes(winrt::array_view<uint8_t>(data));

    std::cout << "Received data: ";
    for (auto byte : data) {
        std::cout << std::hex << static_cast<int>(byte) << ' ';
    }
    std::cout << std::endl;
}

void BluetoothLEManager::PrintDevices() const {
    std::wcout << L"Discovered devices:" << std::endl;
    std::wcout << L"Devices count: " << devices.size() << std::endl;
    for (const auto& device : devices) {
        std::wcout << L"Name: " << device.name << L", ID: " << device.id.c_str() << L", Address: " << device.address << L", Signal Strength: " << device.signalStrength << L" dBm" << std::endl;
        std::wcout.flush(); // 刷新缓冲区
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
            args.RawSignalStrengthInDBm(),
            address,
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