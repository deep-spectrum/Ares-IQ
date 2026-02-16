#ifndef SH_VRT_H
#define SH_VRT_H

#include <cstdint>
#include <cstring>
#include <vector>

typedef int64_t VrtFreq;
typedef int16_t VrtTemp;
typedef int16_t VrtGain;
typedef int16_t VrtRef;

#define VRT_REF_POINT_1 0x01000001 // RF input port 1
// Max packet size in words minus the word count of the data packet
// header+trailer
#define MIN_VRT_DATA_SAMPLES      (128)
#define MAX_VRT_DATA_SAMPLES      (65535 - 6)

#define VRT_DATA_PKT_TYPE         1
#define VRT_CNTX_PKT_TYPE         4

#define VRT_CNTX_HDR_PKT_TYPE     (1 << 30)
#define VRT_CNTX_HDR_TSM          (1 << 24)

#define VRT_DATA_HDR_PKT_TYPE     (1 << 28)
#define VRT_DATA_HDR_T            (1 << 26)
#define VRT_DATA_TRAILER_E_BIT    (1 << 7)

#define VRT_HDR_TSI               (1 << 22)
#define VRT_HDR_TSF               (1 << 21)

#define VRT_TIME_ENABLE           (1 << 31)
#define VRT_VALID_DATA_ENABLE     (1 << 30)
#define VRT_REF_LOCK_ENABLE       (1 << 29)
#define VRT_OVER_RANGE_ENABLE     (1 << 25)
#define VRT_SAMPLE_LOSS_ENABLE    (1 << 24)

#define VRT_TIME_INDICATOR        (1 << 19)
#define VRT_VALID_DATA_INDICATOR  (1 << 18)
#define VRT_REF_LOCK_INDICATOR    (1 << 17)
#define VRT_OVER_RANGE_INDICATOR  (1 << 13)
#define VRT_SAMPLE_LOSS_INDICATOR (1 << 12)

// Context Indicator Field Bits
#define VRT_CNTX_FIELD_CHANGE_BIT       (1 << 31)
#define VRT_CNTX_REFERENCE_POINT_BIT    (1 << 30)
#define VRT_CNTX_BANDWIDTH_BIT          (1 << 29)
#define VRT_CNTX_IF_FREQ_BIT            (1 << 28)
#define VRT_CNTX_RF_FREQ_BIT            (1 << 27)
#define VRT_CNTX_FREQ_OFFSET_BIT        (1 << 26)
#define VRT_CNTX_IF_BAND_OFFSET_BIT     (1 << 25)
#define VRT_CNTX_REFERENCE_LEVEL_BIT    (1 << 24)
#define VRT_CNTX_GAIN_BIT               (1 << 23)
#define VRT_CNTX_OVER_RANGE_BIT         (1 << 22)
#define VRT_CNTX_SAMPLE_RATE_BIT        (1 << 21)
#define VRT_CNTX_TIMESTAMP_ADJUST_BIT   (1 << 20)
#define VRT_CNTX_TIMESTAMP_CAL_BIT      (1 << 19)
#define VRT_CNTX_TEMPERATURE_BIT        (1 << 18)
#define VRT_CNTX_DEVICE_ID_BIT          (1 << 17)
#define VRT_CNTX_STATE_EVENT_BIT        (1 << 16)
#define VRT_CNTX_IF_PAYLOAD_BIT         (1 << 15)
#define VRT_CNTX_FORMATTED_GPS_BIT      (1 << 14)
#define VRT_CNTX_FORMATTED_INS_BIT      (1 << 13)
#define VRT_CNTX_ECEF_BIT               (1 << 12)
#define VRT_CNTX_RELATIVE_EPHEMERIS_BIT (1 << 11)
#define VRT_CNTX_EPHEMERIS_REF_BIT      (1 << 10)
#define VRT_CNTX_GPS_ASCII_BIT          (1 << 9)

