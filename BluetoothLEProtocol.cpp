#include "BluetoothLEProtocol.h"

// 计算给定数据的 checksum
uint8_t BLEProtocol::calculateChecksum(const uint8_t* data, size_t length) {
    uint8_t checksum = 0;
    for (size_t i = 0; i < length; ++i) {
        checksum += data[i];
    }
    return checksum & 0xFF;
}

// Implementation of ProtoRequest's checksum calculation using the unified method
void BLEProtocol::ProtoRequest::calculateChecksum(std::vector<uint8_t>& combinedData) {
    combinedData.resize(1); // Header should fit in 1 byte
    uint8_t byte = 0;
    byte |= static_cast<uint8_t>(header.type) & 0x03;    // Extract type (2 bits)
    byte |= (header.length - 2 & 0x3F) << 2;                 // Extract length (6 bits)
    combinedData[0] = byte;
    combinedData.push_back(request.id);
    combinedData.insert(combinedData.end(), request.payload.begin(), request.payload.end());
    combinedData.insert(combinedData.end(), std::begin(request.reserved), std::end(request.reserved));
    checksum = BLEProtocol::calculateChecksum(combinedData.data(), combinedData.size());
    std::wcout << L"checksum :" << checksum << std::endl;
    combinedData.push_back(checksum);
}

// Implementation of ProtoResponse's checksum calculation using the unified method
void BLEProtocol::ProtoResponse::calculateChecksum(std::vector<uint8_t>& combinedData) {
    combinedData.push_back(static_cast<uint8_t>(header.type));
    combinedData.push_back(header.length);
    combinedData.push_back(response.id);
    combinedData.push_back(response.error);
    combinedData.insert(combinedData.end(), response.payload.begin(), response.payload.end());
    checksum = BLEProtocol::calculateChecksum(combinedData.data(), combinedData.size());
}

// Implementation of ProtoNotification's checksum calculation using the unified method
void BLEProtocol::ProtoNotification::calculateChecksum(std::vector<uint8_t>& combinedData) {
    combinedData.push_back(static_cast<uint8_t>(header.type));
    combinedData.push_back(header.length);
    combinedData.push_back(notification.id);
    combinedData.push_back(static_cast<uint8_t>(notification.event_id));
    combinedData.insert(combinedData.end(), notification.payload.begin(), notification.payload.end());
    checksum = BLEProtocol::calculateChecksum(combinedData.data(), combinedData.size());
}


void BLEProtocol::dncodeResponse(ProtoResponse& rsp, std::vector<uint8_t>& payload, uint16_t& payload_len) {
    /*rsp.header = Header(MsgType::Response, 0);
    rsp.response = Response(id, err, {});*/
    //rsp.calculateChecksum();
}

void BLEProtocol::dncodeNotification(ProtoNotification& ntf, std::vector<uint8_t>& payload, uint16_t& payload_len) {
    /*ntf.header = Header(MsgType::Notification, 0);
    ntf.notification = Notification(0, event_id, {});*/
    //ntf.calculateChecksum();
}

uint8_t BLEProtocol::getProtoLength(const ProtoRequest& proto) {
    return sizeof(Header) + proto.header.length + sizeof(proto.checksum);
}

int BLEProtocol::checkChecksum(const ProtoRequest& proto, uint8_t proto_len, uint8_t sum) {
    // 计算实际的 checksum
    std::vector<uint8_t> combinedData;
    combinedData.push_back(static_cast<uint8_t>(proto.header.type));
    combinedData.push_back(proto.header.length);
    combinedData.push_back(proto.request.id);
    combinedData.insert(combinedData.end(), proto.request.payload.begin(), proto.request.payload.end());

    uint8_t calculated_sum = calculateChecksum(combinedData.data(), combinedData.size());
    return (calculated_sum == sum) ? 0 : -1;
}