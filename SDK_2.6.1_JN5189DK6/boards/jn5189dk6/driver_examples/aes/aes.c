/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/

#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "board.h"

#include "fsl_aes.h"

#include <string.h>

#include "pin_mux.h"
#include <stdbool.h>
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define EXAMPLE_AES AES0
#define DEBUG_USART USART0

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*! @brief 16 bytes key for CBC method: "ultrapassword123". */
static const uint8_t s_CbcKey128[] = {0x75, 0x6c, 0x74, 0x72, 0x61, 0x70, 0x61, 0x73,
                                      0x73, 0x77, 0x6f, 0x72, 0x64, 0x31, 0x32, 0x33};

/*! @brief 24 bytes key for CBC method: "UltraMegaSecretPassword1". */
static const uint8_t s_CbcKey192[] = {0x55, 0x6c, 0x74, 0x72, 0x61, 0x4d, 0x65, 0x67, 0x61, 0x53, 0x65, 0x63,
                                      0x72, 0x65, 0x74, 0x50, 0x61, 0x73, 0x73, 0x77, 0x6f, 0x72, 0x64, 0x31};

/*! @brief 32 bytes key for CBC method: "Thispasswordisveryuncommonforher". */
static const uint8_t s_CbcKey256[] = {0x54, 0x68, 0x69, 0x73, 0x70, 0x61, 0x73, 0x73, 0x77, 0x6f, 0x72,
                                      0x64, 0x69, 0x73, 0x76, 0x65, 0x72, 0x79, 0x75, 0x6e, 0x63, 0x6f,
                                      0x6d, 0x6d, 0x6f, 0x6e, 0x66, 0x6f, 0x72, 0x68, 0x65, 0x72};

/*!
 * @brief Plaintext for CBC method.
 * 16-byte multiple, last '\0' is not used.
 */
static const uint8_t s_CbcPlain[] =
    "Be that word our sign of parting, bird or fiend! I shrieked upstarting"
    "Get thee back into the tempest and the Nights Plutonian shore!"
    "Leave no black plume as a token of that lie thy soul hath spoken!"
    "Leave my loneliness unbroken! quit the bust above my door!"
    "Take thy beak from out my heart, and take thy form from off my door!"
    "Quoth the raven, Nevermore.  ";

/*! @brief Decrypted plaintext from CBC method goes here. */
static uint8_t s_CbcPlainDecrypted[sizeof(s_CbcPlain) - 1U];

/*! @brief Encrypted ciphertext from CBC method goes here. */
static uint8_t s_CbcCipher[sizeof(s_CbcPlain) - 1U];

