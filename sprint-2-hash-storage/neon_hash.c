/*
 * ARM NEON SIMD Hash Function Optimization
 *
 * High-performance hash functions using ARM NEON intrinsics
 * for the hash storage engine. Optimized for ARM64 architecture.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/types.h>

#ifdef __aarch64__
#include <asm/neon.h>
#include <asm/simd.h>
#endif

#define NEON_HASH_CHUNK_SIZE 16 /* Process 16 bytes at a time with NEON */
#define HASH_MULTIPLIER_1 0x9e3779b9U
#define HASH_MULTIPLIER_2 0x85ebca6bU
#define HASH_MULTIPLIER_3 0xc2b2ae35U
#define HASH_MULTIPLIER_4 0x27d4eb2fU

/*
 * Fallback scalar hash function for non-ARM64 or when NEON is unavailable
 */
static uint32_t
scalar_hash_function(const void *data, size_t len)
{
	const unsigned char *bytes = (const unsigned char *)data;
	uint32_t hash = 0x811c9dc5U; /* FNV-1a initial value */
	size_t i;

	for (i = 0; i < len; i++) {
		hash ^= bytes[i];
		hash *= 0x01000193U; /* FNV-1a prime */
	}

	return hash;
}

#ifdef __aarch64__
/*
 * ARM NEON optimized hash function
 * Processes 16 bytes at a time using SIMD instructions
 */
static uint32_t
neon_hash_function_impl(const void *data, size_t len)
{
	const unsigned char *bytes = (const unsigned char *)data;
	size_t full_chunks = len / NEON_HASH_CHUNK_SIZE;
	size_t remainder = len % NEON_HASH_CHUNK_SIZE;
	uint32_t hash = 0x811c9dc5U;
	size_t i;

	/* Process 16-byte chunks with NEON */
	if (full_chunks > 0) {
		/* Initialize NEON vectors */
		uint32x4_t hash_vec = vdupq_n_u32(hash);
		uint32x4_t mult_vec = { HASH_MULTIPLIER_1, HASH_MULTIPLIER_2,
					HASH_MULTIPLIER_3, HASH_MULTIPLIER_4 };

		for (i = 0; i < full_chunks; i++) {
			/* Load 16 bytes of data */
			uint8x16_t data_vec
			    = vld1q_u8(bytes + i * NEON_HASH_CHUNK_SIZE);

			/* Convert to 4x uint32 for processing */
			uint32x4_t data_u32 = vreinterpretq_u32_u8(data_vec);

			/* XOR with current hash values */
			hash_vec = veorq_u32(hash_vec, data_u32);

			/* Multiply by different constants */
			hash_vec = vmulq_u32(hash_vec, mult_vec);

			/* Rotate bits for better distribution */
			hash_vec = vorrq_u32(vshlq_n_u32(hash_vec, 13),
					     vshrq_n_u32(hash_vec, 19));
		}

		/* Combine the 4 hash values into one */
		uint32x2_t hash_pair
		    = vadd_u32(vget_low_u32(hash_vec), vget_high_u32(hash_vec));
		hash = vget_lane_u32(vpadd_u32(hash_pair, hash_pair), 0);
	}

	/* Process remaining bytes with scalar code */
	for (i = full_chunks * NEON_HASH_CHUNK_SIZE; i < len; i++) {
		hash ^= bytes[i];
		hash *= 0x01000193U;
	}

	return hash;
}

/*
 * Batch hash function - hash multiple keys simultaneously
 */
static void
neon_hash_batch_impl(const void **keys, size_t *key_lens, uint32_t *hashes,
		     size_t count)
{
	size_t i;

	/* For small batches, use individual hashing */
	if (count < 4) {
		for (i = 0; i < count; i++) {
			hashes[i]
			    = neon_hash_function_impl(keys[i], key_lens[i]);
		}
		return;
	}

	/* Process 4 keys simultaneously when possible */
	for (i = 0; i + 3 < count; i += 4) {
		/* Initialize hash vectors */
		uint32x4_t hash_vec = vdupq_n_u32(0x811c9dc5U);
		uint32x4_t mult_vec = { HASH_MULTIPLIER_1, HASH_MULTIPLIER_2,
					HASH_MULTIPLIER_3, HASH_MULTIPLIER_4 };

		/* Find minimum length for vectorized processing */
		size_t min_len = key_lens[i];
		size_t j;
		for (j = 1; j < 4; j++) {
			if (key_lens[i + j] < min_len)
				min_len = key_lens[i + j];
		}

		/* Process common length with NEON */
		size_t chunks
		    = min_len / 4; /* Process 4 bytes per key simultaneously */
		for (j = 0; j < chunks; j++) {
			/* Load 4 bytes from each of the 4 keys */
			uint32_t data[4];
			data[0]
			    = *((uint32_t *)((unsigned char *)keys[i] + j * 4));
			data[1] = *(
			    (uint32_t *)((unsigned char *)keys[i + 1] + j * 4));
			data[2] = *(
			    (uint32_t *)((unsigned char *)keys[i + 2] + j * 4));
			data[3] = *(
			    (uint32_t *)((unsigned char *)keys[i + 3] + j * 4));

			uint32x4_t data_vec = vld1q_u32(data);

			/* Update hashes */
			hash_vec = veorq_u32(hash_vec, data_vec);
			hash_vec = vmulq_u32(hash_vec, mult_vec);
		}

		/* Store results and finish with scalar processing */
		vst1q_u32(&hashes[i], hash_vec);

		/* Finish remaining bytes for each key individually */
		for (j = 0; j < 4; j++) {
			const unsigned char *bytes
			    = (const unsigned char *)keys[i + j];
			uint32_t hash = hashes[i + j];
			size_t k;

			for (k = chunks * 4; k < key_lens[i + j]; k++) {
				hash ^= bytes[k];
				hash *= 0x01000193U;
			}
			hashes[i + j] = hash;
		}
	}

	/* Process remaining keys individually */
	for (; i < count; i++) {
		hashes[i] = neon_hash_function_impl(keys[i], key_lens[i]);
	}
}
#endif /* __aarch64__ */

