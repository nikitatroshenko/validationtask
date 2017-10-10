#include "err_handling.h"
#include <cstdio>
#include <cerrno>
#include <cstdlib>
#include <openssl/md5.h>
#include <boost/crc.hpp>
#include <boost/regex.hpp>

#define ARG_CRC32 "-crc32"
#define ARG_MD5 "-md5"

#define DEFAULT_FILE_BUF_SIZE 512

static int get_file_md5(int argc, char *argv[]);
static int get_file_crc32(int argc, char *argv[]);
static int validate_args(int argc, char *argv[]);
static unsigned char parse_hex_chars(char h, char l);

int main(int argc, char *argv[])
{
	int rc = validate_args(argc, argv);

	if (rc) {
		log_error();
	} else {
		printf("success\n");
	}

	return rc;
}

unsigned char parse_hex_chars(char h, char l) {
	char byte[3] {h, l, '\0'};
	return static_cast<char>(strtoul(byte, nullptr, 16));
}

int get_file_md5(int argc, char **argv)
{
	MD5_CTX context{};
	size_t bytes;
	char buf[DEFAULT_FILE_BUF_SIZE];
	unsigned char digest[MD5_DIGEST_LENGTH]{};
	FILE *in = fopen(argv[3], "rb");
	const char *hash = argv[1];

	if (in == nullptr) {
		return errno;
	}

	std::string hash_str(hash);
	boost::regex pattern("^[0-9a-fA-F]{32}$");

	if (!boost::regex_match(hash_str, pattern)) {
		errno = EBADMD5;
		return errno;
	}

	MD5_Init(&context);

	bytes = fread(buf, sizeof *buf, DEFAULT_FILE_BUF_SIZE, in);

	while (bytes > 0) {
		MD5_Update(&context, buf, bytes);
		bytes = fread(buf, sizeof *buf, DEFAULT_FILE_BUF_SIZE, in);
	}

	MD5_Final(digest, &context);

	for (size_t i = 0; i < 2 * MD5_DIGEST_LENGTH; i += 2) {
		unsigned char next_byte = parse_hex_chars(hash[i], hash[i + 1]);

		if (next_byte != digest[i / 2]) {
			errno = EHASHMISMATCH;
			return errno;
		}
	}

	return EXIT_SUCCESS;
}

int get_file_crc32(int argc, char **argv)
{
	boost::crc_32_type crc;
	size_t bytes;
	char buf[DEFAULT_FILE_BUF_SIZE];
	FILE *in = fopen(argv[3], "rb");
	const char *hash = argv[1];
	unsigned long crc_sum;
	unsigned long expected_crc;

	if (in == nullptr) {
		return errno;
	}

	std::string hash_str(hash);
	boost::regex regex("^[0-9a-fA-F]{8}$");

	if (!boost::regex_match(hash_str, regex)) {
		errno = EBADCRC32;
		return errno;
	}

	bytes = fread(buf, sizeof *buf, DEFAULT_FILE_BUF_SIZE, in);
	while (bytes > 0) {
		crc.process_bytes(buf, bytes);
		bytes = fread(buf, sizeof *buf, DEFAULT_FILE_BUF_SIZE, in);
	}

	crc_sum = crc.checksum();
	expected_crc = strtoul(hash, nullptr, 16);

	if (crc_sum != expected_crc) {
		errno = EHASHMISMATCH;
		return errno;
	}

	return EXIT_SUCCESS;
}

int validate_args(int argc, char **argv) {
	if (argc < 4) {
		errno = EBADARGS;
		return errno;
	}

	std::string method_str = std::string(argv[2]);

	if (!strncmp(argv[2], ARG_MD5, strlen(ARG_MD5))) {
		return get_file_md5(argc, argv);
	} else if (!strncmp(argv[2], ARG_CRC32, strlen(ARG_CRC32))) {
		return get_file_crc32(argc, argv);
	} else {
		errno = EINVAL;
		return errno;
	}
}