/*! @brief Expected ciphertext from CBC method using s_CbcKey128 key. */
static const uint8_t s_Cbc128CipherExpected[] = {
    0xeb, 0x69, 0xb5, 0xae, 0x7a, 0xbb, 0xb8, 0xee, 0x4d, 0xe5, 0x28, 0x97, 0xca, 0xab, 0x60, 0x65, 0x63, 0xf9, 0xe8,
    0x4c, 0x7f, 0xda, 0x0a, 0x02, 0x3a, 0x93, 0x16, 0x0d, 0x64, 0x56, 0x5a, 0x86, 0xf2, 0xe8, 0x5b, 0x38, 0x1d, 0x31,
    0xd7, 0x65, 0x7e, 0x8f, 0x8d, 0x53, 0xc5, 0xa6, 0x0c, 0x5d, 0xc5, 0x43, 0x98, 0x3b, 0x49, 0x3a, 0xce, 0x7d, 0xf9,
    0xb5, 0xf7, 0x95, 0x47, 0x89, 0xaf, 0xd8, 0x2f, 0xbd, 0xa4, 0xd8, 0x7f, 0xb9, 0x13, 0x3a, 0xcd, 0x17, 0xc8, 0xc4,
    0xb0, 0x5d, 0xe8, 0xf5, 0x19, 0x39, 0x6a, 0x14, 0x1b, 0x1b, 0x78, 0x5e, 0xe0, 0xd6, 0x67, 0x9a, 0x36, 0x17, 0x9c,
    0x7a, 0x88, 0x26, 0xfd, 0x8f, 0x3d, 0x82, 0xc9, 0xb1, 0x2a, 0x9c, 0xc0, 0xdd, 0xdb, 0x78, 0x61, 0x3b, 0x22, 0x5d,
    0x48, 0x3c, 0xab, 0x10, 0xd3, 0x5d, 0x0d, 0xa1, 0x25, 0x3e, 0x4d, 0xd6, 0x8e, 0xc4, 0x1b, 0x68, 0xbb, 0xa4, 0x2d,
    0x97, 0x2b, 0xd6, 0x23, 0xa0, 0xf2, 0x90, 0x8e, 0x07, 0x75, 0x44, 0xb3, 0xe2, 0x5a, 0x35, 0x38, 0x4c, 0x5d, 0x35,
    0xa9, 0x7c, 0xa3, 0xb6, 0x38, 0xe7, 0xf5, 0x20, 0xdc, 0x0e, 0x6c, 0x7c, 0x4b, 0x4f, 0x93, 0xc1, 0x81, 0x69, 0x02,
    0xb7, 0x66, 0x37, 0x24, 0x0d, 0xb8, 0x9a, 0xa8, 0xd4, 0x42, 0x75, 0x28, 0xe8, 0x33, 0x89, 0x1e, 0x60, 0x82, 0xe9,
    0xf6, 0x45, 0x72, 0x64, 0x65, 0xd2, 0xcd, 0x19, 0xd9, 0x5e, 0xa2, 0x59, 0x31, 0x82, 0x53, 0x20, 0x35, 0x13, 0x76,
    0x7f, 0xeb, 0xc3, 0xbe, 0xfa, 0x4a, 0x10, 0x83, 0x81, 0x0f, 0x24, 0x6d, 0xca, 0x53, 0x07, 0xb9, 0xe0, 0xb9, 0x5d,
    0x91, 0x2d, 0x90, 0x86, 0x5b, 0x9d, 0xaa, 0xcd, 0x28, 0xea, 0x11, 0xfb, 0x83, 0x39, 0x9c, 0xf5, 0x3b, 0xd9, 0xef,
    0x38, 0xc7, 0xa4, 0xad, 0x47, 0xf2, 0x2d, 0xd6, 0x6b, 0x26, 0x28, 0x59, 0xaa, 0x33, 0x01, 0x73, 0xc9, 0x46, 0x97,
    0xa3, 0xe5, 0x11, 0x71, 0x66, 0xef, 0x1f, 0x0b, 0xbc, 0xe7, 0x4f, 0x8c, 0x79, 0xe2, 0x39, 0x14, 0x85, 0xcd, 0xa9,
    0x59, 0xed, 0x78, 0x9d, 0x37, 0xf5, 0x46, 0xfc, 0xa9, 0x8a, 0x16, 0x0a, 0x76, 0x58, 0x6d, 0x59, 0x9e, 0x65, 0xbe,
    0x1b, 0xc2, 0x09, 0xa1, 0xf9, 0x40, 0xab, 0xdb, 0x2e, 0x11, 0x30, 0x29, 0x49, 0x75, 0xf7, 0x74, 0xe1, 0xf3, 0x78,
    0x97, 0x69, 0x2c, 0x6a, 0x0e, 0x0d, 0xbd, 0x72, 0x3d, 0x75, 0xd6, 0x0a, 0x8c, 0xc2, 0x92, 0xd9, 0xb6, 0x46, 0x91,
    0xa7, 0xe4, 0x74, 0x71, 0xf5, 0xb4, 0x21, 0x86, 0x18, 0xa8};

