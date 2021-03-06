/*
 *  Author : WangBoJing , email : 1989wangbojing@gmail.com
 * 
 *  Copyright Statement:
 *  --------------------
 *  This software is protected by Copyright and the information contained
 *  herein is confidential. The software may not be copied and the information
 *  contained herein may not be used or disclosed except with the written
 *  permission of Author. (C) 2016
 * 
 * The following hash function is based on MurmurHash3, placed into the public
 * domain by Austin Appleby.  See http://code.google.com/p/smhasher/ for
 * details.
 *
 * Code From Redis
 
****       *****
  ***        *
  ***        *                         *               *
  * **       *                         *               *
  * **       *                         *               *
  *  **      *                        **              **
  *  **      *                       ***             ***
  *   **     *       ******       ***********     ***********    *****    *****
  *   **     *     **     **          **              **           **      **
  *    **    *    **       **         **              **           **      *
  *    **    *    **       **         **              **            *      *
  *     **   *    **       **         **              **            **     *
  *     **   *            ***         **              **             *    *
  *      **  *       ***** **         **              **             **   *
  *      **  *     ***     **         **              **             **   *
  *       ** *    **       **         **              **              *  *
  *       ** *   **        **         **              **              ** *
  *        ***   **        **         **              **               * *
  *        ***   **        **         **     *        **     *         **
  *         **   **        **  *      **     *        **     *         **
  *         **    **     ****  *       **   *          **   *          *
*****        *     ******   ***         ****            ****           *
                                                                       *
                                                                      *
                                                                  *****
                                                                  ****


 *
 */

#include "NattyAbstractClass.h"
#include "NattyTcpServer.h"
#include "NattyHash.h"
#include "NattyResult.h"

#include <sys/types.h>
#include <sys/socket.h>

#define JEMALLOC_NO_DEMANGLE 1
#define JEMALLOC_NO_RENAME	 1
#include <jemalloc/jemalloc.h>


uint32_t hash_rotl_32(uint32_t x, int8_t r) {
	return ((x << r) | (x >> (32 - r)));
}

U64 hash_rotl_64(U64 x, int8_t r) {
	return ((x << r) | (x >> (64 - r)));
}

uint32_t hash_get_block_32(const uint32_t *p, int i) {
	return (p[i]);
}

U64 hash_get_block_64(const U64 *p, int i) {
	return (p[i]);
}

uint32_t hash_fmix_32(uint32_t h) {

	h ^= h >> 16;
	h *= 0x85ebca6b;
	h ^= h >> 13;
	h *= 0xc2b2ae35;
	h ^= h >> 16;

	return (h);
}

U64 hash_fmix_64(U64 k) {

	k ^= k >> 33;
	k *= KQU(0xff51afd7ed558ccd);
	k ^= k >> 33;
	k *= KQU(0xc4ceb9fe1a85ec53);
	k ^= k >> 33;

	return (k);
}

uint32_t hash_x86_32(const void *key, int len, uint32_t seed) {
	const uint8_t *data = (const uint8_t *) key;
	const int nblocks = len / 4;

	uint32_t h1 = seed;

	const uint32_t c1 = 0xcc9e2d51;
	const uint32_t c2 = 0x1b873593;

	/* body */
	{
		const uint32_t *blocks = (const uint32_t *) (data + nblocks*4);
		int i;

		for (i = -nblocks; i; i++) {
			uint32_t k1 = hash_get_block_32(blocks, i);

			k1 *= c1;
			k1 = hash_rotl_32(k1, 15);
			k1 *= c2;

			h1 ^= k1;
			h1 = hash_rotl_32(h1, 13);
			h1 = h1*5 + 0xe6546b64;
		}
	}

	/* tail */
	{
		const uint8_t *tail = (const uint8_t *) (data + nblocks*4);

		uint32_t k1 = 0;

		switch (len & 3) {
			case 3: k1 ^= tail[2] << 16;
			case 2: k1 ^= tail[1] << 8;
			case 1: k1 ^= tail[0]; k1 *= c1; k1 = hash_rotl_32(k1, 15);
			 k1 *= c2; h1 ^= k1;
		}
	}

	/* finalization */
	h1 ^= len;

	h1 = hash_fmix_32(h1);

	return (h1);
}