// Context Indicator Field Sizes
#define VRT_CNTX_FIELD_CHANGE_SIZE       (0)
#define VRT_CNTX_REFERENCE_POINT_SIZE    (1)
#define VRT_CNTX_BANDWIDTH_SIZE          (2)
#define VRT_CNTX_IF_FREQ_SIZE            (2)
#define VRT_CNTX_RF_FREQ_SIZE            (2)
#define VRT_CNTX_FREQ_OFFSET_SIZE        (2)
#define VRT_CNTX_IF_BAND_OFFSET_SIZE     (2)
#define VRT_CNTX_REFERENCE_LEVEL_SIZE    (1)
#define VRT_CNTX_GAIN_SIZE               (1)
#define VRT_CNTX_OVER_RANGE_SIZE         (1)
#define VRT_CNTX_SAMPLE_RATE_SIZE        (2)
#define VRT_CNTX_TIMESTAMP_ADJUST_SIZE   (2)
#define VRT_CNTX_TIMESTAMP_CAL_SIZE      (1)
#define VRT_CNTX_TEMPERATURE_SIZE        (1)
#define VRT_CNTX_DEVICE_ID_SIZE          (2)
#define VRT_CNTX_STATE_EVENT_SIZE        (1)
#define VRT_CNTX_IF_PAYLOAD_SIZE         (2)
#define VRT_CNTX_FORMATTED_GPS_SIZE      (11)
#define VRT_CNTX_FORMATTED_INS_SIZE      (11)
#define VRT_CNTX_ECEF_SIZE               (13)
#define VRT_CNTX_RELATIVE_EPHEMERIS_SIZE (13)
#define VRT_CNTX_EPHEMERIS_REF_SIZE      (1)
#define VRT_CNTX_GPS_ASCII_SIZE          (1)

typedef enum SmVRTPacketType {
    smVRTDataPacket = 0,
    smVRTContextPacket = 1,
    smVRTInvalidPacket = 2
} SmVRTPacketType;

//
// Signal Hound VRT (VITA 49) packet structures in memory
//

// Common to all VRT packets
typedef struct VRTPktPrologue {
    uint32_t header;
    uint32_t streamIdent;
    uint32_t seconds;
    uint32_t picoUpper;
    uint32_t picoLower;
} VRTPktPrologue;

// Signal Data Packet metadata
typedef struct VRTDataPktMetadata {
    VRTPktPrologue prologue;
} VRTDataPktMetadata;

// Context Packet metadata
typedef struct VRTContextPktMetadata {
    VRTPktPrologue prologue;
    uint32_t indicators;
} VRTContextPktMetadata;

// VITA 49 GPS subfield format
typedef struct VRTFormattedGPS {
    uint32_t header;
    uint32_t timestampInt;
    uint32_t timestampFracUpper;
    uint32_t timestampFracLower;
    int32_t latitude;
    int32_t longitude;
    uint32_t altitude;
    int32_t speedOverGround;
    uint32_t headingAngle;
    int32_t trackAngle;
    uint32_t magneticVariation;
} VRTFormattedGPS;

// Context Packet payload fields
typedef struct VRTContextFields {
    int64_t bandwidth;
    int64_t frequency;
    uint32_t reference;
    uint32_t gain;
    int64_t sampleRate;
    uint32_t temperature;
    uint32_t devUid;
    uint32_t devModel;
    VRTFormattedGPS formattedGps;
} VRTContextFields;

//
// VRT packets parsed for easy use
//

typedef struct VRTUserPktHeader {
    uint32_t packetType;
    uint32_t packetCount;
    uint32_t packetSize;
} VRTUserPktHeader;

typedef struct VRTUserPktPrologue {
    VRTUserPktHeader header;
    uint32_t streamIdent;
    uint32_t seconds;
    uint32_t picoUpper;
    uint32_t picoLower;
} VRTUserPktPrologue;

typedef struct VRTUserDataTrailerField {
    VRTUserDataTrailerField() : enabled(false), indicator(false) {}

    bool enabled;
    bool indicator;
} VRTUserDataTrailerField;