/*! @brief Expected ciphertext from CBC method using s_CbcKey192 key. */
static const uint8_t s_Cbc192CipherExpected[] = {
    0xb5, 0xb8, 0xe5, 0x87, 0x40, 0x71, 0xdf, 0x48, 0x17, 0xf1, 0xe0, 0xa4, 0x92, 0xf1, 0xcf, 0x78, 0xb4, 0xb3, 0x92,
    0x42, 0xd6, 0x3b, 0x23, 0x3c, 0xa7, 0x82, 0xcc, 0x6a, 0xa4, 0xf5, 0x52, 0x8e, 0xdf, 0x02, 0x14, 0x2d, 0x1d, 0xae,
    0x3e, 0x86, 0x87, 0x41, 0x8d, 0xe9, 0x5b, 0x12, 0x38, 0x24, 0x7e, 0x46, 0xa7, 0xb1, 0x5f, 0x8a, 0x8f, 0x69, 0xdc,
    0x56, 0x8f, 0x37, 0x80, 0x53, 0xff, 0x67, 0x67, 0x54, 0xa7, 0x79, 0x2b, 0x7b, 0x66, 0x21, 0x78, 0x80, 0x34, 0x02,
    0x18, 0xd7, 0xc0, 0xef, 0x05, 0xdb, 0x25, 0x4d, 0x42, 0x05, 0xbb, 0x69, 0x35, 0x63, 0xc1, 0x31, 0xe3, 0x47, 0xc2,
    0xde, 0x67, 0xfe, 0x9f, 0x60, 0xf6, 0x6c, 0xb5, 0x41, 0x5e, 0x25, 0xa6, 0xec, 0xfe, 0xb0, 0x3e, 0x87, 0x61, 0x8e,
    0x5c, 0x03, 0x8e, 0x8b, 0x20, 0x74, 0xcd, 0x49, 0xa8, 0x04, 0xb0, 0xca, 0x10, 0xaa, 0x27, 0x5d, 0xe7, 0xfe, 0x90,
    0x3e, 0x50, 0xe4, 0x3e, 0x94, 0x68, 0xd1, 0xcc, 0x54, 0x28, 0xba, 0x2d, 0x2a, 0x88, 0x0d, 0xfa, 0xb2, 0x0a, 0x15,
    0x8d, 0x0a, 0xdc, 0xbc, 0x16, 0xd8, 0xaf, 0x1d, 0xce, 0x9a, 0xfa, 0x90, 0x96, 0x62, 0xbd, 0x11, 0x62, 0x09, 0x80,
    0xfe, 0xbd, 0x6d, 0xca, 0xbc, 0x6a, 0x07, 0xf9, 0x5e, 0x63, 0xe2, 0x6d, 0xfe, 0x7d, 0x88, 0xa2, 0xb6, 0x8e, 0xaf,
    0x1a, 0x80, 0x62, 0x19, 0x4c, 0x68, 0xfc, 0x61, 0x18, 0x58, 0x33, 0x76, 0x20, 0x84, 0x5d, 0xd6, 0x49, 0x97, 0xb7,
    0x79, 0x83, 0xf0, 0x69, 0x2f, 0xce, 0x73, 0x86, 0x5a, 0x6f, 0xfa, 0x96, 0x66, 0x97, 0xf3, 0xa0, 0xb3, 0xed, 0x67,
    0x36, 0x64, 0x08, 0x28, 0x75, 0xb5, 0x58, 0x19, 0x85, 0x01, 0x28, 0x3e, 0xb1, 0x8e, 0x68, 0x4e, 0x9f, 0x95, 0x86,
    0xae, 0xe0, 0x6e, 0x60, 0xbe, 0xa0, 0xfc, 0x5e, 0x8b, 0x5e, 0xe8, 0x96, 0xe9, 0xfa, 0xcb, 0x3d, 0xce, 0x9d, 0x70,
    0xbe, 0xa2, 0x05, 0x52, 0xbb, 0xa2, 0x79, 0xc9, 0xac, 0xf5, 0x91, 0xa2, 0xe4, 0xda, 0xa4, 0x5f, 0x89, 0x75, 0x45,
    0x7b, 0x58, 0xe3, 0xdb, 0x0f, 0xef, 0xd6, 0xa7, 0x88, 0x9c, 0x0d, 0xf3, 0x5b, 0x49, 0xb1, 0x27, 0xe3, 0x81, 0x92,
    0x93, 0x91, 0xaf, 0x27, 0x6b, 0x5a, 0x2e, 0x1a, 0x0c, 0xb6, 0xc5, 0x50, 0xc4, 0xb3, 0xf8, 0xfd, 0x0a, 0xff, 0xc7,
    0x8c, 0x55, 0xde, 0xde, 0x6f, 0x7c, 0xb9, 0xaa, 0x8d, 0x18, 0x17, 0xc5, 0x55, 0x95, 0x59, 0xd0, 0x00, 0x53, 0x63,
    0xaf, 0xe9, 0xf9, 0xde, 0x93, 0xe2, 0xa6, 0x90, 0xe5, 0xa9};