void hash_x86_128(const void *key, const int len, uint32_t seed, U64 r_out[2])
{
	const uint8_t * data = (const uint8_t *) key;
	const int nblocks = len / 16;

	uint32_t h1 = seed;
	uint32_t h2 = seed;
	uint32_t h3 = seed;
	uint32_t h4 = seed;

	const uint32_t c1 = 0x239b961b;
	const uint32_t c2 = 0xab0e9789;
	const uint32_t c3 = 0x38b34ae5;
	const uint32_t c4 = 0xa1e38b93;

 /* body */
	{
		const uint32_t *blocks = (const uint32_t *) (data + nblocks*16);
		int i;

		for (i = -nblocks; i; i++) {
			uint32_t k1 = hash_get_block_32(blocks, i*4 + 0);
			uint32_t k2 = hash_get_block_32(blocks, i*4 + 1);
			uint32_t k3 = hash_get_block_32(blocks, i*4 + 2);
			uint32_t k4 = hash_get_block_32(blocks, i*4 + 3);

			k1 *= c1; k1 = hash_rotl_32(k1, 15); k1 *= c2; h1 ^= k1;

			h1 = hash_rotl_32(h1, 19); h1 += h2;
			h1 = h1*5 + 0x561ccd1b;

			k2 *= c2; k2 = hash_rotl_32(k2, 16); k2 *= c3; h2 ^= k2;

			h2 = hash_rotl_32(h2, 17); h2 += h3;
			h2 = h2*5 + 0x0bcaa747;

			k3 *= c3; k3 = hash_rotl_32(k3, 17); k3 *= c4; h3 ^= k3;

			h3 = hash_rotl_32(h3, 15); h3 += h4;
			h3 = h3*5 + 0x96cd1c35;

			k4 *= c4; k4 = hash_rotl_32(k4, 18); k4 *= c1; h4 ^= k4;

			h4 = hash_rotl_32(h4, 13); h4 += h1;
			h4 = h4*5 + 0x32ac3b17;
		}
	}

 /* tail */
	{
		const uint8_t *tail = (const uint8_t *) (data + nblocks*16);
		uint32_t k1 = 0;
		uint32_t k2 = 0;
		uint32_t k3 = 0;
		uint32_t k4 = 0;

		switch (len & 15) {
			case 15: k4 ^= tail[14] << 16;
			case 14: k4 ^= tail[13] << 8;
			case 13: k4 ^= tail[12] << 0;
			k4 *= c4; k4 = hash_rotl_32(k4, 18); k4 *= c1; h4 ^= k4;

			case 12: k3 ^= tail[11] << 24;
			case 11: k3 ^= tail[10] << 16;
			case 10: k3 ^= tail[ 9] << 8;
			case  9: k3 ^= tail[ 8] << 0;
			k3 *= c3; k3 = hash_rotl_32(k3, 17); k3 *= c4; h3 ^= k3;

			case  8: k2 ^= tail[ 7] << 24;
			case  7: k2 ^= tail[ 6] << 16;
			case  6: k2 ^= tail[ 5] << 8;
			case  5: k2 ^= tail[ 4] << 0;
			k2 *= c2; k2 = hash_rotl_32(k2, 16); k2 *= c3; h2 ^= k2;

			case  4: k1 ^= tail[ 3] << 24;
			case  3: k1 ^= tail[ 2] << 16;
			case  2: k1 ^= tail[ 1] << 8;
			case  1: k1 ^= tail[ 0] << 0;
			k1 *= c1; k1 = hash_rotl_32(k1, 15); k1 *= c2; h1 ^= k1;
		}
	}

	/* finalization */
	h1 ^= len; h2 ^= len; h3 ^= len; h4 ^= len;

	h1 += h2; h1 += h3; h1 += h4;
	h2 += h1; h3 += h1; h4 += h1;

	h1 = hash_fmix_32(h1);
	h2 = hash_fmix_32(h2);
	h3 = hash_fmix_32(h3);
	h4 = hash_fmix_32(h4);

	h1 += h2; h1 += h3; h1 += h4;
	h2 += h1; h3 += h1; h4 += h1;

	r_out[0] = (((U64) h2) << 32) | h1;
	r_out[1] = (((U64) h4) << 32) | h3;
}

