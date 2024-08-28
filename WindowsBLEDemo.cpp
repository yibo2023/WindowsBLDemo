#include "BluetoothLEManager.h"
#include <windows.h>
#include <iostream>
#include <string>

int main() {
    try {
        // 创建 BluetoothLEManager 实例
        BluetoothLEManager bleManager;

        // 扫描附近的 BLE 设备
        bleManager.ScanForDevices();

        // 选择要连接的设备名称
        std::wstring deviceToConnect = L"Device Name"; // 替换为你想连接的设备名称
        bleManager.ConnectToDevice(deviceToConnect);

        // 发送数据到连接的设备
        std::vector<uint8_t> dataToSend = { 0x01, 0x02, 0x03, 0x01, 0x02, 0x03, 0x01, 0x02, 0x03, 0x01, 0x02, 0x03 }; // 示例数据
        bleManager.SendData(dataToSend);

        // 接收数据从连接的设备
        bleManager.ReceiveData();

        std::wcout << L"Press Ctrl+C to exit..." << std::endl;
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(5)); // 每5秒输出一次信息
            std::wcout << L"Waiting..." << std::endl;
        }
    }
    catch (const std::exception& ex) {
        std::cerr << "Exception: " << ex.what() << std::endl;
    }

    return 0;
}
