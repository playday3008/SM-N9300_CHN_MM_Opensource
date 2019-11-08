#include "ext4_utils.h"
#ifndef _LARGEFILE64_SOURCE
#define _LARGEFILE64_SOURCE     /* See feature_test_macros(7) */
#endif
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include "ext4.h"
#include "libdmverity_hashgen.h"

#define VERITY_METADATA_MAGIC_NUMBER 0xb001b001

#define	ERR_WRONG_NR_PARAMETER -1
#define	ERR_NO_SUCH_OPERATION -2

//#define   SYSTEM_DEV "/dev/block/platform/msm_sdcc.1/by-name/system"

#if defined(EXYNOS_7420)
static const char * SYSTEM_DEV = "/dev/block/platform/15570000.ufs/by-name/SYSTEM";
#elif defined(EXYNOS_8890)
static const char * SYSTEM_DEV = "/dev/block/platform/155a0000.ufs/by-name/SYSTEM";
#elif TRE_PROJECT
static const char * SYSTEM_DEV = "/dev/block/mmcblk0p18";
#elif defined(EXYNOS_7580) || defined(EXYNOS_3475) || defined(EXYNOS_7870)
static const char * SYSTEM_DEV = "/dev/block/platform/13540000.dwmmc0/by-name/SYSTEM";
#elif defined(EXYNOS_5433) || defined(EXYNOS_5430)
static const char * SYSTEM_DEV = "/dev/block/platform/15540000.dwmmc0/by-name/SYSTEM";
#elif defined(APQ_8084)
static const char * SYSTEM_DEV = "/dev/block/platform/msm_sdcc.1/by-name/system";
#else
static const char * SYSTEM_DEV = "/dev/block/bootdevice/by-name/system";
#endif

#define	TARGET_DEV SYSTEM_DEV
#define	TMP_HASH_FILE "/tmp/dmverity"
#define TMP_HASH_TABLE "/tmp/dmverity_table"
#define DMVERITY_BLOCK_SIZE 4096
#define DMVERITY_META_SIZE (DMVERITY_BLOCK_SIZE*8)

#define META_VERITION  0
#define DM_VERITY_VERSION   1

#define	SHA1 "sha1"
#define	SHA256 "sha256"
#define	MD5 "md5"
#ifdef USE_SHA256
#define HASH_NAME SHA256
#elif USE_SHA1
#define HASH_NAME SHA1
#else
#define HASH_NAME MD5
#endif

struct verity_meta_header {
	unsigned magic_number;
	int protocol_version;
	char signature[256];
	unsigned table_length;
};

int ext4_part_size(const char *blk_device, uint64_t *device_size)
{
	int data_device;
	struct ext4_super_block sb;

	data_device = open(blk_device, O_RDONLY);
	if (data_device < 0) {
		fprintf(stderr, "Error opening block device (%s)", strerror(errno));
		return -1;
	}

	if (1024 != lseek64(data_device, 1024, SEEK_SET)) {
		close(data_device);
		fprintf(stderr, "Error seeking to superblock in %s\n", blk_device);
		return -1;
	}

	if (read(data_device, &sb, sizeof(sb)) != sizeof(sb)) {
		close(data_device);
		fprintf(stderr, "Error reading superblock in %s\n", blk_device);
		return -1;
	}

	ext4_parse_sb(&sb, &info);
	*device_size = info.len;

	close(data_device);
	return 0;
}

void bytes_to_hex(const char * in, char * out, int size) {
    const char * pin = in;
    const char * hex = "0123456789ABCDEF";
    char * pout = out;
    int i = 0;
    for (; i < size - 1; ++i) {
        *pout++ = hex[(*pin >> 4) & 0xF];
        *pout++ = hex[(*pin++) & 0xF];
    }
    *pout++ = hex[(*pin >> 4) & 0xF];
    *pout++ = hex[(*pin) & 0xF];
}

ssize_t hex_to_bytes(const char *string, char * bytes)
{
	char buf[3] = "xx\0", *endp;
	size_t i, len;

	len = strlen(string);
	if (len % 2)
		return -EINVAL;
	len /= 2;

	for (i = 0; i < len; i++) {
		memcpy(buf, &string[i * 2], 2);
		bytes[i] = strtoul(buf, &endp, 16);
		if (endp != &buf[2]) {
			return -EINVAL;
		}
	}
	return i;
}

