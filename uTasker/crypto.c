/**********************************************************************
    Mark Butcher    Bsc (Hons) MPhil MIET

    M.J.Butcher Consulting
    Birchstrasse 20f,    CH-5406, R�tihof
    Switzerland

    www.uTasker.com    Skype: M_J_Butcher

    ---------------------------------------------------------------------
    File:      crypto.c
    Project:   Single Chip Embedded Internet
    ---------------------------------------------------------------------
    Copyright (C) M.J.Butcher Consulting 2004..2020
    *********************************************************************
    18.10.2018 Add SHA256 with mmCAU support (for 1 block)
    23.04.2019 Allow LTC based SHA256 to be used without AES enabled
    22.03.2020 Add aes_mbedTLS_var_init()
    25.07.2020 Add AES-128-CTR option for i.MX RT on-fly decryption checking
    
*/        

#include "config.h"
#if defined CRYPTOGRAPHY && defined CRYPTO_AES                           // AES support is required to be present
    #if ((!defined CAU_V1_AVAILABLE && !defined CAU_V2_AVAILABLE && !defined AES_DISABLE_CAU) && (!defined LTC_AVAILABLE || defined AES_DISABLE_LTC)) // if hardware accelerator is not available or is not to be used
        #undef NATIVE_AES_CAU
    #endif
    #if defined NATIVE_AES_CAU && (!defined LTC_AVAILABLE || defined AES_DISABLE_LTC) // hardware accelerator is available and is to be used ((LTC has priority over mmCAU)
    // Freescale mmCAU crypto acceleration routines
    //
    extern void mmcau_aes_set_key(const unsigned char *key, const int key_size, unsigned char *key_sch);
    extern void mmcau_aes_decrypt(const unsigned char *in, const unsigned char *key_sch, const int nr, unsigned char *out);
    extern void mmcau_aes_encrypt(const unsigned char *in, const unsigned char *key_sch, const int nr, unsigned char *out);
    #elif defined CRYPTO_MBEDTLS
    extern void aes_mbedTLS_var_init(void);
    #endif
    #if defined _WINDOWS || !defined NATIVE_AES_CAU                      // software based implemenation to be used (always used by simulation - possibly in parallel with HW code)
        #if defined CRYPTO_OPEN_SSL
            #include "../../stack/SSL/openssl-1.0.2/aes.h"               // AES uses wolfSSL code
        #elif defined CRYPTO_WOLF_SSL
            #include "../../stack/SSL/wolfssl-3.9.6/aes.h"               // AES uses wolfSSL code
        #else
            #include "../../stack/SSL/mbedtls-1.3.10/aes.h"              // AES uses mbedTLS code
        #endif
        #define SOFTWARE_BASE_AES
    #endif

/* =================================================================== */
/*                          local definitions                          */
/* =================================================================== */

    #if !defined AES_INSTANCE_COUNT
        #define AES_INSTANCE_COUNT   2                                   // if not otherwise specified we have 2 instances (one for encryption and one for decryption)
    #endif

/* =================================================================== */
/*                      local structure definitions                    */
/* =================================================================== */

typedef struct stAES_INSTANCE
{
    #if defined NATIVE_AES_CAU                                           // hardware accelerator to be used
    unsigned long ulRounds;                                              // ensures long word alignment
        #if !defined SOFTWARE_BASE_AES
    unsigned char iv[AES_BLOCK_LENGTH];                                  // long word aligned
        #endif
        #if !defined LTC_AVAILABLE || defined AES_DISABLE_LTC            // mmCAU
    unsigned char key[60 * 4];                                           // long word aligned
        #endif
        #if defined NATIVE_AES_CAU && (defined LTC_AVAILABLE && !defined AES_DISABLE_LTC)
    unsigned long ulInSequence;
        #endif
    #endif
    #if defined SOFTWARE_BASE_AES                                        // software based implemenation to be used (always used by simulation - possibly in parallel with HW code)
        #if defined CRYPTO_OPEN_SSL                                      // uses openSSL method
    AES_KEY       aes_encrypt_context;
    unsigned char iv[AES_BLOCK_LENGTH];
        #elif defined CRYPTO_WOLF_SSL                                    // uses wolfSSL method
    Aes           aes_encrypt_context;
        #else                                                            // uses mbedTLS method
    aes_context   aes_encrypt_context;
    unsigned char iv[AES_BLOCK_LENGTH];
        #endif
    #endif
} AES_INSTANCE;


/* =================================================================== */
/*                      local variable definitions                     */
/* =================================================================== */

    #if defined NATIVE_AES_CAU
    static AES_INSTANCE aes_instance[AES_INSTANCE_COUNT] = {{0}};
    #elif defined CRYPTO_OPEN_SSL || defined CRYPTO_WOLF_SSL
    static AES_INSTANCE aes_instance[AES_INSTANCE_COUNT] = {{{{0}}}};
    #else
    static AES_INSTANCE aes_instance[AES_INSTANCE_COUNT] = {{{0}}};
    #endif

    #if defined NATIVE_AES_CAU && (defined LTC_AVAILABLE && !defined AES_DISABLE_LTC) // use LTC HW accelerator (has priority over mmCAU when both are available)
