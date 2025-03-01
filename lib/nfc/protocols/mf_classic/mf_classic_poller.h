#pragma once

#include "mf_classic.h"
#include <lib/nfc/protocols/iso14443_3a/iso14443_3a_poller.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief MfClassicPoller opaque type definition.
 */
typedef struct MfClassicPoller MfClassicPoller;

/**
 * @brief Enumeration of possible MfClassic poller event types.
 */
typedef enum {
    MfClassicPollerEventTypeRequestMode, /**< Poller requests to fill the mode. */

    MfClassicPollerEventTypeRequestReadSector, /**< Poller requests data to read sector. */

    MfClassicPollerEventTypeRequestSectorTrailer, /**< Poller requests sector trailer for writing block. */
    MfClassicPollerEventTypeRequestWriteBlock, /**< Poller requests data to write block. */

    MfClassicPollerEventTypeRequestKey, /**< Poller requests key for sector authentication. */
    MfClassicPollerEventTypeNextSector, /**< Poller switches to next sector during dictionary attack. */
    MfClassicPollerEventTypeDataUpdate, /**< Poller updates data. */
    MfClassicPollerEventTypeFoundKeyA, /**< Poller found key A. */
    MfClassicPollerEventTypeFoundKeyB, /**< Poller found key B. */
    MfClassicPollerEventTypeKeyAttackStart, /**< Poller starts key attack. */
    MfClassicPollerEventTypeKeyAttackStop, /**< Poller stops key attack. */
    MfClassicPollerEventTypeKeyAttackNextSector, /**< Poller switches to next sector during key attack. */

    MfClassicPollerEventTypeCardDetected, /**< Poller detected card. */
    MfClassicPollerEventTypeCardLost, /**< Poller lost card. */
    MfClassicPollerEventTypeSuccess, /**< Poller succeeded. */
    MfClassicPollerEventTypeFail, /**< Poller failed. */
} MfClassicPollerEventType;

/**
 * @brief MfClassic poller mode.
 */
typedef enum {
    MfClassicPollerModeRead, /**< Poller reading mode. */
    MfClassicPollerModeWrite, /**< Poller writing mode. */
    MfClassicPollerModeDictAttackStandard, /**< Poller dictionary attack mode. */
    MfClassicPollerModeDictAttackEnhanced, /**< Poller enhanced dictionary attack mode. */
} MfClassicPollerMode;

/**
 * @brief MfClassic poller nested attack phase.
 */
typedef enum {
    MfClassicNestedPhaseNone, /**< No nested attack has taken place yet. */
    MfClassicNestedPhaseAnalyzePRNG, /**< Analyze nonces produced by the PRNG to determine if they fit a weak PRNG */
    MfClassicNestedPhaseDictAttack, /**< Search keys which match the expected PRNG properties and parity for collected nonces */
    MfClassicNestedPhaseDictAttackVerify, /**< Verify candidate keys by authenticating to the sector with the key */
    MfClassicNestedPhaseDictAttackResume, /**< Resume nested dictionary attack from the last tested (invalid) key */
    MfClassicNestedPhaseCalibrate, /**< Perform necessary calculations to recover the plaintext nonce during later collection phase (weak PRNG tags only) */
    MfClassicNestedPhaseRecalibrate, /**< Collect the next plaintext static encrypted nonce for backdoor static encrypted nonce nested attack */
    MfClassicNestedPhaseCollectNtEnc, /**< Log nonces collected during nested authentication for key recovery */
    MfClassicNestedPhaseFinished, /**< Nested attack has finished */
} MfClassicNestedPhase;

/**
 * @brief MfClassic pseudorandom number generator (PRNG) type.
 */
typedef enum {
    MfClassicPrngTypeUnknown, // Tag not yet tested
    MfClassicPrngTypeNoTag, // No tag detected during test
    MfClassicPrngTypeWeak, // Weak PRNG, standard Nested
    MfClassicPrngTypeHard, // Hard PRNG, Hardnested
} MfClassicPrngType;

/**
 * @brief MfClassic authentication backdoor type.
 */
typedef enum {
    MfClassicBackdoorUnknown, // Tag not yet tested
    MfClassicBackdoorNone, // No observed backdoor
    MfClassicBackdoorAuth1, // Tag responds to v1 auth backdoor
    MfClassicBackdoorAuth2, // Tag responds to v2 auth backdoor (sometimes static encrypted)
    MfClassicBackdoorAuth3, // Tag responds to v3 auth backdoor (static encrypted nonce)
} MfClassicBackdoor;

/**
 * @brief MfClassic poller request mode event data.
 *
 * This instance of this structure must be filled on MfClassicPollerEventTypeRequestMode event.
 */
typedef struct {
    MfClassicPollerMode mode; /**< Mode to be used by poller. */
    const MfClassicData* data; /**< Data to be used by poller. */
} MfClassicPollerEventDataRequestMode;