/*! @brief Expected ciphertext from CBC method using s_CbcKey256 key. */
static const uint8_t s_Cbc256CipherExpected[] = {
    0x09, 0x9b, 0xf5, 0xb3, 0xaf, 0x11, 0xa9, 0xd1, 0xa1, 0x81, 0x78, 0x6c, 0x6e, 0x74, 0xf3, 0xb8, 0x70, 0xee, 0x31,
    0x4d, 0x6d, 0x54, 0xab, 0x37, 0xcb, 0xeb, 0x58, 0x6f, 0x09, 0x5f, 0x72, 0xc4, 0x5a, 0xd0, 0x56, 0xc8, 0x3d, 0x93,
    0x45, 0xe2, 0x7e, 0x97, 0xaa, 0xc3, 0xc9, 0xf5, 0xde, 0x74, 0x73, 0x45, 0x35, 0xea, 0x1f, 0x5e, 0x81, 0xbf, 0x9d,
    0xb5, 0xc9, 0x77, 0x77, 0x1c, 0x00, 0xde, 0x67, 0x34, 0xff, 0x62, 0x48, 0x89, 0xd9, 0xbe, 0x92, 0xd4, 0x7e, 0xaf,
    0x9d, 0x8a, 0x65, 0x14, 0x1f, 0x62, 0xaa, 0x0a, 0xe4, 0x37, 0x8e, 0x18, 0x3c, 0x75, 0x5e, 0x38, 0x6a, 0xa9, 0x5d,
    0x26, 0x54, 0x0a, 0xd8, 0xeb, 0x7a, 0x25, 0xa6, 0xd4, 0x18, 0x13, 0x1f, 0x30, 0xfc, 0x37, 0x09, 0x77, 0x90, 0x26,
    0x88, 0x0e, 0x53, 0x67, 0xba, 0xe2, 0xfa, 0x38, 0xb9, 0x74, 0xa9, 0x5b, 0xda, 0x6a, 0xe0, 0xb3, 0x39, 0xed, 0x07,
    0xae, 0xe6, 0x86, 0x44, 0x2d, 0xf2, 0xd8, 0x1f, 0x86, 0x2c, 0xac, 0x01, 0x4c, 0x9b, 0xce, 0x65, 0x6a, 0x8a, 0x3a,
    0xf0, 0xf9, 0xfd, 0x15, 0x65, 0xb6, 0xaf, 0xdc, 0x90, 0xc5, 0x47, 0x96, 0x28, 0xb0, 0x1c, 0x56, 0x2e, 0xc6, 0xdd,
    0x4e, 0x71, 0xd3, 0x73, 0xf5, 0x7c, 0xa6, 0x66, 0x8b, 0x44, 0xaf, 0x53, 0x61, 0x16, 0xe3, 0x41, 0x94, 0xe7, 0x6d,
    0x3d, 0xdb, 0xe1, 0x92, 0x52, 0x39, 0x05, 0x97, 0xf4, 0x41, 0xc8, 0xbe, 0x54, 0xec, 0x9a, 0x52, 0xf1, 0x79, 0x0c,
    0x71, 0x05, 0x14, 0xc8, 0x16, 0x86, 0xdb, 0xa3, 0x8e, 0x1c, 0x41, 0x5b, 0x7a, 0x3b, 0x77, 0xa9, 0x27, 0x7a, 0xde,
    0xcd, 0xaa, 0x86, 0x2e, 0x52, 0x87, 0x54, 0x1c, 0x88, 0x4c, 0xdb, 0x3e, 0xab, 0x48, 0xaa, 0x51, 0x5a, 0xcd, 0xb0,
    0xe7, 0x68, 0x91, 0x33, 0x9e, 0xfd, 0x07, 0x9d, 0xdf, 0x18, 0x51, 0xa5, 0xc0, 0xa6, 0x68, 0xbc, 0xd2, 0x6b, 0x1f,
    0x03, 0xfc, 0xf3, 0x71, 0xed, 0x5b, 0x28, 0x35, 0xa8, 0x56, 0x93, 0x4c, 0xdc, 0x1f, 0xa1, 0x88, 0xe8, 0xbe, 0x08,
    0x48, 0xe8, 0x28, 0x1d, 0x16, 0xb4, 0x1e, 0xeb, 0xca, 0xdd, 0x43, 0x18, 0xfe, 0x49, 0x24, 0xfd, 0x23, 0x83, 0x44,
    0x2b, 0xc3, 0x33, 0x80, 0x62, 0xb9, 0xa6, 0xb8, 0x48, 0x1e, 0x72, 0x52, 0xef, 0xee, 0x56, 0xd8, 0x05, 0x08, 0xad,
    0xc2, 0xe9, 0xb7, 0x46, 0x12, 0xbc, 0xc8, 0x7d, 0xe2, 0x87, 0x9d, 0x57, 0xf7, 0x6f, 0x10, 0x6e, 0x8c, 0x32, 0x8e,
    0x2f, 0x78, 0x20, 0xf0, 0x23, 0x29, 0x54, 0xef, 0x66, 0x8d};

