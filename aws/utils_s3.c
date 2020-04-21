/*
 * utils_s3.c
 *
 *  Created on: Sep 23, 2019
 *      Author: yj
 */
#include "utils_s3.h"
#include "task_s3.h"


char policy[1024];

char dateStamp[32];
uint8_t DateKey[32];
uint8_t DateRegionKey[32];
uint8_t DateRegionServiceKey[32];
uint8_t SigningKey[32];
uint8_t signature_raw[32];


BYTE* createS3Policy(char* key, char* bucket, char* region, char* accessKey){

	uint16_t str_pointer = 0;

	time_t now;
	struct tm timeinfo = {0};
	struct tm expiration_timeinfo = {0};

	strcpy(policy, "{\"expiration\":\"");
	str_pointer = 15;

	// Set Time
	time(&now);
	localtime_r(&now, &timeinfo);
	now += 24*60*60;
	localtime_r(&now, &expiration_timeinfo);
	strftime(policy + str_pointer, 26, "%Y-%m-%dT%H:%M:%S.000Z\"", &expiration_timeinfo);
	str_pointer += 25;

	// Set Key
	strcpy(policy + str_pointer, ",\"conditions\":[[\"starts-with\",\"$key\",\""); // # : 38
	str_pointer += 38;
	strcpy(policy + str_pointer, key);
	str_pointer += strlen(key);

	// Set Bucket
	strcpy(policy + str_pointer, "\"],{\"bucket\":\"");
	str_pointer += 14;
	strcpy(policy + str_pointer, bucket);
	str_pointer += strlen(bucket);

	// Set credential
	strcpy(policy + str_pointer, "\"},{\"x-amz-algorithm\":\"AWS4-HMAC-SHA256\"},{\"acl\":\"public-read\"},{\"x-amz-credential\":\"");
	str_pointer += 85;


	int tmp_pointer = 0;
	strcpy(aws_s3.AWSCredential, accessKey); tmp_pointer += strlen(accessKey);
	strftime(aws_s3.AWSCredential + tmp_pointer, 11, "/%Y%m%d/", &timeinfo); tmp_pointer += 10;
	strcpy(aws_s3.AWSCredential + tmp_pointer, region); tmp_pointer += strlen(region);
	strcpy(aws_s3.AWSCredential + tmp_pointer, "/s3/aws4_request");


	strcpy(policy + str_pointer, accessKey);
	str_pointer += strlen(accessKey);
	strftime(policy + str_pointer, 11, "/%Y%m%d/", &timeinfo);
	str_pointer += 10;
	strcpy(policy + str_pointer, region);
	str_pointer += strlen(region);
	strcpy(policy + str_pointer, "/s3/aws4_request\"},{\"x-amz-date\":\"");
	str_pointer += 34;

	// Set Date
	strftime(dateStamp, 9, "%Y%m%d", &timeinfo);
	strftime(policy + str_pointer, 21,"%Y%m%dT000000Z\"}]}", &timeinfo);
	str_pointer += 20;

	strftime(aws_s3.AWSDate, 17, "%Y%m%dT000000Z", &timeinfo);
	return policy;
}


BYTE* StringToSign(){
	crypto_base64_encode(policy, aws_s3.policy_base64, strlen(policy), 0);
	return aws_s3.policy_base64;
}


void HexiDigest(char* dest, uint8_t* low_data){
	uint8_t i;
	uint8_t low, high;

	for(i = 0; i < 32; i++){
		high = low_data[i] >> 4; low = low_data[i] & 0x0F;
		if(high < 10) dest[2*i] = '0' + high;
		else dest[2*i] = 'a' + high - 10;

		if(low < 10) dest[2*i + 1] = '0' + low;
		else dest[2*i + 1] = 'a' + low - 10;
	}

	dest[64] = '\0';
}

char* createS3Signature(char* key, char* filename){

	strcpy(aws_s3.key, key); strcpy(aws_s3.key + strlen(key), "/"); strcpy(aws_s3.key + strlen(key) + 1, filename);
	strcpy(aws_s3.url, aws_s3.bucket);
	strcpy(aws_s3.url + strlen(aws_s3.bucket), ".s3.amazonaws.com");

	createS3Policy(key, aws_s3.bucket, aws_s3.region, aws_s3.AWSAccessKeyId);
	StringToSign();

	char x[256];
	strcpy(x, "AWS4");
	strcpy(x+4, aws_s3.AWSSecretAccessKey);


	hmac_sha256 (DateKey, (uint8_t*)dateStamp, strlen(dateStamp), (uint8_t*)x, strlen(x));
	hmac_sha256 (DateRegionKey, (uint8_t*)aws_s3.region, strlen(aws_s3.region), DateKey, 32);
	hmac_sha256 (DateRegionServiceKey, (uint8_t*)"s3", 2, DateRegionKey, 32);
	hmac_sha256 (SigningKey, (uint8_t*)"aws4_request", 12, DateRegionServiceKey, 32);
	hmac_sha256 (signature_raw, (uint8_t*)aws_s3.policy_base64, strlen(aws_s3.policy_base64), SigningKey, 32);

	HexiDigest(aws_s3.signature, signature_raw);

	return aws_s3.signature;
}
