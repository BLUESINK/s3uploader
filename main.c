#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <libgen.h>

#include "task_s3.h"

int main(int argc, char* argv[]) {

	if(argc > 3){
		printf("Input argument invalid.\n");
		return 0;
	}

	// Read AWS Data
	FILE *f_aws = NULL;
	f_aws = fopen(argv[1], "r");
	if(f_aws == NULL){
		printf("AWS Data file error.\n");
		exit(0);
	}

	char line_buff[128];
	char* pos;

	// Read Bucket name
	if(fgets(line_buff, sizeof(line_buff), f_aws) == NULL){printf("No AWS Bucket name!"); fclose(f_aws); return false;}
	pos = strchr(line_buff, '\n'); if(pos) *pos = '\0'; pos = strchr(line_buff, '\r'); if(pos) *pos = '\0';
	strcpy(aws_s3.bucket, line_buff); printf("[AWS Bucket Name] %s\n", aws_s3.bucket);

	// Read Region info
	if(fgets(line_buff, sizeof(line_buff), f_aws) == NULL){printf("No AWS Region information!"); fclose(f_aws); return false;}
	pos = strchr(line_buff, '\n'); if(pos) *pos = '\0'; pos = strchr(line_buff, '\r'); if(pos) *pos = '\0';
	strcpy(aws_s3.region, line_buff); printf("[AWS Region] %s\n", aws_s3.region);

	// Read AccessKey
	if(fgets(line_buff, sizeof(line_buff), f_aws) == NULL){printf("No Access Key information!"); fclose(f_aws); return false;}
	pos = strchr(line_buff, '\n'); if(pos) *pos = '\0'; pos = strchr(line_buff, '\r'); if(pos) *pos = '\0';
		strcpy(aws_s3.AWSAccessKeyId, line_buff); printf("[AWS Access Key] %s\n", aws_s3.AWSAccessKeyId);

	// Read Secret Access Key
	if(fgets(line_buff, sizeof(line_buff), f_aws) == NULL){printf("No Secret Access Key information!"); fclose(f_aws); return false;}
	pos = strchr(line_buff, '\n'); if(pos) *pos = '\0'; pos = strchr(line_buff, '\r'); if(pos) *pos = '\0';
	strcpy(aws_s3.AWSSecretAccessKey, line_buff); printf("[AWS Secret Access Key] %s\n", aws_s3.AWSSecretAccessKey);

	fclose(f_aws);

	FILE *f;
	f = fopen(argv[2], "rb");

	if(f == NULL){
		printf("Target data error");
		return 0;
	}

	task_s3(f, "1", basename(argv[2]));

	fclose(f);

	return 0;
}
