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

    //PrintDevices();
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
                m_connectedDevice = winrt::Windows::Devices::Bluetooth::BluetoothLEDevice::FromBluetoothAddressAsync(device.bluetoothAddress).get();

                if (m_connectedDevice) {
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
    if (m_connectedDevice) {
        m_connectedDevice.Close();
        m_connectedDevice = nullptr;
        std::wcout << L"Disconnected from device." << std::endl;
    }
    else {
        std::cerr << "No connected device to disconnect." << std::endl;
    }
}

// 获取指定索引的 GATT 服务
winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattDeviceService BluetoothLEManager::GetGattService(uint32_t index) {
    if (!m_connectedDevice) {
        throw std::runtime_error("No connected device.");
    }

    auto servicesResult = m_connectedDevice.GetGattServicesAsync().get();
    if (servicesResult.Status() != GattCommunicationStatus::Success) {
        throw std::runtime_error("Failed to get GATT services.");
    }

    auto services = servicesResult.Services();
    if (services.Size() <= index) {
        throw std::runtime_error("Requested GATT service index out of bounds.");
    }

    return services.GetAt(index);
}

// 获取指定索引的 GATT 特征
winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattCharacteristic BluetoothLEManager::GetGattCharacteristic(const winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattDeviceService& service, uint32_t index) {
    auto characteristicsResult = service.GetCharacteristicsAsync().get();
    if (characteristicsResult.Status() != GattCommunicationStatus::Success) {
        throw std::runtime_error("Failed to get characteristics.");
    }

    auto characteristics = characteristicsResult.Characteristics();
    if (characteristics.Size() <= index) {
        throw std::runtime_error("Requested GATT characteristic index out of bounds.");
    }

    return characteristics.GetAt(index);
}

void BluetoothLEManager::SendData(const std::vector<uint8_t>& data, uint32_t serviceIndex, uint32_t characteristicIndex) {
    try {
        auto service = GetGattService(serviceIndex);
        auto characteristic = GetGattCharacteristic(service, characteristicIndex);

        // 创建数据流
        auto writer = winrt::Windows::Storage::Streams::DataWriter();
        writer.WriteBytes(winrt::array_view<uint8_t const>(data));
        //writer.WriteBytes(data);

        // 发送数据
        auto result = characteristic.WriteValueAsync(writer.DetachBuffer()).get();

        if (result == GattCommunicationStatus::Success) {
            std::cout << "Data sent successfully." << std::endl;
        }
        else {
            std::cerr << "Failed to send data." << std::endl;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Exception caught: " << e.what() << std::endl;
    }
}

void BluetoothLEManager::ReceiveData(uint32_t serviceIndex, uint32_t characteristicIndex) {
    try {
        auto service = GetGattService(serviceIndex);
        auto characteristic = GetGattCharacteristic(service, characteristicIndex);

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
    catch (const std::exception& e) {
        std::cerr << "Exception caught: " << e.what() << std::endl;
    }
}

const std::vector<uint8_t>& BluetoothLEManager::GetReceivedData() const {
    return m_vReceivedData;
}

void BluetoothLEManager::OnCharacteristicValueChanged(GattCharacteristic const& sender, GattValueChangedEventArgs const& args) {
    auto reader = winrt::Windows::Storage::Streams::DataReader::FromBuffer(args.CharacteristicValue());
    m_vReceivedData.clear();
    m_vReceivedData.resize(reader.UnconsumedBufferLength());
    reader.ReadBytes(m_vReceivedData);

    std::cout << "Received data: ";
    for (auto byte : m_vReceivedData) {
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