void hash_x64_128(const void *key, const int len, const uint32_t seed, U64 r_out[2]) {
	const uint8_t *data = (const uint8_t *) key;
	const int nblocks = len / 16;

	U64 h1 = seed;
	U64 h2 = seed;

	const U64 c1 = KQU(0x87c37b91114253d5);
	const U64 c2 = KQU(0x4cf5ad432745937f);

 /* body */
	{
		const U64 *blocks = (const U64 *) (data);
		int i;

		for (i = 0; i < nblocks; i++) {
			U64 k1 = hash_get_block_64(blocks, i*2 + 0);
			U64 k2 = hash_get_block_64(blocks, i*2 + 1);

			k1 *= c1; k1 = hash_rotl_64(k1, 31); k1 *= c2; h1 ^= k1;

			h1 = hash_rotl_64(h1, 27); h1 += h2;
			h1 = h1*5 + 0x52dce729;

			k2 *= c2; k2 = hash_rotl_64(k2, 33); k2 *= c1; h2 ^= k2;

			h2 = hash_rotl_64(h2, 31); h2 += h1;
			h2 = h2*5 + 0x38495ab5;
		}
	}

 /* tail */
	{
		const uint8_t *tail = (const uint8_t*)(data + nblocks*16);
		U64 k1 = 0;
		U64 k2 = 0;

		switch (len & 15) {
			case 15: k2 ^= ((U64)(tail[14])) << 48;
			case 14: k2 ^= ((U64)(tail[13])) << 40;
			case 13: k2 ^= ((U64)(tail[12])) << 32;
			case 12: k2 ^= ((U64)(tail[11])) << 24;
			case 11: k2 ^= ((U64)(tail[10])) << 16;
			case 10: k2 ^= ((U64)(tail[ 9])) << 8;
			case  9: k2 ^= ((U64)(tail[ 8])) << 0;
			k2 *= c2; k2 = hash_rotl_64(k2, 33); k2 *= c1; h2 ^= k2;

			case  8: k1 ^= ((U64)(tail[ 7])) << 56;
			case  7: k1 ^= ((U64)(tail[ 6])) << 48;
			case  6: k1 ^= ((U64)(tail[ 5])) << 40;
			case  5: k1 ^= ((U64)(tail[ 4])) << 32;
			case  4: k1 ^= ((U64)(tail[ 3])) << 24;
			case  3: k1 ^= ((U64)(tail[ 2])) << 16;
			case  2: k1 ^= ((U64)(tail[ 1])) << 8;
			case  1: k1 ^= ((U64)(tail[ 0])) << 0;
			k1 *= c1; k1 = hash_rotl_64(k1, 31); k1 *= c2; h1 ^= k1;
		}
	}

	/* finalization */
	h1 ^= len; h2 ^= len;

	h1 += h2;
	h2 += h1;

	h1 = hash_fmix_64(h1);
	h2 = hash_fmix_64(h2);

	h1 += h2;
	h2 += h1;

	r_out[0] = h1;
	r_out[1] = h2;
}

/******************************************************************************/
/* API. */
void hash(const void *key, size_t len, const uint32_t seed, size_t r_hash[2])
{
#if (LG_SIZEOF_PTR == 3 && !defined(JEMALLOC_BIG_ENDIAN))
 hash_x64_128(key, len, seed, (U64 *)r_hash);
#else
 U64 hashes[2];
 hash_x86_128(key, len, seed, hashes);
 r_hash[0] = (size_t)hashes[0];
 r_hash[1] = (size_t)hashes[1];
#endif
}

#define NTY_HASH_CRCTABLE_LENGTH			256
#define NTY_HASH_CRC_KEY					0x12345678ul

static U32 u32HashCrcTable[NTY_HASH_CRCTABLE_LENGTH] = {0};
HashNode* ntyHashSearchNode(void *_self, U32 key);


static void ntyGenHashCrcTable(void) {
	U16 i,j;
	U32 u32CrcNum = 0;

	for (i = 0;i < NTY_HASH_CRCTABLE_LENGTH;i ++) {
		U32 u32CrcNum = (i << 24);
		for (j = 0;j < 8;j ++) {
			if (u32CrcNum & 0x80000000L) {
				u32CrcNum = (u32CrcNum << 1) ^ NTY_HASH_CRC_KEY;
			} else {
				u32CrcNum = (u32CrcNum << 1);
			}
		}
		u32HashCrcTable[i] = u32CrcNum;
	}
}

