/*
 * dm-verity volume handling
 *
 * Copyright (C) 2012, Red Hat, Inc. All rights reserved.
 *
 * This file is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this file; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifdef _USE_LFS
#define _LARGEFILE64_SOURCE
#define _FILE_OFFSET_BITS 64
#endif
#include <sys/types.h> 
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <inttypes.h>
#include <time.h>
#include <stdlib.h>
#include <errno.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/mount.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <openssl/md5.h>
#include <openssl/sha.h>
#include <openssl/rsa.h>
#include "libdmverity_hashgen.h"

#include <pthread.h>

#define VERBOSE 1
#define SHA1_NAME "sha1"
#define SHA256_NAME "sha256"
#define MD5_NAME "md5"

#define VERITY_MAX_LEVELS	63
#ifdef VERBOSE
extern void bytes_to_hex(const char * in, char * out, int size);
#endif

int blk_zero_handle = 0;  //flag if blk 0 is handled, 0 means NO

static unsigned get_bits_up(size_t u) {
    unsigned i = 0;
    while ((1U << i) < u)
        i++;
    return i;
}

static unsigned get_bits_down(size_t u) {
    unsigned i = 0;
    while ((u >> i) > 1U)
        i++;
    return i;
}

static int mult_overflow(loff_t *u, loff_t b, size_t size) {
    *u = (uint64_t) b * size;
    if ((loff_t) (*u / size) != b || (loff_t) *u < 0)
        return 1;
    return 0;
}

static int verify_hash_block(const char *hash_name, int version, char *hash,
                             size_t hash_size, const char *data, size_t data_size,
                             const unsigned char *salt, size_t salt_size) {
    if(0 == strcmp(hash_name, SHA256_NAME)){
        SHA256_CTX ctx;	
        SHA256_Init(&ctx);
        if (version == 1) {
            SHA256_Update(&ctx, salt, salt_size);
            SHA256_Update(&ctx, data, data_size);
        } else if (version == 0) {
            SHA256_Update(&ctx, data, data_size);
            SHA256_Update(&ctx, salt, salt_size);
        } else {
            printf("wrong version number %d\n", version);
            goto out;
        }
        SHA256_Final(hash, &ctx);
    }else if(0 == strcmp(hash_name, SHA1_NAME)){
        SHA_CTX ctx;
	
        SHA1_Init(&ctx);

        if (version == 1) {
            SHA1_Update(&ctx, salt, salt_size);
            SHA1_Update(&ctx, data, data_size);
        } else if (version == 0) {
            SHA1_Update(&ctx, data, data_size);
            SHA1_Update(&ctx, salt, salt_size);
        } else {
            printf("wrong version number %d\n", version);
            goto out;
        }
        SHA1_Final(hash, &ctx);
    }else if(0 == strcmp(hash_name, MD5_NAME)){
        MD5_CTX ctx;
    
        MD5_Init(&ctx);

        if (version == 1) {
            MD5_Update(&ctx, salt, salt_size);
            MD5_Update(&ctx, data, data_size);
        } else if (version == 0) {
            MD5_Update(&ctx, data, data_size);
            MD5_Update(&ctx, salt, salt_size);
        } else {
            printf("wrong version number %d\n", version);
            goto out;
        }
        MD5_Final(hash, &ctx);
    }else
        return -1;
 out: 
    return 0;
}


#ifdef PARALLEL_HASH  

#define READ_LUM_SIZE 1024  /*number of data blocks read a time*/

typedef struct thread_info_s{
	pthread_t thread_id;
	int       thread_num; 
	char      *hash_buffer;
	char      *data_device; 
	size_t    data_block_size; 
	loff_t   data_block_offset; /* offset from the beginning of this device  */
	loff_t    data_block_count; 
	int       version; 
	size_t    digest_size; 
    unsigned char *salt; 
	size_t    salt_size; 
	char      *hash_name; 
}thread_info;


