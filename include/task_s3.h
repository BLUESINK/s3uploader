/*
 * task_s3.h
 *
 *  Created on: Sep 30, 2019
 *      Author: yj
 */

#ifndef COMPONENTS_AWS_INCLUDE_TASK_S3_H_
#define COMPONENTS_AWS_INCLUDE_TASK_S3_H_

#include <stdbool.h>
#include <stdio.h>

typedef struct AWS_S3{
	char unique_id[128];
	char bucket[128];
	char url[128];
	char region[128];
	char AWSAccessKeyId[128];
	char AWSSecretAccessKey[128];

	char AWSCredential[128];
	char AWSDate[128];
	char key[128];
	char policy_base64[1024];

	char signature[128];
}AWS_S3;

typedef enum AWS_S3_REQ{
	FILE_FSEEK_FAIL,
	DNS_LOOKUP_FAIL,
	SOCKET_ALLOC_FAIL,
	SOCKET_CONN_FAIL,
	SOCKET_WRITE_FAIL,
	AWS_RECEIVE_TIMEOUT,
	AWS_WRITE_FAIL,
	AWS_WRITE_SUCCESS
}AWS_S3_REQ;

extern AWS_S3 aws_s3;

extern AWS_S3_REQ task_s3(FILE* f, char* key, char* filename);

#endif /* COMPONENTS_AWS_INCLUDE_TASK_S3_H_ */
