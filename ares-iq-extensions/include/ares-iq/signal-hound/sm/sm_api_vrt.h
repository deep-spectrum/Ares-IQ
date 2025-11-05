// Copyright (c).2022, Signal Hound, Inc.
// For licensing information, please see the API license in the software_licenses folder

/**
 * \file sm_api_vrt.h
 * \brief VITA 49 interface.
 *
 * These functions are used to stream I/Q data in the VRT data format.
 *
 */

#ifndef SM_API_VRT_H
#define SM_API_VRT_H

#include <ares-iq/signal-hound/sm/sm_api.h>

/**
 * Set the stream identifier, which is used to identify each VRT packet with
 * the device.
 *
 * @param[in] device Device handle.
 *
 * @param[in] sid New stream ID.
 *
 * @return
 */
SM_API SmStatus smSetVrtStreamID(int device, uint32_t sid);

/**
 * Retrieve the number of words in a VRT context packet. Use this to allocate
 * an appropriately sized buffer for #smGetVrtContextPkt.
 *
 * @param[in] device Device handle.
 *
 * @param[out] wordCount Returns the number of words in a VRT context packet.
 *
 * @return
 */
SM_API SmStatus smGetVrtContextPktSize(int device, uint32_t *wordCount);

/**
 * Retrieve one VRT context packet.
 *
 * @param[in] device Device handle.
 *
 * @param[out] words User allocated buffer. Should be length wordCount number
 * of words long, where wordCount was returned from #smGetVrtContextPktSize.
 *
 * @param[out] wordCount Number of words written to the words buffer.
 *
 * @return
 */
SM_API SmStatus smGetVrtContextPkt(int device, uint32_t *words, uint32_t *wordCount);

/**
 * This function specifies the number of I/Q samples in each VRT data packet.
 *
 * @param[in] device Device handle.
 *
 * @param[in] samplesPerPkt The number of I/Q samples.
 *
 * @return
 */
SM_API SmStatus smSetVrtPacketSize(int device, uint16_t samplesPerPkt);

/**
 * Retrieve the number of words in a VRT data packet. Use this and a
 * user-specified packet count to allocate an appropriately sized buffer for
 * #smGetVrtPackets.
 *
 * @param[in] device Device handle.
 *
 * @param[out] samplesPerPkt Returns the number of I/Q samples in a VRT data
 * packet.
 *
 * @param[out] wordCount Returns the number of words in a VRT data packet.
 *
 * @return
 */
SM_API SmStatus smGetVrtPacketSize(int device, uint16_t *samplesPerPkt, uint32_t *wordCount);

/**
 * Retrieve one block of one or more VRT data packets. This function blocks
 * until the data requested is available.
 *
 * @param[in] device Device handle.
 *
 * @param[out] words Pointer to user allocated buffer. Returns the VRT packets.
 * Must be as large as packetCount * packetSize words.
 *
 * @param[out] wordCount The number of words written to the words buffer.
 *
 * @param[in] packetCount The number of VRT data packets to retrive.
 *
 * @param[in] purgeBeforeAcquire When set to smTrue, any buffered I/Q data in
 * the API is purged before beginning the I/Q block acquisition. See the
 * section on Streaming I/Q Data for more detailed information.
 *
 * @return
 */
SM_API SmStatus smGetVrtPackets(int device,
                                uint32_t *words,
                                uint32_t *wordCount,
                                uint32_t packetCount,
                                SmBool purgeBeforeAcquire);

#endif