static void fnInitLTC(const unsigned long *_ptr_ulKey, int iKeyLength,  int iDecrypt)
{
    POWER_UP_LTC_MODULE();                                               // ensure that the module is powered up before use
    WRITE_ONE_TO_CLEAR(LTC0_CW, (LTC_CW_CM | LTC_CW_CDS | LTC_CW_CICV | LTC_CW_CCR | LTC_CW_CKR | LTC_CW_CPKA | LTC_CW_CPKB | LTC_CW_CPKN | LTC_CW_CPKE | LTC_CW_COF | LTC_CW_CIF)); // clear internal registers
    LTC0_CTL = (LTC_CTL_IFS | LTC_CTL_OFS | LTC_CTL_KIS | LTC_CTL_KOS | LTC_CTL_CIS | LTC_CTL_COS); // enable byte swap for registers to be used
  //if (iDecrypt != 0) {
  //    LTC0_MD = (LTC_MD_ALG_AES | LTC_MD_AAI_CBC | LTC_MD_AAI_DK);     // AES key for decryption (this sets the error interrupt but allow decrypt to operate - setting AES algorithm doesn't trigger error interrupt but causes descrpt to fail)
  //}
  //else {
        LTC0_MD = (LTC_MD_ALG_AES | LTC_MD_AAI_CBC);                     // AES key for encryption
  //}
    LTC0_KEY_0 = *_ptr_ulKey++;                                          // key needs to be long word aligned - enter them into the LTC
    LTC0_KEY_1 = *_ptr_ulKey++;
    LTC0_KEY_2 = *_ptr_ulKey++;
    LTC0_KEY_3 = *_ptr_ulKey++;                                          // AES128
    if (iKeyLength > 128) {
        LTC0_KEY_4 = *_ptr_ulKey++;
        LTC0_KEY_5 = *_ptr_ulKey++;                                      // AES192
        if (iKeyLength == 256) {
            LTC0_KEY_6 = *_ptr_ulKey++;
            LTC0_KEY_7 = *_ptr_ulKey;                                    // AES256
            LTC0_KS = 32;
        }
        else {
            LTC0_KS = 24;
        }
    }
    else {
        LTC0_KS = 16;                                                    // the key size must be written after setting the key and it locks the register against further writes
    }
    /*
    LTC0_CTX_0 = 0;                                                      // zero IV
    LTC0_CTX_1 = 0;
    LTC0_CTX_2 = 0;
    LTC0_CTX_3 = 0;*/
    WRITE_ONE_TO_CLEAR(LTC0_STA, LTC_STA_DI);                            // reset the done interrupt
}
    #endif
#endif

#if defined CRYPTOGRAPHY && defined CRYPTO_AES
// AES key or iv initialisation
//
extern int fnAES_Init(int iInstanceCommand, const unsigned char *ptrKey, int iKeyLength)
{
    int iInstance = (iInstanceCommand & AES_INSTANCE_MASK);
    if (iInstance >= AES_INSTANCE_COUNT) {
        return AES_INVALID_INSTANCE_REFERENCE;
    }
    #if defined NATIVE_AES_CAU
    if ((iInstanceCommand & (AES_COMMAND_AES_SET_KEY_ENCRYPT | AES_COMMAND_AES_SET_KEY_DECRYPT)) != 0) {
        if (((CAST_POINTER_ARITHMETIC)ptrKey & 0x3) != 0) {              // mmCAU/LTC requires long word aligned input key
            _EXCEPTION("Check secret key buffer alignment!");
            return AES_ENCRYPT_BAD_ALIGNMENT;
        }
    }
    #elif defined CRYPTO_MBEDTLS && !defined MBEDTLS_AES_ROM_TABLES
    if ((iInstanceCommand & AES_COMMAND_AES_RE_INIT) != 0) {
        aes_mbedTLS_var_init();
    }
    #endif
    if ((iInstanceCommand & (AES_COMMAND_AES_SET_KEY_ENCRYPT | AES_COMMAND_AES_SET_KEY_DECRYPT)) != 0) {
        switch (iKeyLength) {
        case 128:
    #if defined NATIVE_AES_CAU                                           // mmCAU/LTC hardware accelerator
            aes_instance[iInstance].ulRounds = 10;
    #endif
            break;
        case 192:
    #if defined NATIVE_AES_CAU                                           // mmCAU/LTC hardware accelerator
            aes_instance[iInstance].ulRounds = 12;
    #endif
            break;
        case 256:
    #if defined NATIVE_AES_CAU                                           // mmCAU/LTC hardware accelerator
            aes_instance[iInstance].ulRounds = 14;
    #endif
            break;
        default:
            _EXCEPTION("Bad key length!");
            return AES_INVALID_KEY_LENGTH;
        }
    }
    if ((iInstanceCommand & AES_COMMAND_AES_SET_KEY_ENCRYPT) != 0) {     // encyption key
    #if defined NATIVE_AES_CAU
        #if defined LTC_AVAILABLE && !defined AES_DISABLE_LTC            // use LTC HW accelerator (has priority over mmCAU when both are available)
        fnInitLTC((const unsigned long *)ptrKey, iKeyLength, 0);
        aes_instance[iInstance].ulInSequence = 0;                        // encrypt key loaded (not decrypt)
        #else                                                            // use mmCAU HW accelerator
            #if !defined _WINDOWS
        mmcau_aes_set_key(ptrKey, iKeyLength, aes_instance[iInstance].key);
            #endif
        uMemset(aes_instance[iInstance].iv, 0, sizeof(aes_instance[iInstance].iv));
        #endif
    #endif
    #if defined SOFTWARE_BASE_AES                                        // software based implemenation to be used (always used by simulation - possibly in parallel with HW code)
        #if defined CRYPTO_OPEN_SSL                                      // openSSL
        private_AES_set_encrypt_key(ptrKey, iKeyLength, &aes_instance[iInstance].aes_encrypt_context);
        uMemset(aes_instance[iInstance].iv, 0, sizeof(aes_instance[iInstance].iv));
        #elif defined CRYPTO_WOLF_SSL                                    // wolfSSL
        wc_AesSetKey(&aes_instance[iInstance].aes_encrypt_context, ptrKey, (iKeyLength/8), NULL, AES_ENCRYPTION); // initial vectors are zeroed in the function
        #else                                                            // mbedTLS
        aes_setkey_enc(&aes_instance[iInstance].aes_encrypt_context, ptrKey, iKeyLength); // set encryption key
        uMemset(aes_instance[iInstance].iv, 0, sizeof(aes_instance[iInstance].iv)); // reset the vector content
        #endif
    #endif
    }
    else if ((iInstanceCommand & AES_COMMAND_AES_SET_KEY_DECRYPT) != 0) {// decrypt
    #if defined NATIVE_AES_CAU
        #if defined LTC_AVAILABLE && !defined AES_DISABLE_LTC            // LTC hardware accelerator
        fnInitLTC((const unsigned long *)ptrKey, iKeyLength, 1);
        aes_instance[iInstance].ulInSequence = 0;                        // encrypt key loaded (not decrypt)
        #else                                                            // mmCAU hardware accelerator
            #if !defined _WINDOWS
        mmcau_aes_set_key(ptrKey, iKeyLength, aes_instance[iInstance].key);
            #endif
        uMemset(aes_instance[iInstance].iv, 0, sizeof(aes_instance[iInstance].iv));
        #endif
    #endif
    #if defined SOFTWARE_BASE_AES                                        // software based implemenation to be used (always used by simulation - possibly in parallel with HW code)
        #if defined CRYPTO_OPEN_SSL                                      // openSSL software method
        private_AES_set_decrypt_key(ptrKey, iKeyLength, &aes_instance[iInstance].aes_encrypt_context);
        uMemset(aes_instance[iInstance].iv, 0, sizeof(aes_instance[iInstance].iv));
        #elif defined CRYPTO_WOLF_SSL                                    // wolfSSL software method
        wc_AesSetKey(&aes_instance[iInstance].aes_encrypt_context, ptrKey, (iKeyLength/8), NULL, AES_DECRYPTION); // initial vectors are zeroed in the function
        #else                                                            // mbedTLS software method
        aes_setkey_dec(&aes_instance[iInstance].aes_encrypt_context, ptrKey, iKeyLength); // set decryption key
        uMemset(aes_instance[iInstance].iv, 0, sizeof(aes_instance[iInstance].iv));
        #endif
    #endif
    }
    else if ((iInstanceCommand & AES_COMMAND_AES_PRIME_IV) != 0) {       // prime the vector
    #if defined NATIVE_AES_CAU && defined LTC_AVAILABLE && !defined AES_DISABLE_LTC // LTC HW accelerator
        LTC0_CTX_0 = ((ptrKey[3] << 24) | (ptrKey[2] << 16) | (ptrKey[1] << 8) | ptrKey[0]); // prime IV
        LTC0_CTX_1 = ((ptrKey[7] << 24) | (ptrKey[6] << 16) | (ptrKey[5] << 8) | ptrKey[4]);
        LTC0_CTX_2 = ((ptrKey[11] << 24) | (ptrKey[10] << 16) | (ptrKey[9] << 8) | ptrKey[8]);
        LTC0_CTX_3 = ((ptrKey[15] << 24) | (ptrKey[14] << 16) | (ptrKey[13] << 8) | ptrKey[12]);
    #endif
    #if defined SOFTWARE_BASE_AES && defined CRYPTO_WOLF_SSL
        uMemcpy(aes_instance[iInstance].aes_encrypt_context.reg, ptrKey, sizeof(aes_instance[iInstance].aes_encrypt_context.reg)); // prime the iv
    #else
        uMemcpy(aes_instance[iInstance].iv, ptrKey, sizeof(aes_instance[iInstance].iv)); // prime the iv
    #endif
    }
    else if ((iInstanceCommand & AES_COMMAND_AES_RESET_IV) != 0) {       // reset the vector (or prime with zero vector content)
    #if defined NATIVE_AES_CAU && defined LTC_AVAILABLE && !defined AES_DISABLE_LTC // LTC HW accelerator
        LTC0_CTX_0 = 0;                                                  // zero IV
        LTC0_CTX_1 = 0;
        LTC0_CTX_2 = 0;
        LTC0_CTX_3 = 0;
    #endif
    #if defined SOFTWARE_BASE_AES                                        // software based implemenation to be used (always used by simulation - possibly in parallel with HW code)
        #if defined CRYPTO_WOLF_SSL
        uMemset(aes_instance[iInstance].aes_encrypt_context.reg, 0, sizeof(aes_instance[iInstance].aes_encrypt_context.reg)); // zero the initial vector
        #else
        uMemset(aes_instance[iInstance].iv, 0, sizeof(aes_instance[iInstance].iv));
        #endif
    #endif
    }
    return 0;
}