int nDigits(int i) {
    if (i < 0)
        i = -i;
    if (i < 10)
        return 1;
    if (i < 100)
        return 2;
    if (i < 1000)
        return 3;
    if (i < 10000)
        return 4;
    if (i < 100000)
        return 5;
    if (i < 1000000)
        return 6;
    if (i < 10000000)
        return 7;
    if (i < 100000000)
        return 8;
    return 9;
    /* if (i < 1000000000) */
    /*     return 9; */
    /* if (i < 10000000000) */
    /*     return 10; */
    return -1; //too large!
}

static int verify_verity_header(const char *data_device, const int meta_version, 
			const size_t block_size, char **orig_table, long *data_blocks_ptr)
{
	int fd;
	struct verity_meta_header header;
	char *table;
	int ret = -1;
	uint64_t ext4_size, data_blocks, meta_start;
	loff_t meta_off;

	if (ext4_part_size(data_device, &ext4_size) < 0) {
		fprintf(stderr, "error getting invalid ext4 size");
		return -1;
	} else {
		printf("ext4 size: %lld\n", (long long int) ext4_size);
	}

	data_blocks = ext4_size / block_size;
	meta_start = (data_blocks * block_size + block_size - 1) / block_size;
	meta_off = meta_start * block_size;


	fd = open(data_device, O_RDONLY);
	if (fd < 0){
		fprintf(stderr, "error opening device\n");
		return -1;
	}
	if (lseek64(fd, meta_off, SEEK_SET) < 0) {
		fprintf(stderr, "Error seeking to meta data");
		goto exit_open;
	}

	if (sizeof(struct verity_meta_header)
	        != read(fd, &header, sizeof(struct verity_meta_header))) {
		fprintf(stderr, "Error reading meta data");
		goto exit_open;
	}

	if (header.protocol_version != meta_version) {
		fprintf(stderr, "version mismatch");
		goto exit_open;
	}

	if (header.magic_number != VERITY_METADATA_MAGIC_NUMBER) {
		fprintf(stderr, "wrong magic number");
		goto exit_open;
	}

	if (header.table_length <= 0) {
		fprintf(stderr, "invalid table length");
		goto exit_open;
	}

	table = malloc(header.table_length+1);
	if(NULL == table) {
		fprintf(stderr, "failed to malloc for table\n");
		goto exit_open;
	}
	table[header.table_length] = 0;

	if ((ssize_t)(header.table_length) != read(fd, table, header.table_length)) {
		printf("invalid table length");
		free(table);
		goto exit_open;
	}

	*orig_table = table;
	if (NULL != data_blocks_ptr)
		*data_blocks_ptr = data_blocks;
	ret = 0;

exit_open:
	close(fd);
	return ret;
}

char *get_salt(char *table)
{
	int i = strlen(table);
	char *ptr = table + i - 1;

	/* Salt is last in the table */
	while(*ptr != ' ' && i >= 0) {
		ptr--;
		i--;
	}

	if (i < 0)
		return NULL;

	ptr++;

	return ptr;
}