/*
 * Public interface: Optimized hash function
 * Automatically selects NEON or scalar implementation
 */
uint32_t
neon_hash_function(const void *key, size_t len)
{
#ifdef __aarch64__
	/* Check if NEON is available and safe to use */
	if (may_use_simd()) {
		uint32_t result;
		kernel_neon_begin();
		result = neon_hash_function_impl(key, len);
		kernel_neon_end();
		return result;
	}
#endif
	/* Fallback to scalar implementation */
	return scalar_hash_function(key, len);
}

/*
 * Public interface: Batch hash function
 * Hash multiple keys efficiently
 */
void
neon_hash_batch(const void **keys, size_t *key_lens, uint32_t *hashes,
		size_t count)
{
	if (!keys || !key_lens || !hashes || count == 0)
		return;

#ifdef __aarch64__
	/* Check if NEON is available and safe to use */
	if (may_use_simd()) {
		kernel_neon_begin();
		neon_hash_batch_impl(keys, key_lens, hashes, count);
		kernel_neon_end();
		return;
	}
#endif
	/* Fallback to scalar implementation */
	size_t i;
	for (i = 0; i < count; i++) {
		hashes[i] = scalar_hash_function(keys[i], key_lens[i]);
	}
}

/*
 * Key comparison function with NEON optimization
 * Compare two keys for equality with SIMD acceleration
 */
int
neon_key_compare(const void *key1, size_t key1_len, const void *key2,
		 size_t key2_len)
{
	if (key1_len != key2_len)
		return 1;

	if (key1_len == 0)
		return 0;

#ifdef __aarch64__
	/* Use NEON for larger comparisons */
	if (key1_len >= 16 && may_use_simd()) {
		const unsigned char *bytes1 = (const unsigned char *)key1;
		const unsigned char *bytes2 = (const unsigned char *)key2;
		size_t full_chunks = key1_len / 16;
		size_t remainder = key1_len % 16;
		size_t i;

		kernel_neon_begin();

		/* Compare 16-byte chunks */
		for (i = 0; i < full_chunks; i++) {
			uint8x16_t vec1 = vld1q_u8(bytes1 + i * 16);
			uint8x16_t vec2 = vld1q_u8(bytes2 + i * 16);
			uint8x16_t cmp = vceqq_u8(vec1, vec2);

			/* Check if all bytes are equal */
			uint64x2_t result = vreinterpretq_u64_u8(cmp);
			if (vgetq_lane_u64(result, 0) != 0xFFFFFFFFFFFFFFFFULL
			    || vgetq_lane_u64(result, 1)
				   != 0xFFFFFFFFFFFFFFFFULL) {
				kernel_neon_end();
				return 1; /* Not equal */
			}
		}

		kernel_neon_end();

		/* Compare remaining bytes */
		return memcmp(bytes1 + full_chunks * 16,
			      bytes2 + full_chunks * 16, remainder);
	}
#endif

	/* Fallback to standard memcmp */
	return memcmp(key1, key2, key1_len);
}

/*
 * Hash quality measurement - count collisions for analysis
 */
uint32_t
neon_hash_distribution_test(const void **keys, size_t *key_lens,
			    size_t key_count, uint32_t bucket_count)
{
	uint32_t *hashes;
	uint32_t *bucket_counts;
	uint32_t collisions = 0;
	size_t i;

	if (!keys || !key_lens || key_count == 0 || bucket_count == 0)
		return 0;

	hashes = kmalloc(key_count * sizeof(uint32_t), GFP_KERNEL);
	if (!hashes)
		return 0;

	bucket_counts = kzalloc(bucket_count * sizeof(uint32_t), GFP_KERNEL);
	if (!bucket_counts) {
		kfree(hashes);
		return 0;
	}

	/* Hash all keys */
	neon_hash_batch(keys, key_lens, hashes, key_count);

	/* Count items per bucket */
	for (i = 0; i < key_count; i++) {
		uint32_t bucket = hashes[i] % bucket_count;
		bucket_counts[bucket]++;
	}

	/* Count collisions (buckets with > 1 item) */
	for (i = 0; i < bucket_count; i++) {
		if (bucket_counts[i] > 1) {
			collisions += bucket_counts[i] - 1;
		}
	}

	kfree(hashes);
	kfree(bucket_counts);

	return collisions;
}

EXPORT_SYMBOL(neon_hash_function);
EXPORT_SYMBOL(neon_hash_batch);
EXPORT_SYMBOL(neon_key_compare);
EXPORT_SYMBOL(neon_hash_distribution_test);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ARM Storage Engine Learning Project");
MODULE_DESCRIPTION("ARM NEON SIMD Hash Function Optimization");
MODULE_VERSION("1.0");