// AES encryption or decryption
//
extern int fnAES_Cipher(int iInstanceCommand, const unsigned char *ptrTextIn, unsigned char *ptrTextOut, unsigned long ulDataLength)
{
    int iInstance = (iInstanceCommand & AES_INSTANCE_MASK);
    if (iInstance >= AES_INSTANCE_COUNT) {
        return AES_INVALID_INSTANCE_REFERENCE;
    }
    if ((iInstanceCommand & (AES_COMMAND_AES_ENCRYPT | AES_COMMAND_AES_DECRYPT | AES_COMMAND_AES_DECRYPT_128)) != 0) { // encryption or decryption to be performed
    #if (defined CRYPTO_OPEN_SSL || defined CRYPTO_MBEDTLS) && defined SOFTWARE_BASE_AES // software based implemenation to be used (always used by simulation - possibly in parallel with HW code)
        int iMode;
    #endif
    #if defined NATIVE_AES_CAU                                           // hardware accelerator in use
        if (aes_instance[iInstance].ulRounds == 0) {
            _EXCEPTION("Initialise AES instance before use!!");
            return AES_INSTANCE_NOT_INITIALISED;
        }
    #endif
    #if defined SOFTWARE_BASE_AES                                        // software based implemenation to be used (always used by simulation - possibly in parallel with HW code)
        #if defined CRYPTO_WOLF_SSL || defined CRYPTO_OPEN_SSL
        if (aes_instance[iInstance].aes_encrypt_context.rounds == 0) {
            _EXCEPTION("Initialise AES instance before use!!");
            return AES_INSTANCE_NOT_INITIALISED;
        }
        #else
        if (aes_instance[iInstance].aes_encrypt_context.nr == 0) {
            _EXCEPTION("Initialise AES instance before use!!");
            return AES_INSTANCE_NOT_INITIALISED;
        }
        #endif
        if ((ulDataLength % AES_BLOCK_LENGTH) != 0) {
            _EXCEPTION("Invalid AES data length being used!!");
            uMemset(ptrTextOut, 0, ulDataLength);                        // if a bad length is called we set the ciphertext to zeros so that if it is still sent it will not be plain text in any way (eg. when the plain text input buffer is being used also as output buffer)
            return AES_ENCRYPT_BAD_LENGTH;
        }
        #if defined CRYPTO_WOLF_SSL && !defined NATIVE_AES_CAU
        if ((iInstanceCommand & AES_COMMAND_AES_RESET_IV) != 0) {
            uMemset(aes_instance[iInstance].aes_encrypt_context.reg, 0, sizeof(aes_instance[iInstance].aes_encrypt_context.reg)); // zero the initial vector
        }
        if ((iInstanceCommand & AES_COMMAND_AES_ENCRYPT) != 0) {
            wc_AesCbcEncrypt(&aes_instance[iInstance].aes_encrypt_context, ptrTextOut, ptrTextIn, ulDataLength);
        }
        else {
            wc_AesCbcDecrypt(&aes_instance[iInstance].aes_encrypt_context, ptrTextOut, ptrTextIn, ulDataLength);
        }
        #endif
    #endif
    #if defined NATIVE_AES_CAU && defined _WINDOWS                       // hardware accelerator
        if ((iInstanceCommand & AES_COMMAND_AES_RESET_IV) != 0) {        // reset the initial vector before starting encrypting/decrypting a new block
            uMemset(aes_instance[iInstance].iv, 0, sizeof(aes_instance[iInstance].iv));
        }
    #endif
        if ((iInstanceCommand & AES_COMMAND_AES_ENCRYPT) != 0) {         // encrypt data
    #if defined NATIVE_AES_CAU                                           // hardware accelerator
        #if !defined LTC_AVAILABLE || defined AES_DISABLE_LTC            // mmCAU hardware accelerator
            register unsigned long *ptrPlainTextInput = (unsigned long *)ptrTextIn;
            register unsigned long *ptrCipherTextOutput = (unsigned long *)ptrTextOut;
            register unsigned long *ptrInputVector = (unsigned long *)aes_instance[iInstance].iv;
            register unsigned long _ulDataLength = ulDataLength;
            if ((((CAST_POINTER_ARITHMETIC)ptrPlainTextInput | (CAST_POINTER_ARITHMETIC)ptrCipherTextOutput) & 0x3) != 0) { // mmCAU requires long word aligned output buffer and input buffer should also be aligned for efficiency
                _EXCEPTION("Check buffer alignment!");
                uMemset(ptrTextOut, 0, ulDataLength);                    // if a bad length is called we set the ciphertext to zeros so that if it is still sent it will not be plain text in any way (eg. when the plain text input buffer is being used also as output buffer)
                return AES_ENCRYPT_BAD_ALIGNMENT;
            }
        #endif
        #if defined LTC_AVAILABLE && !defined AES_DISABLE_LTC            // LTC HW accelerator
            if ((iInstanceCommand & AES_COMMAND_AES_RESET_IV) != 0) {    // if the initial vector is to be reset before start
                LTC0_CTX_0 = 0;                                          // zero IV
                LTC0_CTX_1 = 0;
                LTC0_CTX_2 = 0;
                LTC0_CTX_3 = 0;
            }
            fnExecuteLTC((unsigned long *)ptrTextIn, (unsigned long *)ptrTextOut, ulDataLength, (LTC_MD_ENC_ENCRYPT | LTC_MD_AS_UPDATE | LTC_MD_ALG_AES | LTC_MD_AAI_CBC)); // encrypt using LTC
        #else                                                            // mmCAU HW accelerator
            if (_ulDataLength >= 16) {                                   // first block
                if ((iInstanceCommand & AES_COMMAND_AES_RESET_IV) != 0) {// if the initial vector is to be reset before start
                    ptrCipherTextOutput[0] = ptrPlainTextInput[0];       // iv[] are zero so don't XOR the plaintext input with it (this can be performed as long word copies since both are long word aligned)
                    ptrCipherTextOutput[1] = ptrPlainTextInput[1];
                    ptrCipherTextOutput[2] = ptrPlainTextInput[2];
                    ptrCipherTextOutput[3] = ptrPlainTextInput[3];
                }
                else {
                    ptrCipherTextOutput[0] = (ptrPlainTextInput[0] ^ ptrInputVector[0]); // this can be performed as long word copies since both are long word aligned
                    ptrCipherTextOutput[1] = (ptrPlainTextInput[1] ^ ptrInputVector[1]);
                    ptrCipherTextOutput[2] = (ptrPlainTextInput[2] ^ ptrInputVector[2]);
                    ptrCipherTextOutput[3] = (ptrPlainTextInput[3] ^ ptrInputVector[3]);
                }
            #if !defined _WINDOWS
                mmcau_aes_encrypt((const unsigned char *)ptrCipherTextOutput, aes_instance[iInstance].key, (const int)aes_instance[iInstance].ulRounds, (unsigned char *)ptrCipherTextOutput);
                ptrInputVector[0] = ptrCipherTextOutput[0];              // this can be performed as long word copies since both are long word aligned
                ptrInputVector[1] = ptrCipherTextOutput[1];
                ptrInputVector[2] = ptrCipherTextOutput[2];
                ptrInputVector[3] = ptrCipherTextOutput[3];
            #endif
                ptrPlainTextInput += 4;
                ptrCipherTextOutput += 4;
                _ulDataLength -= 16;
            }
            while (_ulDataLength >= 16) {                                // for each subsequent block
                ptrCipherTextOutput[0] = (ptrPlainTextInput[0] ^ ptrInputVector[0]); // this can be performed as long word copies since both are long word aligned
                ptrCipherTextOutput[1] = (ptrPlainTextInput[1] ^ ptrInputVector[1]);
                ptrCipherTextOutput[2] = (ptrPlainTextInput[2] ^ ptrInputVector[2]);
                ptrCipherTextOutput[3] = (ptrPlainTextInput[3] ^ ptrInputVector[3]);
            #if !defined _WINDOWS
                mmcau_aes_encrypt((const unsigned char *)ptrCipherTextOutput, aes_instance[iInstance].key, (const int)aes_instance[iInstance].ulRounds, (unsigned char *)ptrCipherTextOutput);
                ptrInputVector[0] = ptrCipherTextOutput[0];              // this can be performed as long word copies since both are long word aligned
                ptrInputVector[1] = ptrCipherTextOutput[1];
                ptrInputVector[2] = ptrCipherTextOutput[2];
                ptrInputVector[3] = ptrCipherTextOutput[3];
            #endif
                ptrPlainTextInput += 4;
                ptrCipherTextOutput += 4;
                _ulDataLength -= 16;
            }
        #endif
    #endif
    #if defined SOFTWARE_BASE_AES                                        // software based implemenation to be used (always used by simulation - possibly in parallel with HW code)
        #if (defined CRYPTO_OPEN_SSL || defined CRYPTO_MBEDTLS)
            iMode = AES_ENCRYPT;                                         // the mode to use for SW method
        #endif
    #else
            return 0;
    #endif
        }
        else {                                                           // decryption
    #if defined NATIVE_AES_CAU                                           // hardware decryption to be used
        #if !defined LTC_AVAILABLE || defined AES_DISABLE_LTC            // mmCAU to be used LTC (has priority over mmCAU when both are available and LTC enabled)
            register unsigned long *ptrCipherTextInput = (unsigned long *)ptrTextIn;
            register unsigned long *ptrPlainTextOutput = (unsigned long *)ptrTextOut;
            register unsigned long *ptrInputVector = (unsigned long *)aes_instance[iInstance].iv;
            register unsigned long _ulDataLength = ulDataLength;
            if ((((CAST_POINTER_ARITHMETIC)ptrCipherTextInput | (CAST_POINTER_ARITHMETIC)ptrPlainTextOutput) & 0x3) != 0) { // mmCAU requires long word aligned output buffer and input buffer should also be aligned for efficiency
                _EXCEPTION("Check buffer alignment!");
                uMemset(ptrTextOut, 0, ulDataLength);                    // if a bad length is called we set the ciphertext to zeros so that if it is still sent it will not be plain text in any way (eg. when the plain text input buffer is being used also as output buffer)
                return AES_ENCRYPT_BAD_ALIGNMENT;
            }
            if (ptrCipherTextInput == ptrPlainTextOutput) {              // if the same input buffer being used as output buffer
                unsigned long ulBackup[4];                               // if the decryption is being performed in place we need to backup the original decrypted block to be able to create the following initial vector
                if (_ulDataLength >= 16) {                               // first block
                    ulBackup[0] = ptrCipherTextInput[0];                 // backup the input cipher block
                    ulBackup[1] = ptrCipherTextInput[1];
                    ulBackup[2] = ptrCipherTextInput[2];
                    ulBackup[3] = ptrCipherTextInput[3];
            #if !defined _WINDOWS
                    mmcau_aes_decrypt((const unsigned char *)ptrCipherTextInput, aes_instance[iInstance].key, (const int)aes_instance[iInstance].ulRounds, (unsigned char *)ptrPlainTextOutput); // decrypt a block
            #endif
                    if ((iInstanceCommand & AES_COMMAND_AES_RESET_IV) == 0) { // if iv[] are zero we don't XOR the plaintext input with it
            #if !defined _WINDOWS
                        ptrPlainTextOutput[0] ^= ptrInputVector[0];      // this can be performed as long word copies since both are long word aligned
                        ptrPlainTextOutput[1] ^= ptrInputVector[1];
                        ptrPlainTextOutput[2] ^= ptrInputVector[2];
                        ptrPlainTextOutput[3] ^= ptrInputVector[3];
            #endif
                    }
            #if !defined _WINDOWS
                    ptrInputVector[0] = ulBackup[0];                     // next iv[] is the input before descrypting (this can be performed as long word copies since both are long word aligned)
                    ptrInputVector[1] = ulBackup[1];
                    ptrInputVector[2] = ulBackup[2];
                    ptrInputVector[3] = ulBackup[3];
            #endif
                    ptrCipherTextInput += 4;
                    ptrPlainTextOutput += 4;
                    _ulDataLength -= 16;
                }
                while (_ulDataLength >= 16) {                            // subsequent blocks
                    ulBackup[0] = ptrCipherTextInput[0];                 // backup the input cipher block
                    ulBackup[1] = ptrCipherTextInput[1];
                    ulBackup[2] = ptrCipherTextInput[2];
                    ulBackup[3] = ptrCipherTextInput[3];
            #if !defined _WINDOWS
                    mmcau_aes_decrypt((const unsigned char *)ptrCipherTextInput, aes_instance[iInstance].key, (const int)aes_instance[iInstance].ulRounds, (unsigned char *)ptrPlainTextOutput); // decrypt a block
            #endif
            #if !defined _WINDOWS
                    ptrPlainTextOutput[0] ^= ptrInputVector[0];          // this can be performed as long word copies since both are long word aligned
                    ptrPlainTextOutput[1] ^= ptrInputVector[1];
                    ptrPlainTextOutput[2] ^= ptrInputVector[2];
                    ptrPlainTextOutput[3] ^= ptrInputVector[3];
                    ptrInputVector[0] = ulBackup[0];                     // next iv[] is the input before descrypting (this can be performed as long word copies since both are long word aligned)
                    ptrInputVector[1] = ulBackup[1];
                    ptrInputVector[2] = ulBackup[2];
                    ptrInputVector[3] = ulBackup[3];
            #endif
                    ptrCipherTextInput += 4;
                    ptrPlainTextOutput += 4;
                    _ulDataLength -= 16;
                }
            }
            else {                                                       // not in place (no temporary buffer needed)
                if (_ulDataLength >= 16) {                               // first block
            #if !defined _WINDOWS
                    mmcau_aes_decrypt((const unsigned char *)ptrCipherTextInput, aes_instance[iInstance].key, (const int)aes_instance[iInstance].ulRounds, (unsigned char *)ptrPlainTextOutput); // decrypt a block
            #endif
                    if ((iInstanceCommand & AES_COMMAND_AES_RESET_IV) == 0) { // if iv[] are zero we don't XOR the plaintext input with it
            #if !defined _WINDOWS
                        ptrPlainTextOutput[0] ^= ptrInputVector[0];      // this can be performed as long word copies since both are long word aligned
                        ptrPlainTextOutput[1] ^= ptrInputVector[1];
                        ptrPlainTextOutput[2] ^= ptrInputVector[2];
                        ptrPlainTextOutput[3] ^= ptrInputVector[3];
            #endif
                    }
            #if !defined _WINDOWS
                    ptrInputVector[0] = ptrCipherTextInput[0];           // next iv[] is the input before descrypting (this can be performed as long word copies since both are long word aligned)
                    ptrInputVector[1] = ptrCipherTextInput[1];
                    ptrInputVector[2] = ptrCipherTextInput[2];
                    ptrInputVector[3] = ptrCipherTextInput[3];
            #endif
                    ptrCipherTextInput += 4;
                    ptrPlainTextOutput += 4;
                    _ulDataLength -= 16;
                }
                while (_ulDataLength >= 16) {                            // subsequent blocks
            #if !defined _WINDOWS
                    mmcau_aes_decrypt((const unsigned char *)ptrCipherTextInput, aes_instance[iInstance].key, (const int)aes_instance[iInstance].ulRounds, (unsigned char *)ptrPlainTextOutput); // decrypt a block
            #endif
            #if !defined _WINDOWS
                    ptrPlainTextOutput[0] ^= ptrInputVector[0];          // this can be performed as long word copies since both are long word aligned
                    ptrPlainTextOutput[1] ^= ptrInputVector[1];
                    ptrPlainTextOutput[2] ^= ptrInputVector[2];
                    ptrPlainTextOutput[3] ^= ptrInputVector[3];
                    ptrInputVector[0] = ptrCipherTextInput[0];           // next iv[] is the input before descrypting (this can be performed as long word copies since both are long word aligned)
                    ptrInputVector[1] = ptrCipherTextInput[1];
                    ptrInputVector[2] = ptrCipherTextInput[2];
                    ptrInputVector[3] = ptrCipherTextInput[3];
            #endif
                    ptrCipherTextInput += 4;
                    ptrPlainTextOutput += 4;
                    _ulDataLength -= 16;
                }
            }
        #else                                                            // LTC hardware accelerator
            fnExecuteLTC((unsigned long *)ptrTextIn, (unsigned long *)ptrTextOut, ulDataLength, ((LTC_MD_ENC_DECRYPT | LTC_MD_AS_UPDATE | LTC_MD_ALG_AES | LTC_MD_AAI_CBC | aes_instance[iInstance].ulInSequence))); // decrypt using LTC
            aes_instance[iInstance].ulInSequence = LTC_MD_AAI_DK;        // after the first decryption operation the LTC has the decrypt loader (which it derived from the encrypt key) so we mark that it should be used directly from now on in a decryption sequence
        #endif
    #endif
    #if defined SOFTWARE_BASE_AES                                        // software based implemenation to be used (always used by simulation - possibly in parallel with HW code)
        #if (defined CRYPTO_OPEN_SSL || defined CRYPTO_MBEDTLS)
            iMode = AES_DECRYPT;                                         // decryption method to be used
        #endif
    #else
            return 0;
    #endif
        }
    #if defined SOFTWARE_BASE_AES                                        // software based implemenation to be used (always used by simulation - possibly in parallel with HW code)
        #if defined CRYPTO_OPEN_SSL                                      // use openSSL method
        AES_cbc_encrypt(ptrTextIn, ptrTextOut, ulDataLength, (const AES_KEY *)&aes_instance[iInstance], aes_instance[iInstance].iv, iMode); // encrypt always using cipher block chaining
        #else                                                            // use mbedTLS method
            #if defined _iMX && (defined iMX_BOOTLOADER || defined iMX_SERIAL_LOADER || defined _WINDOWS)
        if ((iInstanceCommand & (AES_COMMAND_AES_DECRYPT_128)) != 0) {   // use AES-CTR-128
            size_t nc_off = 0;
            unsigned char stream_block[16];
            aes_crypt_ctr(&aes_instance[iInstance].aes_encrypt_context, ulDataLength, &nc_off, aes_instance[iInstance].iv, stream_block, ptrTextIn, ptrTextOut);
        }
        else {
            #endif
            aes_crypt_cbc(&aes_instance[iInstance].aes_encrypt_context, iMode, ulDataLength, aes_instance[iInstance].iv, ptrTextIn, ptrTextOut); // encrypt always using cipher block chaining
            #if defined _iMX && (defined iMX_BOOTLOADER || defined iMX_SERIAL_LOADER || defined _WINDOWS)
        }
            #endif
        #endif
    #endif
        return 0;
    }
    return AES_INVALID_CIPHER_OPERATION;
}
#endif