static void* hash_create_job(void *args){
	thread_info *tinfo = (thread_info *)args; 
	char *data_device = tinfo->data_device; 
	size_t  digest_size = tinfo->digest_size; 
	unsigned char *salt = tinfo->salt; 
	ssize_t  salt_size = tinfo-> salt_size; 
	char *hash_name = tinfo->hash_name; 
	size_t  data_block_size =  tinfo->data_block_size; 
	char    *data_buffer = NULL;
	char    *hash_buffer = tinfo->hash_buffer; 
	int     version =  tinfo->version;
	loff_t  data_block_offset = tinfo->data_block_offset; 
	loff_t  blocks_to_calc  = tinfo->data_block_count;  /* this is blocks need to be calculated in this thread */
	loff_t  buffer_offset =  data_block_offset * digest_size; 
	char    *pwrite =  &hash_buffer[buffer_offset];
	off64_t  seek_rd;
	int fd; 
	int i;
	int read_size;


	data_buffer = (char *)malloc( sizeof(char)* data_block_size * READ_LUM_SIZE); 
	if( NULL == data_buffer ) {
		printf("Cannot allocate data buffer in thread %d \n", tinfo->thread_num); 
		exit(EXIT_FAILURE); 
	}

	fd = open(data_device, O_RDONLY);
	if( fd == -1) {
        printf("Cannot open device %s.\n", data_device);
		free(data_buffer); 
        exit(EXIT_FAILURE); 
	}
  
    if (mult_overflow(&seek_rd, data_block_offset, data_block_size) ) {
        printf("Device offset overflow 1 : %lld, %d \n", (long long int)data_block_offset, (int)data_block_size);
        free(data_buffer);
		close(fd); 
		exit(EXIT_FAILURE); 
    }

    if (lseek64(fd, seek_rd, SEEK_SET) < 0) {
        printf("Cannot seek to requested position in data device in thread %d, offset %ld, seek_rd %lld\n", tinfo->thread_num,  (long)data_block_offset, (long long int)seek_rd);
        free(data_buffer);
		close(fd); 
		exit(EXIT_FAILURE); 
    }

	while(blocks_to_calc){

		if (blocks_to_calc >= READ_LUM_SIZE) {
			read_size = READ_LUM_SIZE;
			blocks_to_calc -= READ_LUM_SIZE;
		} else {
			read_size = blocks_to_calc;
			blocks_to_calc = 0;
		}

		if( read(fd, data_buffer, read_size * data_block_size) < 0  ){ 
			printf("Cannot read data device block.");
			free(data_buffer);
			close(fd); 
			exit(EXIT_FAILURE);
		}
        for (i=0; i< read_size; i++) {
		    if(verify_hash_block(hash_name, version, pwrite,  
		    					 digest_size, data_buffer+ i*data_block_size, data_block_size, salt, salt_size)){
		    	printf("Failed to calculate hash!\n");
		    	free(data_buffer);
		    	close(fd); 
		    	exit(EXIT_FAILURE); 
		    }

		    if ((0 == tinfo->thread_num) && (0 == i) && (0 == blk_zero_handle)) {
        			// we are hashing data block 0, use dummy hash, 
		    	    printf ("generate dummy hash for block 0.\n");
        			memset(pwrite, 1, digest_size);
        			blk_zero_handle = 1;  
        	}
        	
		    pwrite += digest_size;  
        }

	}

	close(fd); 
	free(data_buffer);
	return NULL;

 }