static U32 ntyGenHashCrcValue(U8 *buf, int length) {
	U32 u32CRC = 0xFFFFFFFF;
	
	while (length -- > 0) {
		u32CRC = (u32CRC << 8) ^ u32HashCrcTable[((u32CRC >> 24) ^ *buf++) & 0xFF];
	}

	return u32CRC;
}


U32 ntyHashCrcKey(U8 *buf, int length) {
	U32 Crc = ntyGenHashCrcValue(buf, length);

	return (Crc & NATTY_DICTIONARY_MASK);
}

U32 ntyHashKey(Payload *load) {
	U32 Crc = 0x0;
	if (load == NULL) return 0x0;

	Crc = ntyHashCrcKey((U8*)load, sizeof(Payload));
	return Crc;
}

#if 0
U8 ntyHashNodeClear(HashNode *node) {
	node->sockfd = 0;
	node->list = NULL;

	free(node->info);
	node->info = NULL;
	free(node);
	ntylog("ntyHashNodeClear --> end\n");

	return 0;
}


U8 ntyHashDictionaryClear(HashNode *node) {
	if (node->list != NULL) {
		HashNode *Link = node->list;
		
		memset(node->info, 0, sizeof(Payload));
		memcpy(node->info, Link->info, sizeof(Payload));
		
		node->sockfd = Link->sockfd;
		node->list = Link->list;

		ntyHashNodeClear(Link);
		Link = NULL;
	} else {
		node->sockfd = 0;
		node->list = NULL;
		
		free(node->info);			
		node->info = NULL;
	}
}

U8 ntyHashLinkClear(HashNode *node) {
	if (node->list != NULL) {
		HashNode *Link = node->list;
		
		memset(node->info, 0, sizeof(Payload));
		memcpy(node->info, Link->info, sizeof(HashNode));

		node->sockfd = Link->sockfd;
		node->list = Link->list;

		ntyHashNodeClear(Link);
	} else {
	// node
		ntyHashNodeClear(node);
	}
}

#endif

void* ntyHashInitialize(HashTable *table) {
#if 0
	table->Dictionary = (HashNode*)malloc(NATTY_DICTIONARY_LENGTH*sizeof(HashNode));
	memset(table->Dictionary, 0, NATTY_DICTIONARY_LENGTH*sizeof(HashNode));
#else
	table->Dictionary = (HashSlot*)malloc(NATTY_DICTIONARY_LENGTH*sizeof(HashSlot));
	memset(table->Dictionary, 0, NATTY_DICTIONARY_LENGTH*sizeof(HashSlot));
#endif

	ntyGenHashCrcTable();
	return table;
}

void* ntyHashRelease(HashTable *table) {
	free(table->Dictionary);
	return table;
}

void* ntyHashCtor(void *self, va_list *params) {
	return ntyHashInitialize(self);
}

void* ntyHashDtor(void *self) {
	 ntyHashRelease(self);
	 return self;
}