#if defined CRYPTOGRAPHY && defined CRYPTO_SHA
    #if defined NATIVE_SHA256_CAU && ((defined LTC_AVAILABLE && defined LTC_HAS_SHA) && !defined SHA_DISABLE_LTC) // LTC hardware accelerator
static void fnExecuteLTC(unsigned long *ptrPlainTextInput, unsigned long *ptrCipherTextOutput, unsigned long _ulDataLength, unsigned long ulCommand)
{
    while (_ulDataLength != 0) {
        LTC0_MD = ulCommand;                                             // set/re-write the mode
        unsigned long ulThisLengthIn;
        unsigned long ulThisLengthOut;
        if (_ulDataLength > 0xff0) {
            ulThisLengthIn = 0xff0;
        }
        else {
            ulThisLengthIn = _ulDataLength;
        }
        _ulDataLength -= ulThisLengthIn;
        LTC0_DS = ulThisLengthIn;                                        // write the data size
        ulThisLengthIn = ((ulThisLengthIn + (sizeof(unsigned long) - 1))/sizeof(unsigned long)); // the number of long words (rounded up)
        ulThisLengthOut = ulThisLengthIn;
        while (ulThisLengthIn-- != 0) {                                  // copy to the input FIFO and read from the output FIFO
            while ((LTC0_FIFOSTA & LTC_FIFOSTA_IFF) != 0) {              // if the input FIFO is full we must wait before adding further data
                if ((ulCommand & LTC_MD_ALG_AES) != 0) {                 // if AES type
                    if ((LTC0_FIFOSTA & LTC_FIFOSTA_OFL_MASK) != 0) {    // if there is at least one output result ready
                        *ptrCipherTextOutput++ = LTC0_OFIFO;
                        ulThisLengthOut--;
                    }
                }
            }
            LTC0_IFIFO = *ptrPlainTextInput++;                           // long word aligned
        #if defined _WINDOWS
            LTC0_FIFOSTA |= LTC_FIFOSTA_OFL_MASK;
            LTC0_OFIFO = LTC0_IFIFO;
        #endif
            if ((ulCommand & LTC_MD_ALG_AES) != 0) {                     // if AES type
                if ((LTC0_FIFOSTA & LTC_FIFOSTA_OFL_MASK) != 0) {        // if there is at least one output result ready
                    *ptrCipherTextOutput++ = LTC0_OFIFO;
                    ulThisLengthOut--;
                }
            }
        }
        if ((ulCommand & LTC_MD_ALG_AES) != 0) {                         // if AES type
            while (ulThisLengthOut != 0) {
                if ((LTC0_FIFOSTA & LTC_FIFOSTA_OFL_MASK) != 0) {        // if there is at least one output result ready
                    *ptrCipherTextOutput++ = LTC0_OFIFO;
                    ulThisLengthOut--;
                }
            }
        }
        while ((LTC0_STA & LTC_STA_DI) == 0) {                       // wait for completion
        #if defined _WINDOWS
            LTC0_STA |= LTC_STA_DI;
        #endif
        }
        if ((ulCommand & LTC_MD_ALG_AES) != 0) {                         // if AES type
            WRITE_ONE_TO_CLEAR(LTC0_CW, LTC_CW_CDS);                     // clear the data size
            WRITE_ONE_TO_CLEAR(LTC0_STA, LTC_STA_DI);                    // reset the done interrupt
        }
    }
    if ((ulCommand & LTC_MD_ALG_SHA_TYPE) != 0) {                        // if SHA type
        *ptrCipherTextOutput++ = LTC0_CTX_0;                             // return the context
        *ptrCipherTextOutput++ = LTC0_CTX_1;
        *ptrCipherTextOutput++ = LTC0_CTX_2;
        *ptrCipherTextOutput++ = LTC0_CTX_4;
        *ptrCipherTextOutput++ = LTC0_CTX_5;
        *ptrCipherTextOutput++ = LTC0_CTX_6;
        *ptrCipherTextOutput++ = LTC0_CTX_7;
        *ptrCipherTextOutput = LTC0_CTX_8;
        WRITE_ONE_TO_CLEAR(LTC0_CW, (LTC_CW_CM | LTC_CW_CDS | LTC_CW_CICV | LTC_CW_CCR | LTC_CW_CKR | LTC_CW_CPKA | LTC_CW_CPKB | LTC_CW_CPKN | LTC_CW_CPKE | LTC_CW_COF | LTC_CW_CIF)); // clear internal registers
        WRITE_ONE_TO_CLEAR(LTC0_STA, LTC_STA_DI);                        // reset the done interrupt
    }
    else {
        WRITE_ONE_TO_CLEAR(LTC0_CW, (LTC_CW_CDS | LTC_CW_CICV | LTC_CW_CPKA | LTC_CW_CPKB | LTC_CW_CPKN | LTC_CW_CPKE | LTC_CW_COF | LTC_CW_CIF)); // clear internal registers
    }
}
    #endif

