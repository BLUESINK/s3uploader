/*********************************************************************
* Filename:   base64.h
* Author:     Brad Conte (brad AT bradconte.com)
* Copyright:
* Disclaimer: This code is presented "as is" without any guarantees.
* Details:    Defines the API for the corresponding Base64 implementation.
*********************************************************************/

#ifndef BASE64_H
#define BASE64_H

/*************************** HEADER FILES ***************************/
#include <stddef.h>

/**************************** DATA TYPES ****************************/
typedef char BYTE;             // 8-bit byte

/*********************** FUNCTION DECLARATIONS **********************/
// Returns the size of the output. If called with out = NULL, will just return
// the size of what the output would have been (without a terminating NULL).
extern size_t crypto_base64_encode(const BYTE in[], BYTE out[], size_t len, int newline_flag);


#endif   // BASE64_H