/*! @brief Initialization vector for CBC method: 16 bytes: "mysecretpassword". */
static const uint8_t s_CbcIv[AES_BLOCK_SIZE] = {0x6d, 0x79, 0x73, 0x65, 0x63, 0x72, 0x65, 0x74,
                                                0x70, 0x61, 0x73, 0x73, 0x77, 0x6f, 0x72, 0x64};

/*! @brief 16 bytes key for GCM method. */
static const uint8_t s_GcmKey[16] = {0xfe, 0xff, 0xe9, 0x92, 0x86, 0x65, 0x73, 0x1c,
                                     0x6d, 0x6a, 0x8f, 0x94, 0x67, 0x30, 0x83, 0x08};

/*! @brief Plaintext for GCM method. */
static const uint8_t s_GcmPlain[] = {0xd9, 0x31, 0x32, 0x25, 0xf8, 0x84, 0x06, 0xe5, 0xa5, 0x59, 0x09, 0xc5,
                                     0xaf, 0xf5, 0x26, 0x9a, 0x86, 0xa7, 0xa9, 0x53, 0x15, 0x34, 0xf7, 0xda,
                                     0x2e, 0x4c, 0x30, 0x3d, 0x8a, 0x31, 0x8a, 0x72, 0x1c, 0x3c, 0x0c, 0x95,
                                     0x95, 0x68, 0x09, 0x53, 0x2f, 0xcf, 0x0e, 0x24, 0x49, 0xa6, 0xb5, 0x25,
                                     0xb1, 0x6a, 0xed, 0xf5, 0xaa, 0x0d, 0xe6, 0x57, 0xba, 0x63, 0x7b, 0x39};

/*! @brief Decrypted plaintext from GCM method goes here. */
static uint8_t s_GcmPlainDecrypted[sizeof(s_GcmPlain)];

