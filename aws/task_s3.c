/*
 * task_s3.c
 *
 *  Created on: Sep 30, 2019
 *      Author: yj
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "netinet/tcp.h"
#include "sys/types.h"
#include "sys/socket.h"
#include "netinet/in.h"
#include "netdb.h"
#include "arpa/inet.h"


#include "task_s3.h"
#include "utils_s3.h"

#define calc_partLength(X,Y) 	(81 + strlen(X) + strlen(Y))
#define send_fail_return(x, s)		if((x) < 0){ printf("... socket send failed"); close(s); return SOCKET_WRITE_FAIL; }

AWS_S3 aws_s3;

static const char boundary[33] = "4a559d2862a2473abb5cbb2a514f6cc5";
static char partBody[256];
static char request_buff[1024];

ssize_t writePart(int s, char* name, const char* data){

	int str_pointer = 0;
	ssize_t t;

	strcpy(partBody, "--4a559d2862a2473abb5cbb2a514f6cc5\r\nContent-Disposition: form-data; name=\""); str_pointer += strlen(partBody);
	strcpy(partBody + str_pointer, name); str_pointer += strlen(name);
	strcpy(partBody + str_pointer, "\"\r\n\r\n");

	t = write(s, partBody, strlen(partBody)); if(t < 0) return t;

	t = write(s, data, strlen(data)); if(t < 0) return t;

	t = write(s, "\r\n", 2);

	return t;

}

#define READ_BUFFER_SIZE 1024
static uint8_t line_read[READ_BUFFER_SIZE];
ssize_t writeDataPart(int s, char* filename, int filesize, FILE* f){

	int str_pointer = 0;
	int file_1percent = (int)((float)filesize / 100.0f);
	ssize_t t;

	strcpy(partBody, "--4a559d2862a2473abb5cbb2a514f6cc5\r\nContent-Disposition: form-data; name=\"file\"; filename=\""); str_pointer += strlen(partBody);
	strcpy(partBody + str_pointer, filename); str_pointer += strlen(filename);
	strcpy(partBody + str_pointer, "\"\r\nContent-Type: text/plain\r\n\r\n");

	t = write(s, partBody, strlen(partBody)); if(t < 0) return t;

	int count = 0;
	int send_count = 0; int xx = 0;


	while(feof(f) == 0){
		count = fread(line_read, sizeof(uint8_t), READ_BUFFER_SIZE, f);
		t = send(s, (void *)line_read, count, 0); if(t < 0) return t;
		send_count += count;


		if(send_count > file_1percent){
			if((xx /10) % 2) putchar('*');
			else putchar('o');
			xx ++;
			send_count -= file_1percent;
		}

	}

	t = write(s, "\r\n", 2);

	return t;
}

ssize_t writeEndBoundary(int s){
	char x[37];
	strcpy(x, "--");
	strcpy(x + 2, boundary);
	strcpy(x + 34, "--");
	return write(s, x, strlen(x));
}

AWS_S3_REQ task_s3(FILE* f, char* key, char* filename){

	/***************************************************************************/
	// Local variables
	/***************************************************************************/
	int str_pointer = 0;
	int content_length = 0;
	int file_size = 0;

	struct hostent *hp;
	struct sockaddr_in addr;
	int sock;
	int on = 1;

	setbuf(stdout, NULL);

	// https://gist.github.com/nolim1t/126991
	/***************************************************************************/
	// Socket Connect
	/***************************************************************************/
	createS3Signature(key, filename);

	if((hp = gethostbyname(aws_s3.url)) == NULL){
		herror("gethostbyname");
		exit(1);
	}
	printf("IP Address : %s\n", inet_ntoa(*(struct in_addr*)(hp->h_addr_list[0])));
	bcopy(hp->h_addr_list[0], &addr.sin_addr, hp->h_length);
	addr.sin_port = htons(80);
	addr.sin_family = AF_INET;
	sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (const char *)&on, sizeof(int));

	if(sock == -1){
		perror("setsockopt");
		exit(1);
	}

	if(connect(sock, (struct sockaddr *)&addr, sizeof(struct sockaddr)) == -1){
		perror("connect");
		exit(1);
	}

	/***************************************************************************/
    // Initialize policy & signature
	/***************************************************************************/
	// Calculate Content-Length
    content_length = 0;
	content_length = calc_partLength("key", aws_s3.key);
	content_length += calc_partLength("acl", "public-read");
	content_length += calc_partLength("X-Amz-Algorithm", "AWS4-HMAC-SHA256");
	content_length += calc_partLength("X-Amz-Credential", aws_s3.AWSCredential);
	content_length += calc_partLength("X-Amz-Date", aws_s3.AWSDate);
	content_length += calc_partLength("Policy", aws_s3.policy_base64);
	content_length += calc_partLength("X-Amz-Signature", aws_s3.signature);
	content_length += 122 + strlen(filename);


	printf("[Upload] %-15s ", filename);
	if(fseek(f, 0L ,SEEK_END) != 0){
		printf("| Fail to check the file size\r\n");
		return FILE_FSEEK_FAIL;
	}

	file_size = ftell(f);

	content_length += file_size;
	content_length += 38;

	fseek(f, 0L, SEEK_SET);

	printf("(%7d bytes) | ", file_size);
	/***************************************************************************/
    // Send HTTP
	/***************************************************************************/

	// Write Header
	strcpy(request_buff, "POST / HTTP/1.1\r\n"); str_pointer += 17;
	strcpy(request_buff + str_pointer, "Host: "); str_pointer += 6;
	strcpy(request_buff + str_pointer, aws_s3.url); str_pointer += strlen(aws_s3.url);
	strcpy(request_buff + str_pointer, "\r\nAccept-Encoding: identity\r\n"); str_pointer += 29;
	strcpy(request_buff + str_pointer, "Content-Type: multipart/form-data; boundary="); str_pointer += 44;
	strcpy(request_buff + str_pointer, boundary); str_pointer += 32;
	strcpy(request_buff + str_pointer, "\r\nContent-Length: "); str_pointer += 18;
	sprintf(request_buff + str_pointer, "%d\r\n\r\n", content_length);


	send_fail_return(write(sock, request_buff, strlen(request_buff)), sock);

	// Send data
	send_fail_return(writePart(sock, "key", aws_s3.key), sock);
	send_fail_return(writePart(sock, "acl", "public-read"), sock);
	send_fail_return(writePart(sock, "X-Amz-Algorithm", "AWS4-HMAC-SHA256"), sock);
	send_fail_return(writePart(sock, "X-Amz-Credential", aws_s3.AWSCredential), sock);
	send_fail_return(writePart(sock, "X-Amz-Date", aws_s3.AWSDate), sock);
	send_fail_return(writePart(sock, "Policy", aws_s3.policy_base64), sock);
	send_fail_return(writePart(sock, "X-Amz-Signature", aws_s3.signature), sock);

	// Send file
	send_fail_return(writeDataPart(sock, filename, file_size, f), sock);


	// End
	send_fail_return(writeEndBoundary(sock), sock);

	/***************************************************************************/
	// HTTP response
	/***************************************************************************/

	struct timeval receiving_timeout;
	char recv_buf[64];
	char header_code[5];

	receiving_timeout.tv_sec = 5;
	receiving_timeout.tv_usec = 0;
	if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &receiving_timeout,
			sizeof(receiving_timeout)) < 0) {
		//ESP_LOGE(TAG, "... failed to set socket receiving timeout");
	    close(sock);
	    return AWS_RECEIVE_TIMEOUT;
	}



	// Read Header
	bzero(recv_buf, sizeof(recv_buf));

	read(sock, recv_buf, sizeof(recv_buf)-1);
	strncpy(header_code, recv_buf + 9, 3);
	header_code[3] = '\0';

	if(header_code[0] != '2'){
		close(sock);
		printf(" | Fail. (%s)\r\n", header_code);
		return AWS_WRITE_FAIL;
	}else{
		close(sock);
		printf(" | Success. (%s)\r\n", header_code);
		return AWS_WRITE_SUCCESS;
	}


/*
	int r;
	do {
	            bzero(recv_buf, sizeof(recv_buf));
	            r = read(sock, recv_buf, sizeof(recv_buf)-1);
	            for(int i = 0; i < r; i++) {
	                putchar(recv_buf[i]);
	            }
	        } while(r > 0);

	return AWS_WRITE_FAIL;
*/
}
