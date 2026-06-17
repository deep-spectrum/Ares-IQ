/**
 * @file ubx_msg.hpp
 *
 * @brief
 *
 * @date 6/16/26
 *
 * @author Tom Schmitz \<tschmitz@andrew.cmu.edu\>
 */

#ifndef ARES_UBX_MSG_HPP
#define ARES_UBX_MSG_HPP

#include <cstdint>
#include <exception>
#include <string>
#include <vector>

/**
 * @enum UbxMsgType
 * The UBX frame types. Split into class/id.
 */
enum UbxMsgType : uint16_t {
    // UBX Class ACK
    ACK_ACK = 0x0501,
    ACK_NACK = 0x0500,

    // UBX Class AID
    AID_ALM = 0x0B30,
    AID_AOP = 0x0B33,
    AID_EPH = 0x0B31,
    AID_HUI = 0x0B02,
    AID_INI = 0x0B01,

    // UBX Class CFG
    CFG_ANT = 0x0613,
    CFG_BATCH = 0x0693,
    CFG_CFG = 0x0609,
    CFG_DAT = 0x0606,
    CFG_DGNSS = 0x0670,
    CFG_DOSC = 0x0661,
    CFG_ESFALG = 0x0656,
    CFG_ESFA = 0x064C,
    CFG_ESFG = 0x064D,
    CFG_ESFWT = 0x0682,
    CFG_ESRC = 0x0660,
    CFG_GEOFENCE = 0x0669,
    CFG_GNSS = 0x063E,
    CFG_HNR = 0x065C,
    CFG_INF = 0x0602,
    CFG_ITFM = 0x0639,
    CFG_LOGFILTER = 0x0647,
    CFG_MSG = 0x0601,
    CFG_NAV5 = 0x0624,
    CFG_NAVX5 = 0x0623,
    CFG_NMEA = 0x0617,
    CFG_ODO = 0x061E,
    CFG_PM2 = 0x063B,
    CFG_PMS = 0x0686,
    CFG_PRT = 0x0600,
    CFG_PWR = 0x0657,
    CFG_RATE = 0x0608,
    CFG_RINV = 0x0634,
    CFG_RST = 0x0604,
    CFG_RXM = 0x0611,
    CFG_SBAS = 0x0616,
    CFG_SENIF = 0x0688,
    CFG_SLAS = 0x068D,
    CFG_SMGR = 0x0662,
    CFG_SPT = 0x0664,
    CFG_TMODE2 = 0x063D,
    CFG_TMODE3 = 0x0671,
    CFG_TP5 = 0x0631,
    CFG_TXSLOT = 0x0653,
    CFG_USB = 0x061B,

    // UBX Class ESF
    ESF_ALG = 0x1014,
    ESF_INS = 0x1015,
    ESF_MEAS = 0x1002,
    ESF_RAW = 0x1003,
    ESF_STATUS = 0x1010,

    // UBX Class HNR
    HNR_ATT = 0x2801,
    HNR_INS = 0x2802,
    HNR_PVT = 0x2800,

    // UBX Class INF
    INF_DEBUG = 0x0404,
    INF_ERROR = 0x0400,
    INF_NOTICE = 0x0402,
    INF_TEST = 0x0403,
    INF_WARNING = 0x0401,

    // UBX Class LOG
    LOG_BATCH = 0x2111,
    LOG_CREATE = 0x2107,
    LOG_ERASE = 0x2103,
    LOG_FINDTIME = 0x210E,
    LOG_INFO = 0x2108,
    LOG_RETRIEVEBATCH = 0x2110,
    LOG_RETRIEVEPOSEXTRA = 0x210f,
    LOG_RETRIEVEPOS = 0x210b,
    LOG_RETRIEVESTRING = 0x210d,
    LOG_RETRIEVE = 0x2109,
    LOG_STRING = 0x2104,

    // UBX Class MGA
    MGA_ACK_DATA0 = 0x1360,
    MGA_ANO = 0x1320,
    MGA_BDS_EPH = 0x1303,
    MGA_BDS_ALM = 0x1303,
    MGA_BDS_HEALTH = 0x1303,
    MGA_BDS_UTC = 0x1303,
    MGA_BDS_IONO = 0x1303,
    MGA_DBD = 0x1380,
    MGA_FLASH_DATA = 0x1321,
    MGA_FLASH_STOP = 0x1321,
    MGA_FLASH_ACK = 0x1321,
    MGA_GAL_EPH = 0x1302,
    MGA_GAL_ALM = 0x1302,
    MGA_GAL_TIMEOFFSET = 0x1302,
    MGA_GAL_UTC = 0x1302,
    MGA_GLO_EPH = 0x1306,
    MGA_GLO_ALM = 0x1306,
    MGA_GLO_TIMEOFFSET = 0x1306,
    MGA_GPS_EPH = 0x1300,
    MGA_GPS_ALM = 0x1300,
    MGA_GPS_HEALTH = 0x1300,
    MGA_GPS_UTC = 0x1300,
    MGA_GPS_IONO = 0x1300,
    MGA_INI_POS_XYZ = 0x1340,
    MGA_INI_POS_LLH = 0x1340,
    MGA_INI_TIME_UTC = 0x1340,
    MGA_INI_TIME_GNSS = 0x1340,
    MGA_INI_CLKD = 0x1340,
    MGA_INI_FREQ = 0x1340,
    MGA_INI_EOP = 0x1340,
    MGA_QZSS_EPH = 0x1305,
    MGA_QZSS_ALM = 0x1305,
    MGA_QZSS_HEALTH = 0x1305,