#include "../../stack/SSL/mbedtls-2.6/mbedtls/config.h"
#include "../../stack/SSL/mbedtls-2.6/mbedtls/ssl_internal.h"

    #if !defined USE_SECURE_SOCKET_LAYER                                 // dummy function pointers to satisfy mbedTLS SHA256 test functions that are otherwise not used
    void *(*mbedtls_calloc)(size_t n, size_t size) = 0;
    void(*mbedtls_free)(void *ptr) = 0;
    #endif
    #if (((!defined CAU_V1_AVAILABLE && !defined CAU_V2_AVAILABLE) || defined SHA_DISABLE_CAU) && (!(defined LTC_AVAILABLE && defined LTC_HAS_SHA) || defined SHA_DISABLE_LTC))
    #undef NATIVE_SHA256_CAU
    #endif
    #if defined _WINDOWS || !defined NATIVE_SHA256_CAU                   // software based implemenation to be used (always used by simulation - possibly in parallel with HW code)
    #define SOFTWARE_BASE_SHA
    #endif

    #if !defined _WINDOWS && (defined CAU_V1_AVAILABLE || defined CAU_V2_AVAILABLE) && !defined SHA256_DISABLE_CAU
    // Freescale mmCAU crypto acceleration routines
    //
    extern int  mmcau_sha256_initialize_output(const unsigned int *output);
    extern void mmcau_sha256_hash_n(const unsigned char *input, const int num_blks, unsigned int *output);
    extern void mmcau_sha256_update(const unsigned char *input, const int num_blks, unsigned int *output);
    extern void mmcau_sha256_hash(const unsigned char *input, unsigned int *output);
    #endif



    #if defined NATIVE_SHA256_CAU && !((defined LTC_AVAILABLE && defined LTC_HAS_SHA) && !defined SHA_DISABLE_LTC)