int ntyHashInsert(void *_self, U32 key, Payload* load) {
	HashTable *table = _self;
	U32 index = key % NATTY_DICTIONARY_LENGTH;

	if (table == NULL) return NTY_RESULT_ERROR;
	if (load == NULL) return NTY_RESULT_ERROR;
	
#if 0

	HashNode *node = ntyHashSearchNode(table, key);
	if (node != NULL) {
		ntylog(" Error ntyHashInsert ,socket:%d, node id --> %lld, \n", node->sockfd, node->info->id);
		
		node->sockfd = key;
		node->info->id = load->id;
		
		return NTY_RESULT_EXIST;
	}

	node = &table->Dictionary[index];
	//ntylog("ntyHashInsert\n");

	if (node->sockfd == 0x0) { //no exist hash node	
	
		node->info = (Payload*)malloc(sizeof(Payload));
		ASSERT(node->info != NULL);
		node->info->id = load->id;
		
		node->sockfd = key;
		node->list = NULL;
		
	} else { //exist hash node
	
		HashNode *iter = table->Dictionary[index].list;
		node = (HashNode*)malloc(sizeof(HashNode));
		ASSERT(node != NULL);
		
		node->info = (Payload*)malloc(sizeof(Payload));
		ASSERT(node->info != NULL);
		node->info->id = load->id;
		
		node->sockfd = key;
		node->list = NULL;
		
		table->Dictionary[key].list = node;
		node->list = iter;
		
	}
#else

	HashNode *node = ntyHashSearchNode(table, key);
	if (node != NULL) {
		
		node->sockfd = key;
		node->info->id = load->id;
		
		return NTY_RESULT_EXIST;
	}

	HashSlot *slot = &table->Dictionary[index];

	//malloc node
	node = (HashNode*)malloc(sizeof(HashNode));
	if (node == NULL) return NTY_RESULT_ERROR;

	node->info = (Payload*)malloc(sizeof(Payload));
	if (node->info == NULL) {
		free(node);

		return NTY_RESULT_ERROR;
	}

	node->info->id = load->id;
	node->sockfd = key;
	node->next = NULL;

	//insert node
	if(0 == cmpxchg(&slot->slot_lock, 0, 1, WORD_WIDTH)) {
		
		HashNode *iter = slot->list;
		slot->list = node;
		node->next = iter;

		slot->num ++;
		slot->slot_lock = 0;
	} else {
		free(node->info);
		free(node);

		return NTY_RESULT_PROCESS;
	}
	

#endif
	//ntylog(" ntyHashInsert --> %lx\n", (long)node);

	return NTY_RESULT_SUCCESS;
}


HashNode* ntyHashSearchNode(void *_self, U32 key) {
	HashTable *table = _self;
	U32 index = key % NATTY_DICTIONARY_LENGTH;

	if (table == NULL) return NULL;
#if 0
	HashNode *node = &table->Dictionary[index];
	if (node->info == NULL) {
		ntylog("ntyHashSearchNode Empty Node\n");
		return NULL;
	} else {
		if (node->sockfd == key) { //
			ntylog(" key : %d, clientId : %lld\n", key, node->info->id);
			return node;
		} else {
			HashNode *iter = node->list;

			while (iter != NULL) {
				ntylog(" key : %d, clientId : %lld\n", key, iter->info->id);
				if (node->sockfd == key) {
					return iter;
				}
				iter = iter->list;
			}
		}
	}
#else

	HashSlot *slot = &table->Dictionary[index];
	HashNode *node = slot->list;

	while (node) {
		
		if (node->sockfd == key) {
			return node;
		}
		node = node->next;
	}

#endif
	return NULL;
}

Payload* ntyHashSearch(void *_self, U32 key) {
	HashNode *node = ntyHashSearchNode(_self, key);
	if (node == NULL) return NULL;
	return node->info;
}



int ntyHashDelete(void *_self, U32 key) {
	HashTable *table = _self;
	U32 index = key % NATTY_DICTIONARY_LENGTH;

	if (table == NULL) return NTY_RESULT_FAILED;
#if 0	
	HashNode *node = &table->Dictionary[index];
	if (node->info == NULL) {
		ntylog("ntyHashDelete\n");
		return NTY_RESULT_ERROR;
	} else {
			
		if (node->sockfd == key) {
			//ntylog(" Delete Hash Table, ip:%d,port%d\n", load->srcip, load->sport);
			//delete node
			ntylog("ntyHashDelete --> table %d value:%lld\n", key, node->info->id);
			ntyHashDictionaryClear(node);
			return NTY_RESULT_SUCCESS;

		} else {
			HashNode *iter = node->list;
			HashNode *pre = node;
				
			while (iter != NULL) {
				if (node->sockfd == key) {
					ntylog("ntyHashDelete --> node %d value:%lld\n", key, node->info->id);
					
					ntyHashLinkClear(iter);
					pre->list = NULL;
					return NTY_RESULT_SUCCESS;
				}
				pre = iter;
				iter = iter->list;
			}
		}
	}
#else

	HashSlot *slot = &table->Dictionary[index];
	HashNode *node = slot->list;

	if (node == NULL) return NTY_RESULT_NOEXIST;

	HashNode *iter = node->next;
	
	
	while (iter != NULL) {
		if (iter->sockfd == key) {
			if (0 == cmpxchg(&slot->slot_lock, 0, 1, WORD_WIDTH)) {
				slot->num --;

				//delete node
				node->next = iter->next;

				if (iter->info != NULL) {
					free(iter->info);
				}
				free(iter);				
				iter = node;
				
				slot->slot_lock = 0;
			}
		}
		node = iter;
		iter = iter->next;
	}

	//first node
	iter = slot->list;
	if (iter->sockfd == key) {
		if (0 == cmpxchg(&slot->slot_lock, 0, 1, WORD_WIDTH)) {
			slot->num --;

			slot->list = iter->next;

			if (iter->info != NULL) {
				free(iter->info);
			}
			free(iter);	
			
			slot->slot_lock = 0;
		}
	}

#endif

	return NTY_RESULT_SUCCESS;
}

