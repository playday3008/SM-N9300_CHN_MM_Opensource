#ifndef __LIB_DMVERITY_HASH_GEN___
#define __LIB_DMVERITY_HASH_GEN___
int VERITY_create_hash(int version, const char *hash_name,
		       const char *hash_device, const char *data_device,
		       size_t hash_block_size, size_t data_block_size, loff_t data_blocks,
		       loff_t hash_position, unsigned char *root_hash, size_t digest_size,
		       const unsigned char *salt, size_t salt_size);
#endif