#define CRYPTO_LITTLE_ENDIAN                  0
#define CRYPTO_BIG_ENDIAN                     1

#define CRYPTO_ARRAY_LENGTH                   8
#define CRYPTO_MAX_PADDING_LENGTH             56
#define CRYPTO_BLOCK_LENGTH                   64

// Add padding to hash array
//
static void padding_start(mbedtls_sha256_context *ptr_sha256, const unsigned char *input, unsigned long *input_length, unsigned char endianess)
{
    unsigned char *padding_array = ptr_sha256->buffer;                   // long word aligned buffer (one block) followed by pad buffer space
    signed char  padding_length;
    unsigned int temp_length = (*input_length % CRYPTO_BLOCK_LENGTH);
    unsigned int bits_length;
    unsigned int final_length;

    // Get padding length: padding special case when 448 mod 512 -  working with bytes rather than bits
    //
    if (((padding_length = CRYPTO_MAX_PADDING_LENGTH - (temp_length % CRYPTO_BLOCK_LENGTH)) > 0) == 0) {
        padding_length = CRYPTO_BLOCK_LENGTH - (temp_length%CRYPTO_MAX_PADDING_LENGTH);
    }

    padding_length += (temp_length / CRYPTO_BLOCK_LENGTH);
    temp_length = *input_length;

    final_length = temp_length + padding_length + CRYPTO_ARRAY_LENGTH;

        #if defined _WINDOWS
    if (final_length > (sizeof(ptr_sha256->buffer) + sizeof(ptr_sha256->pad_buffer))) {
        _EXCEPTION("context only has space for one block plus padding!");
    }
        #endif

    uMemcpy(padding_array, input, temp_length);                          // copy original data to new padding array

    // Add padding
    //
    padding_array[temp_length++] = 0x80;                                 // first bit enabled
    uMemset(&padding_array[temp_length], 0, (padding_length + 8));       // fill with zero to end of padding array (inlcuding last double word, which will be used to save length to)

    // Add length depending on endianess: get number of bits
    //
    bits_length = (*input_length << 3);
    *input_length = final_length;

    if (endianess == CRYPTO_LITTLE_ENDIAN) {                             // SHA256 needs the length in big endian format
        temp_length += (padding_length - 1);
        padding_array[temp_length++] = (unsigned char)bits_length;
        padding_array[temp_length++] = (unsigned char)(bits_length >> 8);
        padding_array[temp_length++] = (unsigned char)(bits_length >> 16);
        padding_array[temp_length] = (unsigned char)(bits_length >> 24);
    }
    else {                                                               // CRYPTO_BIG_ENDIAN - MD-5 needs it in little-endian format
        temp_length += (padding_length + 3);
        padding_array[temp_length++] = (unsigned char)(bits_length >> 24);
        padding_array[temp_length++] = (unsigned char)(bits_length >> 16);
        padding_array[temp_length++] = (unsigned char)(bits_length >> 8);
        padding_array[temp_length] = (unsigned char)(bits_length);
    }
}
    #endif

