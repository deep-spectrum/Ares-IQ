/**
 * @file ubx_msg.cpp
 *
 * @brief
 *
 * @date 6/16/26
 *
 * @author Tom Schmitz \<tschmitz@andrew.cmu.edu\>
 */

#include <ares-iq/signal-hound/ubx_msg.hpp>
#include <cassert>
#include <cstring>

constexpr uint8_t hdr0 = 0xB5;
constexpr uint8_t hdr1 = 0x62;

constexpr uint8_t ubx_frame_header_overhead = 6;
constexpr uint8_t ubx_frame_header_meta_overhead = 4;
constexpr uint8_t ubx_frame_overhead = 8;

constexpr size_t hdr0_offset = 0;
constexpr size_t hdr1_offset = 1;
constexpr size_t class_offset = 2;
constexpr size_t id_offset = 3;
constexpr size_t len_offset = 4;
constexpr size_t payload_offset = 6;

static void calculate_checksum(const uint8_t *mem, size_t len, uint8_t &CK_A,
                               uint8_t &CK_B) {
    CK_A = 0;
    CK_B = 0;

    for (size_t i = 0; i < len; i++) {
        CK_A = CK_A + mem[i];
        CK_B = CK_B + CK_A;
    }
}

static void generate_checksum(std::vector<uint8_t> &buffer) {
    uint8_t CK_A, CK_B;

    calculate_checksum(buffer.data() + 2, buffer.size() - 2, CK_A, CK_B);

    buffer.emplace_back(CK_A);
    buffer.emplace_back(CK_B);
}

static uint8_t get_class(UbxMsgType type) {
    return static_cast<uint8_t>((type >> 8) & UINT8_MAX);
}

static uint8_t get_id(UbxMsgType type) {
    return static_cast<uint8_t>(type & UINT8_MAX);
}

void build_ubx_msg(const UbxMsg &msg, std::vector<uint8_t> &buffer) {
    buffer.clear();

    if (msg.type == NO_TYPE) {
        throw UbxException("Invalid message type");
    }

    buffer.emplace_back(hdr0);
    buffer.emplace_back(hdr1);
    buffer.emplace_back(get_class(msg.type));
    buffer.emplace_back(get_id(msg.type));

    uint16_t payload_len = static_cast<uint16_t>(msg.payload.size());
    uint8_t payload_len_bytes[sizeof(uint16_t)];
    (void)memcpy(payload_len_bytes, &payload_len, sizeof(uint16_t));
    buffer.insert(buffer.end(), payload_len_bytes,
                  payload_len_bytes + sizeof(uint16_t));

    if (payload_len != 0) {
        buffer.insert(buffer.end(), msg.payload.begin(), msg.payload.end());
    }

    generate_checksum(buffer);
}

static UbxMsgType get_message_type(uint16_t class_id, uint16_t id) {
    UbxMsgType ret = static_cast<UbxMsgType>(((class_id & UINT8_MAX) << 8) |
                                             (id & UINT8_MAX));
    return ret;
}

static bool parse(const uint8_t *nmea, size_t len, size_t &i,
                  std::vector<UbxMsg> &msg_list) {
    if ((i + hdr1_offset) >= len) {
        return true;
    }

    if (!(nmea[i] == hdr0 && nmea[i + hdr1_offset] == hdr1)) {
        // UBX message not found
        i++;
        return false;
    }

    // UBX message found
    UbxMsg msg;

    msg.type = get_message_type(nmea[i + class_offset], nmea[i + id_offset]);

    uint16_t payload_len;
    (void)memcpy(&payload_len, &nmea[i + len_offset], sizeof(payload_len));

    if ((i + ubx_frame_overhead + payload_len) > len) {
        return true;
    }

    const uint8_t *payload = nmea + i + payload_offset;
    msg.payload.assign(payload, &payload[payload_len - 1]);

    if ((i + ubx_frame_header_overhead + payload_len + 1) >= len) {
        // Incomplete frame
        return true;
    }

    msg.ck_a = nmea[i + ubx_frame_header_overhead + payload_len];
    msg.ck_b = nmea[i + ubx_frame_header_overhead + payload_len + 1];

    uint8_t ck_a, ck_b;
    calculate_checksum(&nmea[i + class_offset],
                       ubx_frame_header_meta_overhead + payload_len, ck_a,
                       ck_b);
    msg.bad_checksum = (msg.ck_a != ck_a) || (msg.ck_b != ck_b);
    i += ubx_frame_overhead + payload_len;

    msg_list.emplace_back(msg);

    return false;
}

void parse_ubx_msg(const uint8_t *nmea, size_t len,
                   std::vector<UbxMsg> &msg_list) {
    msg_list.clear();

    if (len < ubx_frame_overhead) {
        return;
    }

    bool break_loop = false;
    for (size_t i = 0; i < len && !break_loop;) {
        break_loop = parse(nmea, len, i, msg_list);
    }
}
