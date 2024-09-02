#ifndef BLUETOOTHLEPROTOCOL_H
#define BLUETOOTHLEPROTOCOL_H
#include <iostream>
#include <vector>
#include <cstdint>

//#pragma pack(1)  // Aligned by 1 byte

/**
 * @class BLEProtocol
 * @brief This class encapsulates the BLE protocol, providing structures and methods to handle
 *        requests, responses, and notifications over BLE communication.
 */
class BLEProtocol {
public:
    static constexpr uint8_t MSG_ID_MAX = 0x3F;  ///< Max value for message ID before it resets to 0.

    /**
     * @enum NotifyEventID
     * @brief Enum representing various notification event IDs.
     */
    enum class NotifyEventID : uint8_t {
        PairingOK = 0,        ///< BLE pairing success
        PairingTimeout,       ///< BLE pairing timeout
        Max = 0xFF            ///< Max value for event ID
    };

    /**
     * @enum MsgType
     * @brief Enum representing the type of BLE message (Request, Response, Notification).
     */
    enum class MsgType : uint8_t {
        Request = 0x0,        ///< Request command
        Response = 0x1,       ///< Response command
        Notification = 0x2    ///< Notification
    };

    /**
     * @struct Header
     * @brief Struct representing the header of a BLE protocol message.
     */
    struct Header {
        MsgType type:2;         ///< Message type (2 bits)
        uint8_t length:6;       ///< Payload length (6 bits)

        Header(MsgType type, uint8_t len) : type(type), length(len) {}
    };

    /**
     * @struct Request
     * @brief Struct representing a request message in the BLE protocol.
     */
    struct Request {
        uint8_t id;                     ///< Command ID
        std::vector<uint8_t> payload;   ///< Message payload
        uint8_t reserved[4] = { 0 };      ///< Reserved bytes

        Request(uint8_t id, const std::vector<uint8_t>& data)
            : id(id), payload(data) {}
    };

    /**
     * @struct Response
     * @brief Struct representing a response message in the BLE protocol.
     */
    struct Response {
        uint8_t id;                     ///< Command ID
        int8_t error;                   ///< Error number
        std::vector<uint8_t> payload;   ///< Message payload
        uint8_t reserved[4] = { 0 };      ///< Reserved bytes

        Response(uint8_t id, int8_t err, const std::vector<uint8_t>& data)
            : id(id), error(err), payload(data) {}
    };

    /**
     * @struct Notification
     * @brief Struct representing a notification message in the BLE protocol.
     */
    struct Notification {
        uint8_t id;                     ///< Notification ID
        NotifyEventID event_id;         ///< Event ID
        std::vector<uint8_t> payload;   ///< Message payload
        uint8_t reserved[4] = { 0 };      ///< Reserved bytes

        Notification(uint8_t id, NotifyEventID event, const std::vector<uint8_t>& data)
            : id(id), event_id(event), payload(data) {}
    };

    /**
     * @struct ProtoRequest
     * @brief Struct representing a BLE protocol request, including the header and checksum.
     */
    struct ProtoRequest {
        Header header;      ///< Protocol header
        Request request;    ///< Request message
        uint8_t checksum;   ///< Checksum (Sum of all bytes & 0xFF)

        ProtoRequest(Header hdr, Request req)
            : header(hdr), request(req), checksum(0) {}

        void calculateChecksum(std::vector<uint8_t> &combinedData);  ///< Calculate checksum for the request
    };

    /**
     * @struct ProtoResponse
     * @brief Struct representing a BLE protocol response, including the header and checksum.
     */
    struct ProtoResponse {
        Header header;      ///< Protocol header
        Response response;  ///< Response message
        uint8_t checksum;   ///< Checksum (Sum of all bytes & 0xFF)

        ProtoResponse(Header hdr, Response rsp)
            : header(hdr), response(rsp), checksum(0) {}

        void calculateChecksum(std::vector<uint8_t>& combinedData);  ///< Calculate checksum for the response
    };

    /**
     * @struct ProtoNotification
     * @brief Struct representing a BLE protocol notification, including the header and checksum.
     */
    struct ProtoNotification {
        Header header;          ///< Protocol header
        Notification notification;  ///< Notification message
        uint8_t checksum;       ///< Checksum (Sum of all bytes & 0xFF)

        ProtoNotification(Header hdr, Notification ntf)
            : header(hdr), notification(ntf), checksum(0) {}

        void calculateChecksum(std::vector<uint8_t>& combinedData);  ///< Calculate checksum for the notification
    };

    static void dncodeResponse(ProtoResponse& rsp, std::vector<uint8_t>& payload, uint16_t& payload_len);
    static void dncodeNotification(ProtoNotification& ntf, std::vector<uint8_t>& payload, uint16_t& payload_len);
    static uint8_t getProtoLength(const ProtoRequest& proto);
    static int checkChecksum(const ProtoRequest& proto, uint8_t proto_len, uint8_t sum);

    static uint8_t calculateChecksum(const uint8_t* data, size_t length);
};

//#pragma pack(pop)

#endif