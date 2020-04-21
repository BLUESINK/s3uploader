/*
 * utils_s3.h
 *
 *  Created on: Sep 23, 2019
 *      Author: yj
 */

#ifndef COMPONENTS_AWS_INCLUDE_UTILS_S3_H_
#define COMPONENTS_AWS_INCLUDE_UTILS_S3_H_

#include <string.h>
#include <time.h>

#include "crypto_base64.h"
#include "hmac-sha256.h"

extern BYTE policy[1024];
extern BYTE policy_base64[1024];

extern BYTE* createS3Policy(char* key, char* bucket, char* region, char* accessKey);
extern BYTE* StringToSign();
extern char* createS3Signature(char* key, char* filename);

#endif /* COMPONENTS_AWS_INCLUDE_UTILS_S3_H_ */