/*! @brief Expected ciphertext from GCM method. */
static const uint8_t s_GcmCipherExpected[] = {0x42, 0x83, 0x1e, 0xc2, 0x21, 0x77, 0x74, 0x24, 0x4b, 0x72, 0x21, 0xb7,
                                              0x84, 0xd0, 0xd4, 0x9c, 0xe3, 0xaa, 0x21, 0x2f, 0x2c, 0x02, 0xa4, 0xe0,
                                              0x35, 0xc1, 0x7e, 0x23, 0x29, 0xac, 0xa1, 0x2e, 0x21, 0xd5, 0x14, 0xb2,
                                              0x54, 0x66, 0x93, 0x1c, 0x7d, 0x8f, 0x6a, 0x5a, 0xac, 0x84, 0xaa, 0x05,
                                              0x1b, 0xa3, 0x0b, 0x39, 0x6a, 0x0a, 0xac, 0x97, 0x3d, 0x58, 0xe0, 0x91};

/*! @brief Encrypted ciphertext from GCM method goes here. */
static uint8_t s_GcmCipher[sizeof(s_GcmCipherExpected)];

/*! @brief Initialization vector for GCM method. */
static const uint8_t s_GcmIv[12] = {0xca, 0xfe, 0xba, 0xbe, 0xfa, 0xce, 0xdb, 0xad, 0xde, 0xca, 0xf8, 0x88};

/*! @brief Additional authenticated data for GCM method. */
static const uint8_t s_GcmAad[] = {0xfe, 0xed, 0xfa, 0xce, 0xde, 0xad, 0xbe, 0xef, 0xfe, 0xed,
                                   0xfa, 0xce, 0xde, 0xad, 0xbe, 0xef, 0xab, 0xad, 0xda, 0xd2};

/*! @brief Expected tag from GCM method. */
static const uint8_t s_GcmTagExpected[] = {0x5b, 0xc9, 0x4f, 0xbc, 0x32, 0x21, 0xa5, 0xdb,
                                           0x94, 0xfa, 0xe9, 0x5a, 0xe7, 0x12, 0x1a, 0x47};

/*! @brief Encrypted tag from GCM method goes here. */
static uint8_t s_GcmTag[sizeof(s_GcmTagExpected)];

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

static void RunAesCbcExamples(void);

static void EncryptDecryptCbc(const uint8_t *key, size_t keySize, const uint8_t *cipherExpected);

static void RunAesGcmExamples(void);

static void EncryptDecryptGcm(void);

/*******************************************************************************
 * Code
 ******************************************************************************/

/*!
 * @brief Executes examples for AES encryption and decryption in CBC mode.
 */
static void RunAesCbcExamples(void)
{
    EncryptDecryptCbc(s_CbcKey128, sizeof(s_CbcKey128), s_Cbc128CipherExpected);
    EncryptDecryptCbc(s_CbcKey192, sizeof(s_CbcKey192), s_Cbc192CipherExpected);
    EncryptDecryptCbc(s_CbcKey256, sizeof(s_CbcKey256), s_Cbc256CipherExpected);
}

/*!
 * @brief Encrypts and decrypts in AES CBC mode.
 *
 * @param key encryption key
 * @param keySize size of key in bytes
 * @param cipherExpected expected output of encryption
 */
static void EncryptDecryptCbc(const uint8_t *key, size_t keySize, const uint8_t *cipherExpected)
{
    status_t status;

    PRINTF("AES CBC: encrypting using %u bit key ", 8U * keySize);

    status = AES_SetKey(EXAMPLE_AES, key, keySize);
    if (status != kStatus_Success)
    {
        PRINTF("- failed to set key!\r\n\r\n");
        return;
    }

    status = AES_EncryptCbc(EXAMPLE_AES, s_CbcPlain, s_CbcCipher, sizeof(s_CbcCipher), s_CbcIv);
    if (status != kStatus_Success)
    {
        PRINTF("- failed to encrypt!\r\n\r\n");
        return;
    }

    if (memcmp(s_CbcCipher, cipherExpected, sizeof(s_CbcCipher)) == 0)
    {
        PRINTF("done successfully.\r\n");
    }
    else
    {
        PRINTF("- encrypted text mismatch!\r\n\r\n");
        return;
    }

    PRINTF("AES CBC: decrypting back ");

    status = AES_DecryptCbc(EXAMPLE_AES, s_CbcCipher, s_CbcPlainDecrypted, sizeof(s_CbcCipher), s_CbcIv);
    if (status != kStatus_Success)
    {
        PRINTF("- failed to decrypt!\r\n\r\n");
        return;
    }

    if (memcmp(s_CbcPlainDecrypted, s_CbcPlain, sizeof(s_CbcPlainDecrypted)) == 0)
    {
        PRINTF("done successfully.\r\n\r\n");
    }
    else
    {
        PRINTF("- decrypted text mismatch!\r\n\r\n");
    }
}

