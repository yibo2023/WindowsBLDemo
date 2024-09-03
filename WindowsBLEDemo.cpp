﻿#include "BluetoothLEManager.h"
#include "BluetoothLEProtocol.h"
#include <windows.h>
#include <iostream>
#include <string>

//int main() {
//    try {
//        // 创建 BluetoothLEManager 实例
//        BluetoothLEManager bleManager;
//
//        // 扫描附近的 BLE 设备
//        bleManager.ScanForDevices();
//
//        // 选择要连接的设备名称
//        //std::wstring deviceToConnect = L"Device Name"; // 替换为你想连接的设备名称
//        std::wstring deviceToConnect = L"EAF_574775"; // 替换为你想连接的设备名称
//        //std::wstring deviceToConnect = L"S50_39cf5edd"; // 替换为你想连接的设备名称
//        //std::wstring deviceToConnect = L"EAF_5739a5"; // 替换为你想连接的设备名称
//        bleManager.ConnectToDevice(deviceToConnect);
//
//        // 接收数据从连接的设备
//        bleManager.ReceiveData(3, 0);
//
//        // 创建 Header
//        BLEProtocol::Header header(BLEProtocol::MsgType::Request, 10);
//
//        // 创建 Request
//        //std::vector<uint8_t> payload = { 0x7e, 0x5a, 0x03, 0x01, 0x50, 0x00, 0x00, 0x75, 0x00 };
//        std::vector<uint8_t> payload = { 'B', 'P', '1', '2', '3', '4', '5', '6' };
//        BLEProtocol::Request request(1, 1,payload);
//
//        // 创建 ProtoRequest
//        BLEProtocol::ProtoRequest protoRequest(header, request);
//
//        // 计算校验和
//        std::vector<uint8_t> combinedData;
//        combinedData.clear();
//        BLEProtocol::encodeRequest(protoRequest, combinedData);
//
//        std::wcout << L"combinedData.size:" << combinedData.size() << std::endl;
//        
//        // 发送数据到连接的设备
//        //std::vector<uint8_t> dataToSend = { 0x01, 0x02, 0x03, 0x01, 0x02, 0x03, 0x01, 0x02, 0x03, 0x01, 0x02, 0x03 }; // 示例数据
//        bleManager.SendData(combinedData,3,0);
//
//        std::wcout << L"Press Ctrl+C to exit..." << std::endl;
//        while (true) {
//            std::this_thread::sleep_for(std::chrono::seconds(5)); // 每5秒输出一次信息
//            std::wcout << L"Waiting..." << std::endl;
//        }
//    }
//    catch (const std::exception& ex) {
//        std::cerr << "Exception: " << ex.what() << std::endl;
//    }
//
//    return 0;
//}

void runBLEOperations() {
    try {
        // 创建 BluetoothLEManager 实例
        BluetoothLEManager bleManager;

        // 扫描附近的 BLE 设备
        bleManager.ScanForDevices();

        // 选择要连接的设备名称
        std::wstring deviceToConnect = L"EAF_574775"; // 替换为你想连接的设备名称
        bleManager.ConnectToDevice(deviceToConnect);

        // 接收数据从连接的设备
        bleManager.ReceiveData(3, 0);

        // 创建 Header
        BLEProtocol::Header header(BLEProtocol::MsgType::Request, 10);

        // 创建 Request
        std::vector<uint8_t> payload = { 'B', 'P', '1', '2', '3', '4', '5', '6' };
        BLEProtocol::Request request(1, 0, payload);

        // 创建 ProtoRequest
        BLEProtocol::ProtoRequest protoRequest(header, request);

        // 计算校验和
        std::vector<uint8_t> combinedData;
        BLEProtocol::encodeRequest(protoRequest, combinedData);

        std::wcout << L"combinedData.size: " << combinedData.size() << std::endl;

        // 发送数据到连接的设备
        bleManager.SendData(combinedData, 3, 0);

        // 主线程等待接收数据
        bleManager.WaitForDataAndProcess();

        // 数据准备好，开始处理解码
        std::vector<uint8_t> receivedData = bleManager.GetReceivedData();

        BLEProtocol::decodeNotification(receivedData);

        // 阻塞主线程直到输入
        std::wcout << L"Press Enter to exit..." << std::endl;
        std::cin.get(); // 等待用户输入，按 Enter 键退出
    }
    catch (const std::exception& ex) {
        std::cerr << "Exception: " << ex.what() << std::endl;
    }
}

int main() {
    // 创建并启动线程
    std::thread bleThread(runBLEOperations);

    // 主线程可以进行其他工作或者简单地等待
    std::wcout << L"Main thread is waiting for BLE operations to complete..." << std::endl;

    // 等待线程完成
    bleThread.join();

    return 0;
}