typedef struct VRTUserDataTrailer {
    VRTUserDataTrailerField isCalibratedTime;
    VRTUserDataTrailerField isValidData;
    VRTUserDataTrailerField isReferenceLock;
    VRTUserDataTrailerField isOverRange;
    VRTUserDataTrailerField isSampleLoss;
    uint32_t associatedContextPktCount;
} VRTUserDataTrailer;

typedef struct VRTUserDataPkt {
    VRTUserDataPkt &operator=(const VRTUserDataPkt &pkt) {
        prologue = pkt.prologue;
        trailer = pkt.trailer;

        data.resize(pkt.data.size());
        memcpy(&data[0], &pkt.data[0], data.size());

        return *this;
    }
    VRTUserPktPrologue prologue;
    std::vector<float> data;
    VRTUserDataTrailer trailer;
} VRTUserDataPkt;

typedef struct VRTUserContextIndicators {
    bool isContextFieldChanged;
    bool isBandwidth;
    bool isRfFreq;
    bool isReflevel;
    bool isaAtten;
    bool isSampleRate;
    bool isTemperature;
    bool isDevUid;
    bool isDevModel;
    bool isGPS;
} VRTUserContextIndicators;

typedef struct VRTUserGPS {
    double latitude;
    double longitude;
    double altitude;
    uint32_t seconds;
    uint32_t picoUpper;
    uint32_t picoLower;
} VRTUserGPS;

typedef struct VRTUserContextPkt {
    VRTUserPktPrologue prologue;
    VRTUserContextIndicators indicators;
    bool fieldChanged;
    double bandwidth;
    double rfFreq;
    double reflevel;
    double atten;
    double sampleRate;
    double temperature;
    int devUid;
    double devModel;
    VRTUserGPS gps;
} VRTUserContextPkt;

inline void vrtSetBit(uint32_t &input, uint32_t bit) { input |= bit; }

inline uint32_t vrtGetBit(uint32_t &input, uint32_t bit) { return input & bit; }

inline uint32_t swapWord(uint32_t input) {
    return ((input >> 24) & 0x000000FF) | ((input >> 8) & 0x0000FF00) |
           ((input << 8) & 0x00FF0000) | ((input << 24) & 0xFF000000);
}

// Swap between big-little endian
int vrtSwapBytes(uint32_t *srcDst, uint32_t wordCount);
int vrtSwapBytes(const uint32_t *src, uint32_t *dst, uint32_t wordCount);

// Fixed-float conversions for VRT fixed point types
VrtTemp vrtConvertFloatToTemp(float temp);
float vrtConvertTempToFloat(VrtTemp temp);
VrtGain vrtConvertFloatToGain(float gain);
float vrtConvertGainToFloat(VrtGain gain);
VrtRef vrtConvertFloatToRef(float ref);
float vrtConvertRefToFloat(VrtRef ref);
VrtFreq vrtConvertFloatToFreq(double freq);
double vrtConvertFreqToFloat(VrtFreq freq);
int32_t vrtConvertFloatToFixed(float val, int fracBits);
int32_t vrtConvertFloatToFixed(double val, int fracBits);
float vrtConvertFixedToFloat(int32_t val, int fracBits);

uint64_t vrtGetTime();

uint32_t vrtGetPacketType(uint32_t pktHdr);
uint32_t vrtGetPacketCount(uint32_t pktHdr);
uint32_t vrtGetPacketSize(uint32_t pktHdr);
uint32_t vrtGetAssociatedContextPktCount(uint32_t pktTrlr);

uint32_t vrtPackDataHeader(uint8_t packetCount, uint16_t packetSize);
uint32_t vrtPackContextHeader(uint8_t packetCount, uint16_t packetSize);
uint32_t vrtPackDataTrailer(bool isTimeCalibrated, bool isDataValid,
                            bool isExtRefLocked, bool isOverrange,
                            bool isSampleLoss,
                            uint8_t associatedContextPktCount);
uint32_t vrtPackContextIndicatorWord(bool isChange, bool isGPS);

void vrtRmcStringToStruct(char *text, VRTFormattedGPS *cntx);

#endif // SH_VRT_H
