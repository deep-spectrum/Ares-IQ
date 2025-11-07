#include <ares-iq/signal-hound/sm/sh_vrt.h>

#ifdef _WIN32
#define _WINSOCKAPI_ // stops windows.h including winsock.
#endif

#include <chrono>

#pragma warning(disable : 4800)

// Use some static variable at some point
static uint32_t dataPacketCount = 0;

int vrtSwapBytes(uint32_t *srcDst, uint32_t wordCount) {
    return vrtSwapBytes(srcDst, srcDst, wordCount);
}

int vrtSwapBytes(const uint32_t *src, uint32_t *dst, uint32_t wordCount) {
    if (!src || !dst)
        return -1;

    for (uint32_t i = 0; i < wordCount; i++) {
        dst[i] = swapWord(src[i]);
    }

    return 0;
}

VrtTemp vrtConvertFloatToTemp(float temp) { return (VrtTemp)(temp * 64.0f); }

float vrtConvertTempToFloat(VrtTemp temp) { return float(temp) / 64.0f; }

VrtGain vrtConvertFloatToGain(float gain) { return (VrtGain)(gain * 128.0f); }

float vrtConvertGainToFloat(VrtGain gain) { return float(gain) / 128.0f; }

VrtRef vrtConvertFloatToRef(float ref) { return (VrtRef)(ref * 128.0f); }

float vrtConvertRefToFloat(VrtRef ref) { return float(ref) / 128.0f; }

VrtFreq vrtConvertFloatToFreq(double freq) {
    return (VrtFreq)(freq * (0x1 << 20));
}

double vrtConvertFreqToFloat(VrtFreq freq) {
    return double(freq) / (0x1 << 20);
}

int32_t vrtConvertFloatToFixed(float val, int fracBits) {
    return (int32_t)(double(val) * (0x1 << fracBits));
}

int32_t vrtConvertFloatToFixed(double val, int fracBits) {
    return (int32_t)(val * (0x1 << fracBits));
}

float vrtConvertFixedToFloat(int32_t val, int fracBits) {
    return float(val) / (0x1 << fracBits);
}

uint64_t vrtGetTime() {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(
               std::chrono::system_clock::now().time_since_epoch())
        .count();
}

uint32_t vrtGetPacketType(uint32_t pktHdr) {
    return (pktHdr & 0xF0000000) >> 28;
}

uint32_t vrtGetPacketCount(uint32_t pktHdr) {
    return (pktHdr & 0x000F0000) >> 16;
}

uint32_t vrtGetPacketSize(uint32_t pktHdr) { return (pktHdr & 0x0000FFFF); }

uint32_t vrtGetAssociatedContextPktCount(uint32_t pktTrlr) {
    return (pktTrlr & 0x0000003F);
}

uint32_t vrtPackDataHeader(uint8_t packetCount, uint16_t packetSize) {
    uint32_t hdr = 0;

    vrtSetBit(hdr, VRT_DATA_HDR_PKT_TYPE);
    vrtSetBit(hdr, VRT_DATA_HDR_T);
    vrtSetBit(hdr, VRT_HDR_TSI);
    vrtSetBit(hdr, VRT_HDR_TSF);
    vrtSetBit(hdr, (packetCount & 0xF) << 16);
    vrtSetBit(hdr, packetSize);

    return hdr;
}

uint32_t vrtPackContextHeader(uint8_t packetCount, uint16_t packetSize) {
    uint32_t hdr = 0;

    vrtSetBit(hdr, VRT_CNTX_HDR_PKT_TYPE);
    vrtSetBit(hdr, VRT_CNTX_HDR_TSM);
    vrtSetBit(hdr, VRT_HDR_TSI);
    vrtSetBit(hdr, VRT_HDR_TSF);
    vrtSetBit(hdr, (packetCount & 0xF) << 16);
    vrtSetBit(hdr, packetSize);

    return hdr;
}

uint32_t vrtPackDataTrailer(bool isTimeCalibrated, bool isDataValid,
                            bool isExtRefLocked, bool isOverrange,
                            bool isSampleLoss,
                            uint8_t associatedContextPktCount) {
    uint32_t trlr = 0;

    // Enable bits
    vrtSetBit(trlr, VRT_TIME_ENABLE);
    vrtSetBit(trlr, VRT_VALID_DATA_ENABLE);
    vrtSetBit(trlr, VRT_REF_LOCK_ENABLE);
    vrtSetBit(trlr, VRT_OVER_RANGE_ENABLE);
    vrtSetBit(trlr, VRT_SAMPLE_LOSS_ENABLE);

    // Indicator bits
    if (isTimeCalibrated) {
        vrtSetBit(trlr, VRT_TIME_INDICATOR);
    }
    if (isDataValid) {
        vrtSetBit(trlr, VRT_VALID_DATA_INDICATOR);
    }
    if (isExtRefLocked) {
        vrtSetBit(trlr, VRT_REF_LOCK_INDICATOR);
    }
    if (isOverrange) {
        vrtSetBit(trlr, VRT_OVER_RANGE_INDICATOR);
    }
    if (isSampleLoss) {
        vrtSetBit(trlr, VRT_SAMPLE_LOSS_INDICATOR);
    }

    // Associated context indicators
    vrtSetBit(trlr, VRT_DATA_TRAILER_E_BIT);
    vrtSetBit(trlr, (associatedContextPktCount & 0x3F));

    return trlr;
}