/*!
 * @brief Executes examples for AES encryption and decryption in GCM mode.
 */
static void RunAesGcmExamples(void)
{
    EncryptDecryptGcm();
}

/*!
 * @brief Encrypts and decrypts in AES GCM mode.
 */
static void EncryptDecryptGcm(void)
{
    status_t status;

    PRINTF("AES GCM: encrypt ");

    status = AES_SetKey(EXAMPLE_AES, s_GcmKey, sizeof(s_GcmKey));
    if (status != kStatus_Success)
    {
        PRINTF("- failed to set key!\r\n\r\n");
        return;
    }

    status = AES_EncryptTagGcm(EXAMPLE_AES, s_GcmPlain, s_GcmCipher, sizeof(s_GcmPlain), s_GcmIv, sizeof(s_GcmIv),
                               s_GcmAad, sizeof(s_GcmAad), s_GcmTag, sizeof(s_GcmTag));
    if (status != kStatus_Success)
    {
        PRINTF("- failed to encrypt!\r\n\r\n");
        return;
    }

    if (memcmp(s_GcmTag, s_GcmTagExpected, sizeof(s_GcmTagExpected)) != 0)
    {
        PRINTF("- tag mismatch!\r\n\r\n");
        return;
    }

    if (memcmp(s_GcmCipher, s_GcmCipherExpected, sizeof(s_GcmCipherExpected)) != 0)
    {
        PRINTF("- encrypted text mismatch!\r\n\r\n");
        return;
    }

    PRINTF("done successfully.\r\n");

    PRINTF("AES GCM: decrypt ");

    status = AES_DecryptTagGcm(EXAMPLE_AES, s_GcmCipher, s_GcmPlainDecrypted, sizeof(s_GcmCipher), s_GcmIv,
                               sizeof(s_GcmIv), s_GcmAad, sizeof(s_GcmAad), s_GcmTag, sizeof(s_GcmTag));
    if (status != kStatus_Success)
    {
        PRINTF("- failed to decrypt!\r\n\r\n");
        return;
    }

    if (memcmp(s_GcmPlainDecrypted, s_GcmPlain, sizeof(s_GcmPlain)) != 0)
    {
        PRINTF("- decrypted text mismatch!\r\n\r\n");
        return;
    }

    PRINTF("done successfully.\r\n\r\n");
}

/*!
 * @brief Main function.
 */
int main(void)
{
    /* Init hardware */
    /* Security code to allow debug access */
    SYSCON->CODESECURITYPROT = 0x87654320;

    /* attach clock for USART(debug console) */
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    /* reset FLEXCOMM for USART */
    RESET_PeripheralReset(kFC0_RST_SHIFT_RSTn);

    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();
    BOARD_InitPins();

    /* Ungate clock to AES engine and reset it */
    CLOCK_EnableClock(kCLOCK_Aes);
    RESET_PeripheralReset(kAES_RST_SHIFT_RSTn);

    PRINTF("AES Peripheral Driver Example\r\n\r\n");

    /* Example of AES CBC */
    RunAesCbcExamples();

    /* Example of AES GCM */
    RunAesGcmExamples();

    while (1)
    {
    }
}