int ntyHashUpdate(void *_self, U32 key, Payload* load) {
	HashNode *node = ntyHashSearchNode(_self, key);
	if (node == NULL) {
		return NTY_RESULT_FAILED;
	}
	if (node->sockfd == NATTY_ILLEGAL_FD) {
		ntylog(" ntyHashUpdate --> %d\n", node->sockfd);
		return NTY_RESULT_NOEXIST;
	}

	if ( node->info == NULL ){
		return NTY_RESULT_FAILED;
	}
	node->info->id = load->id;
	
	return NTY_RESULT_SUCCESS;
}


static const HashTableHandle ntyHashTableHandle = {
	sizeof(HashTable),
	ntyHashCtor,
	ntyHashDtor,
	ntyHashInsert,
	ntyHashSearch,
	ntyHashDelete,
	ntyHashUpdate,
};

const void *pNtyHashTableHandle = &ntyHashTableHandle;

static void *pHashTableHandle = NULL;

void *ntyHashTableInstance(void) {
	if (pHashTableHandle == NULL) {
		void *pHashTable = New(pNtyHashTableHandle);
		if ((unsigned long)NULL != cmpxchg((void*)(&pHashTableHandle), (unsigned long)NULL, (unsigned long)pHashTable, WORD_WIDTH)) {
			Delete(pHashTable);
		}
	}
	
	return pHashTableHandle;
}

void *ntyHashTableRelease(void) {
	Delete(pHashTableHandle);
	pHashTableHandle = NULL;
}


int ntyHashTableInsert(void *self, U32 key, Payload* load) {
	HashTableHandle * const * pHandle = self;

	if (self && (*pHandle) && (*pHandle)->insert) {
		return (*pHandle)->insert(self, key, load);
	}

	return NTY_RESULT_FAILED;
}

Payload* ntyHashTableSearch(void *self, U32 key) {
	HashTableHandle * const * pHandle = self;

	if (self && (*pHandle) && (*pHandle)->search) {
		return (*pHandle)->search(self, key);
	}
	return NULL;
}


int ntyHashTableDelete(void *self, U32 key) {
	HashTableHandle * const * pHandle = self;

	if (self && (*pHandle) && (*pHandle)->delete) {
		return (*pHandle)->delete(self, key);
	}
	return NTY_RESULT_FAILED;
}

int ntyHashTableUpdate(void *self, U32 key, Payload* load) {
	HashTableHandle * const * pHandle = self;

	if (self && (*pHandle) && (*pHandle)->update) {
		return (*pHandle)->update(self, key, load);
	}
	return NTY_RESULT_FAILED;
}

#if 0
int main() {
	void *hash = ntyHashTableInstance();

	C_DEVID clientId = 240207489190414451;
	int key = 1;

	int i = 0;
	int Count = NATTY_DICTIONARY_LENGTH*5;
	Payload payload = {0};

	for (i = 0;i < Count;i ++) {
		
		payload.id = clientId;
		ntyHashTableInsert(hash, key, &payload);

		key ++;
		clientId ++;
	}

	for (i = 0;i < Count;i ++) {
		key = i * 3 + 1;
		Payload *load = ntyHashTableSearch(hash, key);

		if (load != NULL) {
			ntylog(" load info : %lld\n", load->id);
		}
	}

	for (i = 0;i < Count;i ++) {
		key = i+1;
		ntyHashTableDelete(hash, key);
		//ntylog(" \n");
	}
	ntylog("end\n");
}
#endif