static int get_max_thread_num()
{
 #define NTHREADS 4
	 char core_num;
	 int m_thread;
	 int fd;
	 /* Get Max cpu_num from kernel side */
	 fd = open("/sys/devices/system/cpu/kernel_max", O_RDONLY);

	 if (fd < 0) {
		 fprintf(stderr, "Failed to open sys node\n");
		 /* Default Thread number = 4 */	
		 return NTHREADS;
	 }
	 read(fd, &core_num, sizeof(core_num));
	 close(fd);
	 m_thread = atoi(&core_num) + 1;
	 printf("core_num = %c thread_num = %d\n", core_num, m_thread);
	 return m_thread;
}


 /* Only calcuate the level0 block on disk using thread to speed up  */
 static int create_hash_level0 (const char *data_device , FILE *wr, 
								size_t data_block_size, loff_t hash_block_offset, size_t hash_block_size,
								loff_t data_block_count, int version, const char *hash_name,
								size_t digest_size, const unsigned char *salt, size_t salt_size) {

 /*4 threads is the maximum thread number we can use for now, otherwise integer overflow in fseeko, the seek will fail. And we don't have 64bit support fseeko here */
	 char left_block[hash_block_size];
	 size_t hash_per_block = ((size_t)1 << get_bits_down(hash_block_size / digest_size));
	 size_t digest_size_full = ((size_t)1 << get_bits_up(digest_size));
	 loff_t blocks_to_write = (data_block_count  + hash_per_block - 1) / hash_per_block;
	 loff_t seek_wr;
	 size_t left_bytes;
	 unsigned i;
	 char  *hash_buffer = NULL; 
	 size_t buffer_size;
	 thread_info *tinfo = NULL; 
	 loff_t block_batch ;
	 loff_t last_block; 
	 char *calculated_digest = NULL; 
	 char *writeback ;
	 size_t total_write; 
	 char hex_hash[128];
	 int n_thread;
	 memset(hex_hash, 0, 128);

	 n_thread = get_max_thread_num();

	 /* allocate memory space for hash value written by threads */ 
	 buffer_size =  data_block_count * digest_size * sizeof(char);
	 hash_buffer = (char*) malloc(buffer_size);

	 if(NULL == hash_buffer) {
		 printf( "Cannot allocate hash buffer for %lld blocks * %zu bytes\n", (long long int)data_block_count, digest_size); 
		 return -1 ; 
	 }

	 memset(hash_buffer, 0 , buffer_size); 

	 tinfo = (thread_info *) malloc( n_thread *sizeof(struct thread_info_s)) ; 
	 if( NULL == tinfo ){
		 printf( "Cannot allocat thread_info memory\n"); 
		 free(hash_buffer); 
		 return -1;  
	 }
	 memset(tinfo, 0 , n_thread * sizeof( struct thread_info_s) );
	 
	 /*2: generate threads to calcuate hash values and save on hash buffer  
	  * all calculated digest will be saved in continous memory buffer */
	 block_batch = data_block_count / n_thread; 
	 last_block = data_block_count - block_batch * (n_thread-1); 

	 for( i = 0; i< n_thread; i++){
		 tinfo[i].thread_num = i;  /* custom specific thread id */ 
		 tinfo[i].data_device = (char *)data_device; 
		 tinfo[i].data_block_size = data_block_size; 
		 tinfo[i].data_block_offset = i * block_batch; 

		 if ( i == (n_thread-1)) tinfo[i].data_block_count = last_block;
		 else  tinfo[i].data_block_count = block_batch; 
	
		 tinfo[i].version = version;
		 tinfo[i].digest_size = digest_size; 
		 tinfo[i].salt = (unsigned char *)salt; 
		 tinfo[i].salt_size = salt_size; 
		 tinfo[i].hash_name = (char *)hash_name; 
		 tinfo[i].hash_buffer = hash_buffer; 

		 if( pthread_create(&tinfo[i].thread_id,  NULL,  hash_create_job, &tinfo[i]) != 0){
			 unsigned j ;
			 printf("Failed to create hash thread. \n");
			 for( j = 0 ; j < i ; j++){
				 pthread_join(tinfo[j].thread_id, NULL); 
			 }
			 free(hash_buffer); 
			 free(tinfo);
			 return -EAGAIN; 
		 } 
	 }

	 for(i = 0;  i< n_thread; i++){
		 pthread_join(tinfo[i].thread_id, NULL);  
	 }
	 free(tinfo); 
 
	 printf("Finish level0 blocks !\n");

	 /*3: write hash buffer back to disk */ 
	 if( mult_overflow(&seek_wr, hash_block_offset, hash_block_size)) {
 		 printf("Device offset overflow : %lld, %d\n", (long long int)hash_block_offset, (int)hash_block_size);
 		 free(hash_buffer);
		 return -EINVAL;
	 }

	 if (fseeko(wr, seek_wr, SEEK_SET)) {
		 printf("Cannot seek to requested position in hash device.");
 		 free(hash_buffer);		 
		 return -EIO;
	 }
	 
	 memset(left_block, 0, hash_block_size);

	 writeback = hash_buffer; 
	 total_write = 0 ;
	 /*  back to create_hash procedure */ 
	 while (blocks_to_write--) {
		 left_bytes = hash_block_size;
		 for (i = 0; i < hash_per_block; i++) {
			 if (!data_block_count)
				 break;
			 data_block_count--;
		
			 calculated_digest = writeback;
			 writeback += digest_size; 
			 total_write += digest_size; 

			 if (fwrite(calculated_digest, digest_size, 1, wr) != 1) {
				 printf("Cannot write digest to hash device.");
				 free(hash_buffer); 
				 return -EIO;
			 }
			 
			 if (version == 0) {
				 left_bytes -= digest_size;
			 } else {
				 if (digest_size_full - digest_size) {
					 if (fwrite(left_block, digest_size_full - digest_size, 1, wr) != 1) {
						 printf("Cannot write spare area to hash device.");
						 free(hash_buffer);
						 return -EIO;
					 }
				 }
				 left_bytes -= digest_size_full;
			 }
		 }
		 if (left_bytes) {
			 total_write += left_bytes; 
			 if (fwrite(left_block, left_bytes, 1, wr) != 1) {
				 printf("Cannot write remaining spare area to hash device.");
				 free(hash_buffer);
				 return -EIO;
			 }
		 }
	 }

	 bytes_to_hex(calculated_digest, hex_hash, digest_size);

	 free(hash_buffer);
	 return 0; 

}
#endif


 static int create_hash(FILE *rd, FILE *wr, loff_t data_block,
						size_t data_block_size, loff_t hash_block, size_t hash_block_size,
						loff_t blocks, int version, const char *hash_name,
						char *calculated_digest, size_t digest_size, const unsigned char *salt,
						size_t salt_size) {
	 char left_block[hash_block_size];
	 char data_buffer[data_block_size];
	 //  char read_digest[digest_size];
	 size_t hash_per_block = ((size_t)1 << get_bits_down(hash_block_size / digest_size));
	 size_t digest_size_full = ((size_t)1 << get_bits_up(digest_size));
	 loff_t blocks_to_write = (blocks + hash_per_block - 1) / hash_per_block;
	 loff_t seek_rd, seek_wr;
	 size_t left_bytes;
	 unsigned i;
	

 #ifdef VERBOSE
	 char hex_hash[128];
	 printf("create_hash: %lld, %lld, %lld\n", (long long int)data_block, (long long int)hash_block, (long long int)blocks);
	 memset(hex_hash, 0, 128);
 #endif

	 if (mult_overflow(&seek_rd, data_block, data_block_size)
		 || mult_overflow(&seek_wr, hash_block, hash_block_size)) {
		 printf("Device offset overflow 1 : %lld, %d, %lld, %d\n", (long long int)data_block, (int)data_block_size, (long long int)hash_block, (int)hash_block_size);
		 return -EINVAL;
	 }

	 if (fseeko(rd, seek_rd, SEEK_SET)) {
		 printf("Cannot seek to requested position in data device.");
		 return -EIO;
	 }

	 if (wr && fseeko(wr, seek_wr, SEEK_SET)) {
		 printf("Cannot seek to requested position in hash device.");
		 return -EIO;
	 }

	 memset(left_block, 0, hash_block_size);
	 while (blocks_to_write--) {
		 left_bytes = hash_block_size;
		 for (i = 0; i < hash_per_block; i++) {
			 if (!blocks)
				 break;
			 blocks--;
			 if (fread(data_buffer, data_block_size, 1, rd) != 1) {
				 printf("Cannot read data device block.");
				 return -EIO;
			 }
			 if (verify_hash_block(hash_name, version, calculated_digest,
								   digest_size, data_buffer, data_block_size, salt, salt_size))
				 return -EINVAL;
			 if (0 == blk_zero_handle) {
				// we are hashing data block 0, use dummy hash, 
				printf ("generate dummy hash for block 0.\n");
				memset(calculated_digest, 1, digest_size);
				blk_zero_handle = 1;  
			 }

			 if (!wr)
				 break;

			 if (fwrite(calculated_digest, digest_size, 1, wr) != 1) {
				 printf("Cannot write digest to hash device.");
				 return -EIO;
			 }
			 if (version == 0) {
				 left_bytes -= digest_size;
			 } else {
				 if (digest_size_full - digest_size) {
					 if (fwrite(left_block, digest_size_full - digest_size, 1,
								wr) != 1) {
						 printf("Cannot write spare area to hash device.");
						 return -EIO;
					 }
				 }
				 left_bytes -= digest_size_full;
			 }
		 }
		 if (wr && left_bytes) {
			 if (fwrite(left_block, left_bytes, 1, wr) != 1) {
				 printf("Cannot write remaining spare area to hash device.");
				 return -EIO;
			 }
		 }
	 }
 #ifdef VERBOSE
	 bytes_to_hex(calculated_digest, hex_hash, digest_size);
	 printf("calculated_digest: %s \n", hex_hash);
 #endif
	 return 0;
 }
 
 int VERITY_create_hash(int version, const char *hash_name,
						const char *hash_device, const char *data_device,
						size_t hash_block_size, size_t data_block_size, loff_t data_blocks,
						loff_t hash_position, unsigned char *root_hash, size_t digest_size,
						const unsigned char *salt, size_t salt_size) {
	 char calculated_digest[32];
	 FILE *data_file = NULL;
	 FILE *hash_file = NULL, *hash_file_2;
	 loff_t hash_level_block[VERITY_MAX_LEVELS];
	 loff_t hash_level_size[VERITY_MAX_LEVELS];
	 loff_t data_file_blocks, s;
	 size_t hash_per_block_bits;
	 loff_t data_device_size = 0, hash_device_size = 0;
	 int levels, i, r;

	 if (data_blocks < 0 || hash_position < 0) {
		 printf("Invalid size parameters for verity device.\n");
		 return -EINVAL;
	 }
	 data_file_blocks = data_blocks;

	 if (mult_overflow(&data_device_size, data_blocks, data_block_size)) {
		 printf("Device offset overflow 2 : %lld, %d\n", (long long int)data_blocks, (int)data_block_size);
		 return -EINVAL;
	 }

	 hash_per_block_bits = get_bits_down(hash_block_size / digest_size);
	 if (!hash_per_block_bits)
		 return -EINVAL;

	 levels = 0;
	 if (data_file_blocks) {
		 while (hash_per_block_bits * levels < 64
				&& (data_file_blocks - 1) >> (hash_per_block_bits * levels))
			 levels++;
	 }
	 //	printf("Using %d hash levels.\n", levels);

	 if (levels > VERITY_MAX_LEVELS) {
		 printf("Too many tree levels for verity volume.\n");
		 return -EINVAL;
	 }

	 for (i = levels - 1; i >= 0; i--) {
		 hash_level_block[i] = hash_position;
		 // verity position of block data_file_blocks at level i
		 s = (data_file_blocks + ((loff_t) 1 << ((i + 1) * hash_per_block_bits))
			  - 1) >> ((i + 1) * hash_per_block_bits);
		 hash_level_size[i] = s;
		 if ((hash_position + s) < hash_position || (hash_position + s) < 0) {
			 printf("Device offset overflow 3 : %lld, %lld\n", (long long int)s, (long long int)hash_position);
			 return -EINVAL;
		 }
		 hash_position += s;
	 }

	 if (mult_overflow(&hash_device_size, hash_position, hash_block_size)) {
		 printf("Device offset overflow 4 : %lld, %d\n", (long long int)hash_position, (int)hash_block_size);
		 return -EINVAL;
	 }

	 //	printf("Data device size required %ld bytes.", data_device_size);
	 data_file = fopen(data_device, "r");
	 if (!data_file) {
		 printf("Cannot open device %s.\n", data_device);
		 r = -EIO;
		 goto out;
	 }

	 printf("Hash device size required:%ld bytes.\n", (long)hash_device_size);
	 hash_file = fopen(hash_device, "w+");
	 if (!hash_file) {
		 printf("Cannot open device %s.\n", hash_device);
		 r = -EIO;
		 goto out;
	 }

	 memset(calculated_digest, 0, digest_size);


	 for (i = 0; i < levels; i++) {
		 if (!i) {

			 /* parallelize the first level to speed up reading and hashing time */
#ifdef PARALLEL_HASH  
			 r = create_hash_level0(data_device , hash_file, 
									data_block_size, hash_level_block[0], hash_block_size,  
									data_file_blocks, version, hash_name,
									 digest_size, salt, salt_size);
#else
			 r = create_hash(data_file, hash_file, 0, data_block_size,
							 hash_level_block[i], hash_block_size, data_file_blocks,
							 version, hash_name, calculated_digest, digest_size, salt,
							 salt_size);
#endif
			 if (r)
				 goto out;
		 } else {
			 hash_file_2 = fopen(hash_device, "r+");
			 if (!hash_file_2) {
				 printf("Cannot open device %s.\n", hash_device);
				 r = -EIO;
				 goto out;
			 }
			 r = create_hash(hash_file_2, hash_file, hash_level_block[i - 1],
							 hash_block_size, hash_level_block[i], hash_block_size,
							 hash_level_size[i - 1], version, hash_name,
							 calculated_digest, digest_size, salt, salt_size);
			 fclose(hash_file_2);
			 if (r)
				 goto out;
		 }
	 }

	 if (levels)
		 r = create_hash(hash_file, NULL, hash_level_block[levels - 1],
						 hash_block_size, 0, hash_block_size, 1, version, hash_name,
						 calculated_digest, digest_size, salt, salt_size);
	 else
		 r = create_hash(data_file, NULL, 0, data_block_size, 0, hash_block_size,
						 data_file_blocks, version, hash_name, calculated_digest,
						 digest_size, salt, salt_size);

  out: 
	 if (r == -EIO)
		 printf("Input/output error while creating hash area.\n");
	 else if (r)
		 printf("Creation of hash area failed, errno=%d\n", r);
	 else {
		 fflush(hash_file);
		 fsync(fileno(hash_file));
		 memcpy(root_hash, calculated_digest, digest_size);
		 
	 }

	 if (data_file)
		 fclose(data_file);
	 if (hash_file)
		 fclose(hash_file);

	 return r;
 }