extern int fnSHA256(const unsigned char *ptrInput, unsigned char *ptrOutput, unsigned long ulLength, int iMode)
{
    #if defined SOFTWARE_BASE_SHA || (defined NATIVE_SHA256_CAU && !((defined LTC_AVAILABLE && defined LTC_HAS_SHA) && !defined SHA_DISABLE_LTC))
    static mbedtls_sha256_context sha256;
    #endif
    #if defined NATIVE_SHA256_CAU
        #if (!(defined LTC_AVAILABLE && defined LTC_HAS_SHA) || defined SHA_DISABLE_LTC)
    unsigned long length = ulLength;
            #if defined _LITTLE_ENDIAN && !defined _WINDOWS
    int iOutLongWords;
    unsigned long *ptrLongWord;
            #endif
        #endif
        #if defined NATIVE_SHA256_CAU && !((defined LTC_AVAILABLE && defined LTC_HAS_SHA) && !defined SHA_DISABLE_LTC)
    padding_start(&sha256, ptrInput, &length, CRYPTO_BIG_ENDIAN);        // prepare padding for mmCAU SHA256 operation
        #endif
    #endif
    switch (iMode) {
    case SHA_START_CALCULATE_TERMINATE:
    #if defined NATIVE_SHA256_CAU
        #if ((defined LTC_AVAILABLE && defined LTC_HAS_SHA) && !defined SHA_DISABLE_LTC) // LTC hardware accelerator
        POWER_UP_LTC_MODULE();                                               // ensure that the module is powered up before use
        WRITE_ONE_TO_CLEAR(LTC0_CW, (LTC_CW_CM | LTC_CW_CDS | LTC_CW_CICV | LTC_CW_CCR | LTC_CW_CKR | LTC_CW_CPKA | LTC_CW_CPKB | LTC_CW_CPKN | LTC_CW_CPKE | LTC_CW_COF | LTC_CW_CIF)); // clear internal registers
        LTC0_CTL = (LTC_CTL_IFS | LTC_CTL_OFS | LTC_CTL_KIS | LTC_CTL_KOS | LTC_CTL_CIS | LTC_CTL_COS); // enable byte swap for registers to be used
        fnExecuteLTC((unsigned long *)ptrInput, (unsigned long *)ptrOutput, ulLength, ((LTC_MD_AS_INIT_FINAL | LTC_MD_ALG_SHA256)));
        #elif !defined _WINDOWS                                          // mmCAU hardware accelerator
        mmcau_sha256_update((const unsigned char *)sha256.buffer, (const int)(length/CRYPTO_BLOCK_LENGTH), (unsigned int *)ptrOutput);
            #if defined _LITTLE_ENDIAN
        iOutLongWords = ((CRYPTO_BLOCK_LENGTH / 2) / sizeof(unsigned long));
        ptrLongWord = (unsigned long *)ptrOutput;
        while (iOutLongWords-- != 0) {
            *ptrLongWord = BIG_LONG_WORD(*ptrLongWord);                  // convert the output from big-endian long storage to little-endian
            ptrLongWord++;
        }
            #endif
        #endif
    #endif
    #if defined SOFTWARE_BASE_SHA
        mbedtls_sha256_starts(&sha256, 0);
        mbedtls_sha256_update(&sha256, ptrInput, ulLength);
        mbedtls_sha256_finish(&sha256, ptrOutput);
    #endif
        break;
    case SHA_START_CALCULATE_STAY_OPEN:
    #if defined NATIVE_SHA256_CAU
        #if ((defined LTC_AVAILABLE && defined LTC_HAS_SHA) && !defined SHA_DISABLE_LTC) // LTC hardware accelerator
        #elif !defined _WINDOWS                                          // mmCAU hardware accelerator
        mmcau_sha256_initialize_output((const unsigned int *)sha256.state);
        mmcau_sha256_hash_n((const unsigned char *)sha256.buffer, (const int)(length/CRYPTO_BLOCK_LENGTH), (unsigned int *)sha256.state);
        #endif
    #endif
    #if defined SOFTWARE_BASE_SHA
        mbedtls_sha256_starts(&sha256, 0);
        mbedtls_sha256_update(&sha256, ptrInput, ulLength);
      //mbedtls_sha256_finish(&sha256, ptrOutput);
    #endif
        break;
    case SHA_CONTINUE_CALCULATING:
    #if defined NATIVE_SHA256_CAU
        #if ((defined LTC_AVAILABLE && defined LTC_HAS_SHA) && !defined SHA_DISABLE_LTC) // LTC hardware accelerator
        #elif !defined _WINDOWS                                          // mmCAU hardware accelerator
        mmcau_sha256_hash_n((const unsigned char *)sha256.buffer, (const int)(length/CRYPTO_BLOCK_LENGTH), (unsigned int *)sha256.state);
        #endif
    #endif
    #if defined SOFTWARE_BASE_SHA
      //mbedtls_sha256_starts(&sha256, 0);
        mbedtls_sha256_update(&sha256, ptrInput, ulLength);
      //mbedtls_sha256_finish(&sha256, ptrOutput);
    #endif
        break;
    case SHA_CONTINUE_CALCULATING_TERMINATE:
    #if defined NATIVE_SHA256_CAU
        #if ((defined LTC_AVAILABLE && defined LTC_HAS_SHA) && !defined SHA_DISABLE_LTC) // LTC hardware accelerator
        #elif !defined _WINDOWS                                          // mmCAU hardware accelerator
        mmcau_sha256_hash_n((const unsigned char *)sha256.buffer, (const int)(length / CRYPTO_BLOCK_LENGTH), (unsigned int *)ptrOutput);
            #if defined _LITTLE_ENDIAN
        iOutLongWords = ((CRYPTO_BLOCK_LENGTH / 2) / sizeof(unsigned long));
        ptrLongWord = (unsigned long *)ptrOutput;
        while (iOutLongWords-- != 0) {
            *ptrLongWord = BIG_LONG_WORD(*ptrLongWord);                  // convert the output from big-endian long storage to little-endian
            ptrLongWord++;
        }
            #endif
        #endif
    #endif
    #if defined SOFTWARE_BASE_SHA
      //mbedtls_sha256_starts(&sha256, 0);
        mbedtls_sha256_update(&sha256, ptrInput, ulLength);
        mbedtls_sha256_finish(&sha256, ptrOutput);
    #endif
        break;
    }
    return 0;
}
#endif