    // UBX Class MON
    MON_BATCH = 0x0A32,
    MON_GNSS = 0x0A28,
    MON_HW2 = 0x0A0B,
    MON_HW = 0x0A09,
    MON_IO = 0x0A02,
    MON_MSGPP = 0x0A06,
    MON_PATCH = 0x0A27,
    MON_RXBUF = 0x0A07,
    MON_RXR = 0x0A21,
    MON_SMGR = 0x0A2E,
    MON_SPT = 0x0A2F,
    MON_TXBUF = 0x0A08,
    MON_VER = 0x0A04,

    // UBX Class NAV
    NAV_AOPSTATUS = 0x0160,
    NAV_ATT = 0x0105,
    NAV_CLOCK = 0x0122,
    NAV_COV = 0x0136,
    NAV_DGPS = 0x0131,
    NAV_DOP = 0x0104,
    NAV_EELL = 0x013d,
    NAV_EOE = 0x0161,
    NAV_GEOFENCE = 0x0139,
    NAV_HPPOSECEF = 0x0113,
    NAV_HPPOSLLH = 0x0114,
    NAV_NMI = 0x0128,
    NAV_ODO = 0x0109,
    NAV_ORB = 0x0134,
    NAV_POSECEF = 0x0101,
    NAV_POSLLH = 0x0102,
    NAV_PVT = 0x0107,
    NAV_RELPOSNED = 0x013C,
    NAV_RESETODO = 0x0110,
    NAV_SAT = 0x0135,
    NAV_SBAS = 0x0132,
    NAV_SLAS = 0x0142,
    NAV_SOL = 0x0106,
    NAV_STATUS = 0x0103,
    NAV_SVINFO = 0x0130,
    NAV_SVIN = 0x013B,
    NAV_TIMEBDS = 0x0124,
    NAV_TIMEGAL = 0x0125,
    NAV_TIMEGLO = 0x0123,
    NAV_TIMEGPS = 0x0120,
    NAV_TIMELS = 0x0126,
    NAV_TIMEUTC = 0x0121,
    NAV_VELECEF = 0x0111,
    NAV_VELNED = 0x0112,

    // UBX Class RXM
    RXM_IMES = 0x0261,
    RXM_MEASX = 0x0214,
    RXM_PMREQ = 0x0241,
    RXM_RAWX = 0x0215,
    RXM_RLM = 0x0259,
    RXM_RTCM = 0x0232,
    RXM_SFRBX = 0x0213,
    RXM_SVSI = 0x0220,

    // UBX Class SEC
    SEC_UNIQID = 0x2703,

    // UBX Class TIM
    TIM_DOSC = 0x0D11,
    TIM_FCHG = 0x0D16,
    TIM_HOC = 0x0D17,
    TIM_SMEAS = 0x0D13,
    TIM_SVIN = 0x0D04,
    TIM_TM2 = 0x0D03,
    TIM_TOS = 0x0D12,
    TIM_TP = 0x0D01,
    TIM_VCOCAL = 0x0D15,
    TIM_VRFY = 0x0D06,

    // UBX Class UPD
    UPD_SOS = 0x0914,

    // Invalid
    NO_TYPE = 0
};

/**
 * @struct UbxMsg
 * Structured UBX message.
 */
struct UbxMsg {
    /**
     * The message type (message class and message ID).
     */
    UbxMsgType type = NO_TYPE;

    /**
     * The message payload.
     */
    std::vector<uint8_t> payload;

    /**
     * Checksum A.
     */
    uint8_t ck_a = 0;

    /**
     * Checksum B.
     */
    uint8_t ck_b = 0;

    /**
     * Flag indicating that the received checksum and the calculated checksum
     * does not match.
     */
    bool bad_checksum = false;
};

/**
 * @class UbxException
 * Ubx message API exception.
 */
class UbxException : std::exception {
  public:
    /**
     * Constructor.
     * @param msg The error message.
     */
    explicit UbxException(const char *msg) : _msg(msg) {}

    /**
     * What caused the exception.
     * @return The error message.
     */
    const char *what() const noexcept override { return _msg.c_str(); }

  private:
    std::string _msg;
};

/**
 * @struct UbxMonVerPayload
 * The structured payload of the UBX-MON-VER response.
 */
struct UbxMonVerPayload {
    /**
     * The software version string.
     */
    std::string sw_version;

    /**
     * The hardware version string.
     */
    std::string hw_version;

    /**
     * Any extra version strings.
     */
    std::vector<std::string> extension;
};

/**
 * Serialize a UBX message.
 *
 * @param[in] msg The UBX message to construct.
 * @param[in,out] buffer The buffer to place the serialized UBX message in.
 *
 * @throws UbxException if message type is @p NO_TYPE.
 *
 * @note Clears @p buffer before placing the serialized UBX message into the
 * buffer.
 */
void build_ubx_msg(const UbxMsg &msg, std::vector<uint8_t> &buffer);

/**
 * Parse UBX messages from a buffer.
 *
 * @param[in] nmea The nmea message buffer to parse the UBX messages from.
 * @param[in] len The length of the nmea message buffer.
 * @param[in,out] msg_list The vector to output the deserialized messages to.
 *
 * @note @p msg_list gets cleared on each invocation.
 */
void parse_ubx_msg(const uint8_t *nmea, size_t len,
                   std::vector<UbxMsg> &msg_list);

/**
 * Parse a UBX-MON-VER message payload.
 *
 * @param[in] msg The UBX message to parse the payload for. Must be of type @p
 * MON_VER.
 * @param[in,out] payload The deserialized payload.
 *
 * @throws UbxException if message type is not @p MON_VER.
 * @throws UbxException if the message as a bad checksum.
 * @throws UbxException if the payload is malformed.
 */
void parse_ubx_mon_ver(const UbxMsg &msg, UbxMonVerPayload &payload);

#endif // ARES_UBX_MSG_HPP