/**
 * @brief MfClassic poller next sector event data.
 *
 * The instance of this structure is filled by poller and passed with
 * MfClassicPollerEventTypeNextSector event.
 */
typedef struct {
    uint8_t current_sector; /**< Current sector number. */
} MfClassicPollerEventDataDictAttackNextSector;

/**
 * @brief MfClassic poller update event data.
 *
 * The instance of this structure is filled by poller and passed with
 * MfClassicPollerEventTypeDataUpdate event.
 */
typedef struct {
    uint8_t sectors_read; /**< Number of sectors read. */
    uint8_t keys_found; /**< Number of keys found. */
    uint8_t current_sector; /**< Current sector number. */
    MfClassicNestedPhase nested_phase; /**< Nested attack phase. */
    MfClassicPrngType prng_type; /**< PRNG (weak or hard). */
    MfClassicBackdoor backdoor; /**< Backdoor type. */
    uint16_t nested_target_key; /**< Target key for nested attack. */
    uint16_t
        msb_count; /**< Number of unique most significant bytes seen during Hardnested attack. */
} MfClassicPollerEventDataUpdate;

/**
 * @brief MfClassic poller key request event data.
 *
 * The instance of this structure must be filled on MfClassicPollerEventTypeRequestKey event.
 */
typedef struct {
    MfClassicKey key; /**< Key to be used by poller. */
    bool key_provided; /**< Flag indicating if key is provided. */
} MfClassicPollerEventDataKeyRequest;

/**
 * @brief MfClassic poller read sector request event data.
 *
 * The instance of this structure must be filled on MfClassicPollerEventTypeRequestReadSector event.
 */
typedef struct {
    uint8_t sector_num; /**< Sector number to be read. */
    MfClassicKey key; /**< Key to be used by poller. */
    MfClassicKeyType key_type; /**< Key type to be used by poller. */
    bool key_provided; /**< Flag indicating if key is provided. */
} MfClassicPollerEventDataReadSectorRequest;

/**
 * @brief MfClassic poller sector trailer request event data.
 *
 * The instance of this structure must be filled on MfClassicPollerEventTypeRequestSectorTrailer event.
 */
typedef struct {
    uint8_t sector_num; /**< Sector number to be read. */
    MfClassicBlock sector_trailer; /**< Sector trailer to be used by poller. */
    bool sector_trailer_provided; /**< Flag indicating if sector trailer is provided. */
} MfClassicPollerEventDataSectorTrailerRequest;

/**
 * @brief MfClassic poller write block request event data.
 *
 * The instance of this structure must be filled on MfClassicPollerEventTypeRequestWriteBlock event.
 */
typedef struct {
    uint8_t block_num; /**< Block number to be written. */
    MfClassicBlock write_block; /**< Block to be written. */
    bool write_block_provided; /**< Flag indicating if block is provided. */
} MfClassicPollerEventDataWriteBlockRequest;

/**
 * @brief MfClassic poller key attack event data.
 *
 * The instance of this structure is filled by poller and passed with
 * MfClassicPollerEventTypeKeyAttackNextSector event.
 */
typedef struct {
    uint8_t current_sector; /**< Current sector number. */
} MfClassicPollerEventKeyAttackData;

/**
 * @brief MfClassic poller event data.
 */
typedef union {
    MfClassicError error; /**< Error code on MfClassicPollerEventTypeFail event. */
    MfClassicPollerEventDataRequestMode poller_mode; /**< Poller mode context. */
    MfClassicPollerEventDataDictAttackNextSector next_sector_data; /**< Next sector context. */
    MfClassicPollerEventDataKeyRequest key_request_data; /**< Key request context. */
    MfClassicPollerEventDataUpdate data_update; /**< Data update context. */
    MfClassicPollerEventDataReadSectorRequest
        read_sector_request_data; /**< Read sector request context. */
    MfClassicPollerEventKeyAttackData key_attack_data; /**< Key attack context. */
    MfClassicPollerEventDataSectorTrailerRequest sec_tr_data; /**< Sector trailer request context. */
    MfClassicPollerEventDataWriteBlockRequest write_block_data; /**< Write block request context. */
} MfClassicPollerEventData;

/**
 * @brief MfClassic poller event.
 *
 * Upon emission of an event, an instance of this struct will be passed to the callback.
 */
typedef struct {
    MfClassicPollerEventType type; /**< Event type. */
    MfClassicPollerEventData* data; /**< Pointer to event specific data. */
} MfClassicPollerEvent;