static int verify_verity(const int meta_version, const int dm_verity_version,
                         const char * data_device, const size_t block_size) {
	int r = -1;
	long data_blocks;
	int fd = -1;
	char *table;
	int ret;
	printf("verify_verity\n");
	fflush(stdout);

	ret = verify_verity_header(data_device, meta_version, block_size, &table, &data_blocks);
	if (0 != ret) {
		fprintf(stderr, "Failed to verify verity header\n");
		return -1;
	}

	printf("table 1: %s\n", table);
    fflush(stdout);
    //
    /////////////now verify hash tree
    char * version_str = strtok(table, " ");
    char * data_dev_str = strtok(NULL, " ");
    char * hash_dev_str = strtok(NULL, " ");
    char * data_blk_size_str = strtok(NULL, " ");
    char * hash_blk_size_str = strtok(NULL, " ");
    char * data_blocks_size_str = strtok(NULL, " ");
    char * hash_start_size_str = strtok(NULL, " ");
    char * alg_str = strtok(NULL, " ");
    char * digest_str = strtok(NULL, " ");
    char * salt_str = strtok(NULL, " ");

    if (dm_verity_version != atoi(version_str)) {
        printf("wrong version in table\n");
        goto exit_malloc;
    }
    if (0 != strcmp(data_dev_str, data_device)) {
        printf("wrong data device in table");
        goto exit_malloc;
    }
    if ((ssize_t)block_size != atoi(data_blk_size_str)) {
        printf("wrong block_size in table\n");
        goto exit_malloc;
    }
    if ((ssize_t)block_size != atoi(hash_blk_size_str)) {
        printf("wrong block_size in table\n");
        goto exit_malloc;
    }
    if (data_blocks != atoi(data_blocks_size_str)) {
        printf("wrong data_blocks in table\n");
        goto exit_malloc;
    }
    if (strlen(digest_str) % 2) {
        printf("wrong digest size in table\n");
        goto exit_malloc;
    }
    if (strlen(salt_str) % 2) {
        printf("wrong salt size in table\n");
        goto exit_malloc;
    }
    int digest_size = strlen(digest_str) / 2;
    int salt_size = strlen(salt_str) / 2;
    char * table_digest = malloc(digest_size);
    char * salt = malloc(salt_size);
    const char * tmp_hash_file = TMP_HASH_FILE;
    char * digest = malloc(digest_size);
    unlink(tmp_hash_file);
    if (hex_to_bytes(salt_str, salt) < 0) {
        printf("wrong salt in table\n");
        goto exit_malloc2;
    }
    if (hex_to_bytes(digest_str, table_digest) < 0) {
        printf("wrong digest in table\n");
        goto exit_malloc2;
    }
    printf("%d %d %x %x\n", digest_size, salt_size, digest, salt);
    fflush(stdout);
    if (VERITY_create_hash(dm_verity_version, alg_str, tmp_hash_file, data_device,
                           block_size, block_size, data_blocks, 0, (unsigned char *)digest,
                           digest_size, (const unsigned char*)salt, salt_size)) {
        printf("error calculation hash\n");
        goto exit_malloc2;
    }
    if (0 != memcmp(digest, table_digest, digest_size)) {
        printf("root hash digest mismatch\n");
        goto exit_malloc2;
    }
#if 0
    const size_t hash_start = atoi(hash_start_size_str);
    const size_t hash_off = hash_start * block_size;
    if (file_cmp(hash_dev_str, tmp_hash_file, hash_off, 0, 1024 * 1024)) {
        goto exit_malloc2;
    }
#endif
    printf("verity verified\n");
    r = 0;
 exit_malloc2: if (table_digest)
        free(table_digest);
    if (salt)
        free(salt);
    if (digest)
        free(digest);
 exit_malloc: if (table)
        free(table);
 exit_open: if (fd >= 0)
        close(fd);
    unlink(tmp_hash_file);
    return r;
}

static int generate_salt(char * salt, int salt_len) {
    printf("generate random salt.\n");
    FILE *fp;
    int r;
    int amount_read = 0;
    fp=fopen("/dev/urandom", "r");
    if (fp) {
        while (amount_read < salt_len) {
            r = (int)fread(salt + amount_read, 1, salt_len-amount_read, fp);
            if (r > 0) { amount_read += r; }
            else if (!r) { break; }
            else if (errno != EINTR) {
                amount_read = -1;
                break;
            }
        }
        //fread(salt, 1, salt_len, fp);
        fclose(fp);
        //printf("salt2 is generated: %s. amount_read is %d.\n", salt, amount_read);
    } else {
        printf("fopen failed.\n");
    }
    return amount_read;
}