uint32_t vrtPackContextIndicatorWord(bool isChange, bool isGPS) {
    uint32_t w = 0;

    if (isChange) {
        vrtSetBit(w, VRT_CNTX_FIELD_CHANGE_BIT);
    }
    vrtSetBit(w, VRT_CNTX_BANDWIDTH_BIT);
    vrtSetBit(w, VRT_CNTX_RF_FREQ_BIT);
    vrtSetBit(w, VRT_CNTX_REFERENCE_LEVEL_BIT);
    vrtSetBit(w, VRT_CNTX_GAIN_BIT);
    vrtSetBit(w, VRT_CNTX_SAMPLE_RATE_BIT);
    vrtSetBit(w, VRT_CNTX_TEMPERATURE_BIT);
    vrtSetBit(w, VRT_CNTX_DEVICE_ID_BIT);
    if (isGPS) {
        vrtSetBit(w, VRT_CNTX_FORMATTED_GPS_BIT);
    }

    return w;
}

#include <sstream>
#include <string>

typedef std::vector<std::string> string_list;

void commaSeparate(char *text, string_list &stringList) {
    stringList.clear();

    std::stringstream ss(text);
    std::string str;

    while (ss >> str) {
        stringList.push_back(str);
        if (ss.peek() == ',') {
            ss.ignore();
        }
    }
}

double latitudeStringToDecimal(std::string &pos, std::string &dir) {
    char deg[2] = {pos[0], pos[1]};

    double decimal = atof(deg) + (atof(pos.c_str() + 2) / 60.0);

    if (dir == "S") {
        decimal *= -1.0;
    }

    return decimal;
}

double longitudeStringToDecimal(std::string &pos, std::string &dir) {
    char deg[3] = {pos[0], pos[1], pos[2]};

    double decimal = atof(deg) + (atof(pos.c_str() + 3) / 60.0);

    if (dir == "W") {
        decimal *= -1.0;
    }

    return decimal;
}

int32_t dateTimeToEpochSeconds(std::string &dateStr, std::string &timeStr) {
    tm ti;

    char day[2] = {dateStr[0], dateStr[1]};
    char month[2] = {dateStr[2], dateStr[3]};
    char year[2] = {dateStr[4], dateStr[5]};
    char hour[2] = {timeStr[0], timeStr[1]};
    char min[2] = {timeStr[2], timeStr[3]};
    char sec[2] = {timeStr[4], timeStr[5]};

    ti.tm_year = atoi(year) + 100;
    ti.tm_mon = atoi(month);
    ti.tm_mday = atoi(day);
    ti.tm_hour = atoi(hour);
    ti.tm_min = atoi(min);
    ti.tm_sec = atoi(sec);

    return (int32_t)mktime(&ti);
}

// String into this function is the full RMC string
//   starting at the $GPRMC text (including it)
void vrtRmcStringToStruct(char *text, VRTFormattedGPS *cntx) {
    int fix = atoi(text + 7);

    string_list sl;
    commaSeparate(text, sl);

    // Index 0: $GPRMC

    // 1: Time
    // 9: Date
    cntx->timestampInt = dateTimeToEpochSeconds(sl[9], sl[1]);
    cntx->timestampFracUpper = 0;
    cntx->timestampFracLower = 0;

    // 2: Status

    // 3: Lat
    // 4: Lat dir 'N'
    cntx->latitude =
        vrtConvertFloatToFixed(latitudeStringToDecimal(sl[3], sl[4]), 22);

    // 5: Long
    // 6: Long dir 'E'
    cntx->longitude =
        vrtConvertFloatToFixed(longitudeStringToDecimal(sl[5], sl[6]), 22);

    // 7: Speed over ground
    cntx->speedOverGround = vrtConvertFloatToFixed(atof(sl[7].c_str()), 16);

    // 8: Track angle
    cntx->trackAngle = vrtConvertFloatToFixed(atof(sl[8].c_str()), 22);

    // 10: Magnetic variation
    // 11: Magnetic variation, direction 'W'
    double magVar = atof(sl[10].c_str());
    if (sl[11] == "W") {
        magVar *= -1.0;
    }
    cntx->magneticVariation = vrtConvertFloatToFixed(magVar, 22);
}