/**
 * @brief Collect tag nonce during authentication.
 *
 * Must ONLY be used inside the callback function.
 *
 * Starts authentication procedure and collects tag nonce.
 *
 * @param[in, out] instance pointer to the instance to be used in the transaction.
 * @param[in] block_num block number for authentication.
 * @param[in] key_type key type to be used for authentication.
 * @param[out] nt pointer to the MfClassicNt structure to be filled with nonce data.
 * @param[in] backdoor_auth flag indicating if backdoor authentication is used.
 * @return MfClassicErrorNone on success, an error code on failure.
 */
MfClassicError mf_classic_poller_get_nt(
    MfClassicPoller* instance,
    uint8_t block_num,
    MfClassicKeyType key_type,
    MfClassicNt* nt,
    bool backdoor_auth);

/**
 * @brief Collect tag nonce during nested authentication.
 *
 * Must ONLY be used inside the callback function.
 *
 * Starts nested authentication procedure and collects tag nonce.
 *
 * @param[in, out] instance pointer to the instance to be used in the transaction.
 * @param[in] block_num block number for authentication.
 * @param[in] key_type key type to be used for authentication.
 * @param[out] nt pointer to the MfClassicNt structure to be filled with nonce data.
 * @param[in] backdoor_auth flag indicating if backdoor authentication is used.
 * @return MfClassicErrorNone on success, an error code on failure.
 */
MfClassicError mf_classic_poller_get_nt_nested(
    MfClassicPoller* instance,
    uint8_t block_num,
    MfClassicKeyType key_type,
    MfClassicNt* nt,
    bool backdoor_auth);

/**
 * @brief Perform authentication.
 *
 * Must ONLY be used inside the callback function.
 *
 * Perform authentication as specified in Mf Classic protocol. Initialize crypto state for futher
 * communication with the tag.
 *
 * @param[in, out] instance pointer to the instance to be used in the transaction.
 * @param[in] block_num block number for authentication.
 * @param[in] key key to be used for authentication.
 * @param[in] key_type key type to be used for authentication.
 * @param[out] data pointer to MfClassicAuthContext structure to be filled with authentication data.
 * @param[in] backdoor_auth flag indicating if backdoor authentication is used.
 * @return MfClassicErrorNone on success, an error code on failure.
 */
MfClassicError mf_classic_poller_auth(
    MfClassicPoller* instance,
    uint8_t block_num,
    MfClassicKey* key,
    MfClassicKeyType key_type,
    MfClassicAuthContext* data,
    bool backdoor_auth);

/**
 * @brief Perform nested authentication.
 *
 * Must ONLY be used inside the callback function.
 *
 * Perform nested authentication as specified in Mf Classic protocol.
 *
 * @param[in, out] instance pointer to the instance to be used in the transaction.
 * @param[in] block_num block number for authentication.
 * @param[in] key key to be used for authentication.
 * @param[in] key_type key type to be used for authentication.
 * @param[out] data pointer to MfClassicAuthContext structure to be filled with authentication data.
 * @param[in] backdoor_auth flag indicating if backdoor authentication is used.
 * @param[in] early_ret return immediately after receiving encrypted nonce.
 * @return MfClassicErrorNone on success, an error code on failure.
 */
MfClassicError mf_classic_poller_auth_nested(
    MfClassicPoller* instance,
    uint8_t block_num,
    MfClassicKey* key,
    MfClassicKeyType key_type,
    MfClassicAuthContext* data,
    bool backdoor_auth,
    bool early_ret);

/**
 * @brief Halt the tag.
 *
 * Must ONLY be used inside the callback function.
 *
 * Halt the tag and reset crypto state of the poller.
 *
 * @param[in, out] instance pointer to the instance to be used in the transaction.
 * @return MfClassicErrorNone on success, an error code on failure.
 */
MfClassicError mf_classic_poller_halt(MfClassicPoller* instance);

/**
 * @brief Read block from tag.
 *
 * Must ONLY be used inside the callback function.
 *
 * @param[in, out] instance pointer to the instance to be used in the transaction.
 * @param[in] block_num block number to be read.
 * @param[out] data pointer to the MfClassicBlock structure to be filled with block data.
 * @return MfClassicErrorNone on success, an error code on failure.
 */
MfClassicError mf_classic_poller_read_block(
    MfClassicPoller* instance,
    uint8_t block_num,
    MfClassicBlock* data);

/**
 * @brief Write block to tag.
 *
 * Must ONLY be used inside the callback function.
 *
 * @param[in, out] instance pointer to the instance to be used in the transaction.
 * @param[in] block_num block number to be written.
 * @param[in] data pointer to the MfClassicBlock structure to be written.
 * @return MfClassicErrorNone on success, an error code on failure.
 */
MfClassicError mf_classic_poller_write_block(
    MfClassicPoller* instance,
    uint8_t block_num,
    MfClassicBlock* data);