static int rehash_verity(const int meta_version, const int dm_verity_version,
                         const char * data_device, const size_t block_size)
{
	#if 0
    int r = -1;
	char *table, *salt = NULL;

	salt = malloc(32); /* biggest length */
	if (NULL == salt) {
		fprintf(stderr, "Failed to malloc mem for salt\n");
		return -1;
	}
    #endif
    
    int ret = 0;
	  
	uint64_t part_size;
	if(ext4_part_size(TARGET_DEV, &part_size)) {
        printf("failed to get part size.\n");
        return -1;
    }

    const char * tmp_hash_file = TMP_HASH_FILE;
    const char * tmp_hash_table = TMP_HASH_TABLE;
    //unlink(tmp_hash_file);
    //unlink(tmp_hash_table);

    const char * hash_name = HASH_NAME;
    int i;
    char * table = NULL, *p;
    const long data_blocks = part_size / DMVERITY_BLOCK_SIZE;
#ifdef USE_SHA256
    const int digest_size = 32;//sha256
#elif USE_SHA1
    const int digest_size = 20;//sha1
#else
    const int digest_size = 16;//md5
#endif
    char salt[digest_size];
    char root_hash[digest_size];
    const loff_t hash_start = (part_size + DMVERITY_META_SIZE)/DMVERITY_BLOCK_SIZE;
    const loff_t hash_position = DMVERITY_META_SIZE/DMVERITY_BLOCK_SIZE;

    struct verity_meta_header meta_header;
    FILE * fp;

    //0.1 generate random salt
    /* generate_salt(salt); */
    if (generate_salt(salt, digest_size) != digest_size) {
        printf("Error generating random salt.\n");
        // how to handle it better? now it will use whatever compiler gives: all 0's
    }
    
    // 1. generate hash, table and write to a tmp hash file
    if(VERITY_create_hash(dm_verity_version,                  \
                      hash_name,                              \
                      TMP_HASH_FILE,                          \
                      data_device,                             \
                      block_size,                    \
                      block_size,                    \
                      data_blocks,                            \
                      hash_position,                          \
                      (unsigned char *)root_hash,             \
                      digest_size,                            \
                      (const unsigned char *)salt,            \
                      digest_size))
    {
        printf("failed to create hash tree\n");
        ret = -1;
        goto rehash_out;
    }  


    int table_size = nDigits(dm_verity_version) + 1 + strlen(TARGET_DEV) + 1
        + strlen(TARGET_DEV) + 1 + nDigits(DMVERITY_BLOCK_SIZE) + 1
        + nDigits(DMVERITY_BLOCK_SIZE) + 1 + nDigits(data_blocks) + 1
        + nDigits(hash_start) + 1 + strlen(hash_name) + 1 + digest_size * 2
        + 1 + digest_size * 2 + 1;
    table = malloc(table_size);
    if(NULL == table){
        printf("malloc failed\n");
        ret = -1;
        goto rehash_out;
    }
    table[table_size-1] = 0;
    i = sprintf(table, "%d %s %s %lld %lld %lld %lld %s ", dm_verity_version, TARGET_DEV,
                TARGET_DEV, (long long int) DMVERITY_BLOCK_SIZE,
                (long long int) DMVERITY_BLOCK_SIZE, (long long int) data_blocks,
                (long long int) hash_start, hash_name);
    if(i <= 0){
        printf("sprintf error");
        free(table);
        ret = -1;
        goto rehash_out;
    }
    //printf("Table: %s\n", table);
    p = table + i;
    bytes_to_hex(root_hash, p, digest_size);
    p += digest_size * 2;
    p += sprintf(p, " ");
    bytes_to_hex(salt, p, digest_size);
    printf("table: %s", table);
    
    // 2.1 generate meta_header
    meta_header.magic_number = VERITY_METADATA_MAGIC_NUMBER;
    meta_header.protocol_version = 0;
    meta_header.table_length = strlen(table);//not including trailing NULL
    memset(&meta_header.signature, 0, sizeof(meta_header.signature));
    //tmp_hash_file it should have been created by generate_dm_verity_hash already.
    
    // 2.2 write table and meta_header to tmp hash file
    fp = fopen(tmp_hash_file, "r+");
    if (NULL == fp) {
        printf("failed to open temp file\n");
        ret = -1;
        goto rehash_out;
    }
    if(1 != fwrite(&meta_header, sizeof(struct verity_meta_header), 1, fp)){
        printf("failed to write temp file\n");
        fclose(fp);
        ret = -1;
        goto rehash_out;
    }
    printf("write meta_header %d\n", sizeof(struct verity_meta_header));
    if(1 != fwrite(table, meta_header.table_length+1, 1, fp)){
        printf("failed to write temp file\n");
        fclose(fp);
        ret = -1;
        goto rehash_out;
    }
    printf("write table %d\n", meta_header.table_length+1);
    fflush(fp);
    fsync(fileno(fp));
    fclose(fp);

    // 2.3 write table  to tmp hash meta table file
    fp = fopen(tmp_hash_table, "w");
    if (NULL == fp) {
        printf("failed to open temp meta table file\n");
        ret = -1;
        goto rehash_out;
    }
    
    if(1 != fwrite(table, meta_header.table_length+1, 1, fp)){
        printf("failed to write temp meta table file\n");
        fclose(fp);
        ret = -1;
        goto rehash_out;
    }
    printf("write table %d\n", meta_header.table_length+1);
    fflush(fp);
    fsync(fileno(fp));
    fclose(fp);
    
    // 3. write tmp hash file to the /system (not signed)
    /*
    if(file_to_device(tmp_hash_file, TARGET_DEV, 1024*1024, part_size)){
        printf("failed to write hash 001\n");
        goto rehash_out;
    }
    */
rehash_out:
    //unlink(tmp_hash_file);
    if(table)
        free(table);
	return ret;
}

int main(int argc, char *argv[])
{
	const int dm_verity_version = 1;
	if (argc != 2)
		return ERR_WRONG_NR_PARAMETER;

	if (!strcmp(argv[1], "verify")) {
		return verify_verity(0, 1, TARGET_DEV, DMVERITY_BLOCK_SIZE);
	} else if (!strcmp(argv[1], "rehash")) {
		return rehash_verity(0, 1, TARGET_DEV, DMVERITY_BLOCK_SIZE);
	} else {
		return ERR_NO_SUCH_OPERATION;
	}
	return 0;
}