/**
 * @brief Perform value command on tag.
 *
 * Must ONLY be used inside the callback function.
 *
 * Perform Increment, Decrement or Restore command on tag. The result is stored in internal transfer
 * block of the tag. Use mf_classic_poller_value_transfer to transfer the result to the tag.
 *
 * @param[in, out] instance pointer to the instance to be used in the transaction.
 * @param[in] block_num block number to be used for value command.
 * @param[in] cmd value command to be performed.
 * @param[in] data value to be used for value command.
 * @return MfClassicErrorNone on success, an error code on failure.
 */
MfClassicError mf_classic_poller_value_cmd(
    MfClassicPoller* instance,
    uint8_t block_num,
    MfClassicValueCommand cmd,
    int32_t data);

/**
 * @brief Transfer internal transfer block to tag.
 *
 * Must ONLY be used inside the callback function.
 *
 * Transfer internal transfer block to tag. The block is filled by mf_classic_poller_value_cmd.
 *
 * @param[in, out] instance pointer to the instance to be used in the transaction.
 * @param[in] block_num block number to be used for value command.
 * @return MfClassicErrorNone on success, an error code on failure.
 */
MfClassicError mf_classic_poller_value_transfer(MfClassicPoller* instance, uint8_t block_num);

/**
 * @brief Transmit and receive Iso14443_3a standard frames in poller mode.
 *
 * Must ONLY be used inside the callback function.
 *
 * The rx_buffer will be filled with any data received as a response to data
 * sent from tx_buffer, with a timeout defined by the fwt parameter.
 *
 * @param[in, out] instance pointer to the instance to be used in the transaction.
 * @param[in] tx_buffer pointer to the buffer containing the data to be transmitted.
 * @param[out] rx_buffer pointer to the buffer to be filled with received data.
 * @param[in] fwt frame wait time (response timeout), in carrier cycles.
 * @return MfClassicErrorNone on success, an error code on failure.
 */
MfClassicError mf_classic_poller_send_standard_frame(
    MfClassicPoller* instance,
    const BitBuffer* tx_buffer,
    BitBuffer* rx_buffer,
    uint32_t fwt_fc);

/**
 * @brief Transmit and receive Iso14443_3a frames in poller mode.
 *
 * Must ONLY be used inside the callback function.
 *
 * The rx_buffer will be filled with any data received as a response to data
 * sent from tx_buffer, with a timeout defined by the fwt parameter.
 *
 * @param[in, out] instance pointer to the instance to be used in the transaction.
 * @param[in] tx_buffer pointer to the buffer containing the data to be transmitted.
 * @param[out] rx_buffer pointer to the buffer to be filled with received data.
 * @param[in] fwt frame wait time (response timeout), in carrier cycles.
 * @return MfClassicErrorNone on success, an error code on failure.
 */
MfClassicError mf_classic_poller_send_frame(
    MfClassicPoller* instance,
    const BitBuffer* tx_buffer,
    BitBuffer* rx_buffer,
    uint32_t fwt_fc);

/**
 * @brief Transmit and receive Iso14443_3a frames with custom parity bits in poller mode.
 *
 * Must ONLY be used inside the callback function.
 *
 * The rx_buffer will be filled with any data received as a response to data
 * sent from tx_buffer, with a timeout defined by the fwt parameter.
 *
 * Custom parity bits must be set in the tx_buffer. The rx_buffer will contain
 * the received data with the parity bits.
 *
 * @param[in, out] instance pointer to the instance to be used in the transaction.
 * @param[in] tx_buffer pointer to the buffer containing the data to be transmitted.
 * @param[out] rx_buffer pointer to the buffer to be filled with received data.
 * @param[in] fwt frame wait time (response timeout), in carrier cycles.
 * @return MfClassicErrorNone on success, an error code on failure.
 */
MfClassicError mf_classic_poller_send_custom_parity_frame(
    MfClassicPoller* instance,
    const BitBuffer* tx_buffer,
    BitBuffer* rx_buffer,
    uint32_t fwt_fc);

/**
 * @brief Transmit and receive Mifare Classic encrypted frames with custom parity bits in poller mode.
 *
 * Must ONLY be used inside the callback function.
 *
 * The rx_buffer will be filled with any data received as a response to data
 * sent from tx_buffer, with a timeout defined by the fwt parameter.
 *
 * @param[in, out] instance pointer to the instance to be used in the transaction.
 * @param[in] tx_buffer pointer to the buffer containing the plain data to be transmitted.
 * @param[out] rx_buffer pointer to the buffer to be filled with decyphered received data.
 * @param[in] fwt frame wait time (response timeout), in carrier cycles.
 * @return MfClassicErrorNone on success, an error code on failure.
 */
MfClassicError mf_classic_poller_send_encrypted_frame(
    MfClassicPoller* instance,
    const BitBuffer* tx_buffer,
    BitBuffer* rx_buffer,
    uint32_t fwt_fc);

#ifdef __cplusplus
}
#endif
