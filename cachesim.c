/*
 * CS3375 Computer Architecture
 * Course Project
 * Cache Simulator Design and Development
 * FALL 2017
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "cachesim.h"

int main(int argc, char *argv[])
{
	int x = argc;
	if (!(argc == 3 || argc == 4)) {
		printf("Usage: %s <direct/full/4/8/16> <trace file name> \n", argv[0]);
		printf("OR\n");
		printf("Usage: %s <4/8/16> <trace file name> <nru/rr> \n", argv[0]);
		return 1;
	}
	
	char* trace_file_name = argv[2];
	char buff[0x100];
	struct direct_mapped_cache d_cache;
	struct fully_associative_cache f_cache;
	struct n_way_cache n_cache;
	char mem_request[20];
	uint64_t address;
	FILE *fp;
	FILE *log;
	/* Opening the memory trace file */
	fp = fopen(trace_file_name, "r");
	/* Create output log file | If not defined, nru | Otherwise, append replacement method */
	if (argc == 4)
	{
		snprintf(buff, sizeof(buff), "%s_%s_%s_log.txt", trace_file_name, argv[1], argv[3]);
	}
	else if(argc == 3) {
		snprintf(buff, sizeof(buff), "%s_%s_log.txt", trace_file_name, argv[1]);
	}
	log = fopen(buff, "w");

#ifdef DBGSETTINGS
	printf("BLOCK SIZE = %d Bytes\n", BLOCK_SIZE);
	printf("%d-WAY\n", atoi(argv[1]));
	printf("CACHE SIZE = %d Bytes\n", CACHE_SIZE);
	printf("NUMBER OF BLOCKS = %d\n", NUM_BLOCKS);
	printf("NUMBER OF SETS = %d\n", NUM_SETS);
	printf("\n");
	printf("FILE = %s\n", trace_file_name);
	// To file
	fprintf(log, "BLOCK SIZE = %d Bytes\n", BLOCK_SIZE);
	fprintf(log, "%d-WAY\n", atoi(argv[1]));
	fprintf(log, "CACHE SIZE = %d Bytes\n", CACHE_SIZE);
	fprintf(log, "NUMBER OF BLOCKS = %d\n", NUM_BLOCKS);
	fprintf(log, "NUMBER OF SETS = %d\n", NUM_SETS);
	fprintf(log, "\n");
	fprintf(log, "FILE = %s\n", trace_file_name);
#endif

	/* Initialization of various caches moved into each brach, to avoid unnecessary creation*/
	/* Simulating direct-mapped cache */
	if (strncmp(argv[1], "direct", 6) == 0) {
		/* Initialization */
		for (int i = 0; i<NUM_BLOCKS; i++) {
			d_cache.valid_field[i] = 0;
			d_cache.dirty_field[i] = 0;
			d_cache.tag_field[i] = 0;
		}
		d_cache.hitMiss.hits = 0;
		d_cache.hitMiss.misses = 0;
		/* Read the memory request address and access the cache */
		while (fgets(mem_request, 20, fp) != NULL) {
			address = convert_address(mem_request, log);
			direct_mapped_cache_access(&d_cache, address, log);
		}
		/*Print out the results to log file*/
		printHitMiss(&d_cache.hitMiss, log, -1);
	}
	/* Simulating fully associative cache */
	else if (strncmp(argv[1], "full", 4) == 0) {
		/* Initialization */
		for (int i = 0; i<NUM_BLOCKS; i++) {
			f_cache.valid_field[i] = 0;
			f_cache.dirty_field[i] = 0;
			f_cache.reference_field[i] = 0;
			f_cache.tag_field[i] = 999999;
		}
		f_cache.hitMiss.hits = 0;
		f_cache.hitMiss.misses = 0;
		while (fgets(mem_request, 20, fp) != NULL) {
			address = convert_address(mem_request, log);
			fully_associative_cache_access(&f_cache, address, log);
		}
		/*Print out the results to log file*/
		printHitMiss(&f_cache.hitMiss, log, 0);
	}
	/* Simulating n-way mapped cache with NRU*/
	else if (argc==3 || strncmp(argv[3], "nru", 3) == 0){
		/* Initialization */
		for (int i = 0; i<NUM_BLOCKS; i++) {
			n_cache.valid_field[i] = 0;
			n_cache.dirty_field[i] = 0;
			n_cache.reference_field[i] = 0;
			n_cache.tag_field[i] = -1;
		}
		n_cache.hitMiss.hits = 0;
		n_cache.hitMiss.misses = 0;

		// Convert and Insert address
		while (fgets(mem_request, 20, fp) != NULL) {
			address = convert_address(mem_request, log);
			n_way_cache_access(&n_cache, address, log, atoi(argv[1]));
		}

		/*Print out the results to log file*/
		printHitMiss(&n_cache.hitMiss, log, atoi(argv[1]));
	}
	/* Simulating n-way mapped cache with RR algorithm*/
	else if (strncmp(argv[3], "rr", 2) == 0)
	{
		//Initialize
		for (int i = 0; i<NUM_BLOCKS; i++) {
			n_cache.valid_field[i] = 0;
			n_cache.dirty_field[i] = 0;
			n_cache.reference_field[i] = 0;
			n_cache.tag_field[i] = -1;
		}
		n_cache.hitMiss.hits = 0;
		n_cache.hitMiss.misses = 0;
		//Convery and add to cache
		while (fgets(mem_request, 20, fp) != NULL) {
			address = convert_address(mem_request, log);
			rr_n_way_cache_access(&n_cache, address, log, atoi(argv[1]));
		}
		/*Print out the results to log file*/
		printHitMiss(&n_cache.hitMiss, log, atoi(argv[1]));
	}

	fclose(fp);
	fclose(log);
	return 0;
}

uint64_t convert_address(char memory_addr[], FILE *log)
/* Converts the physical 32-bit address in the trace file to the "binary" \\
* (a uint64 that can have bitwise operations on it) */
{
	uint64_t binary = 0;
	int i = 0;

	while (memory_addr[i] != '\n') {
		if (memory_addr[i] <= '9' && memory_addr[i] >= '0') {
			binary = (binary * 16) + (memory_addr[i] - '0');
		}
		else {
			if (memory_addr[i] == 'a' || memory_addr[i] == 'A') {
				binary = (binary * 16) + 10;
			}
			if (memory_addr[i] == 'b' || memory_addr[i] == 'B') {
				binary = (binary * 16) + 11;
			}
			if (memory_addr[i] == 'c' || memory_addr[i] == 'C') {
				binary = (binary * 16) + 12;
			}
			if (memory_addr[i] == 'd' || memory_addr[i] == 'D') {
				binary = (binary * 16) + 13;
			}
			if (memory_addr[i] == 'e' || memory_addr[i] == 'E') {
				binary = (binary * 16) + 14;
			}
			if (memory_addr[i] == 'f' || memory_addr[i] == 'F') {
				binary = (binary * 16) + 15;
			}
		}
		i++;
	}

#ifdef DBG
	printf("%s converted to %llu\n", memory_addr, binary);
#endif
	return binary;
}

void direct_mapped_cache_access(struct direct_mapped_cache *cache, uint64_t address, FILE *log)
{
	uint64_t block_addr = address >> (unsigned)log2(BLOCK_SIZE);
	uint64_t index = block_addr % NUM_BLOCKS;
	uint64_t tag = block_addr >> (unsigned)log2(NUM_BLOCKS);

#ifdef DBG
	printf("Memory address: %llu, Block address: %llu, Index: %llu, Tag: %llu ", address, block_addr, index, tag);
#endif

	if (cache->valid_field[index] && cache->tag_field[index] == tag) { /* Cache hit */
		cache->hitMiss.hits += 1;
#ifdef DBG
		printf("Hit!\n");
#endif
	}
	else {
		/* Cache miss */
		cache->hitMiss.misses += 1;
#ifdef DBG
		printf("Miss!\n");
#endif
		if (cache->valid_field[index] && cache->dirty_field[index]) {
			/* Write the cache block back to memory */
		}
		cache->tag_field[index] = tag;
		cache->valid_field[index] = 1;
		cache->dirty_field[index] = 0;
	}
}

void fully_associative_cache_access(struct fully_associative_cache *cache, uint64_t address, FILE *log)
{
	// Get block_address
	uint64_t block_addr = address >> (unsigned)log2(BLOCK_SIZE);
	srand(time(NULL));
	// index = last line of cache checked
	int index = 0;
	// List of not recently referenced lines and the number of them
	int listNonRef[NUM_BLOCKS];
	int numNonRef = 0;
#ifdef DBG
	printf("Memory block_addr: %llu ", block_addr);
#endif
	// While tag is not the same as the initialized state
	while (cache->tag_field[index] != 999999)
	{
		// If line is valid and tag is the same, then hit
		if (cache->valid_field[index] && cache->tag_field[index] == block_addr)
		{
#ifdef DBG
			printf("Hit!\n");
#endif
			// Add Hit
			cache->hitMiss.hits += 1;
			// Recently Referenced
			cache->reference_field[index] = 1;
			index++;
			return;
		}
		//List of 0 references if no slot found. 
		if (cache->reference_field[index] == 0)
		{
			listNonRef[numNonRef] = index;
			numNonRef++;
		}
		index++;
		// If index reaches the last line
		if (index >= NUM_BLOCKS)
		{
			break;
		}
	}
	// if tag is the same as uninitialized and index is not at the end, then insert
	if (cache->tag_field[index] == 999999 && index<NUM_BLOCKS)
	{
#ifdef DBG
		printf("Miss!\n");
#endif
		//Add Miss
		cache->hitMiss.misses += 1;
		//Recently Referenced
		cache->reference_field[index] = 1;
		//Valid
		cache->valid_field[index] = 1;
		// Insert Line
		cache->tag_field[index] = block_addr;
	}
	// if index is equal to number of block and there is atleast 1 not recently referenced lines
	else if (index >= NUM_BLOCKS && numNonRef != 0)
	{
		// random number between 0 and number of not recently referenced lines
		unsigned int random = (rand() % (numNonRef));
		// get a random not recently referenced line
		int randReplace = listNonRef[random];
#ifdef DBG
		printf("Miss!\n");
#endif
		//Add Miss
		cache->hitMiss.misses += 1;
		// Insert address at random not recently referenced line
		cache->tag_field[randReplace] = block_addr;
		//Recently Referenced
		cache->reference_field[randReplace] = 1;
		cache->valid_field[randReplace] = 1;
	}
	// If cache is full and all lines are recently referenced
	else if (index >= NUM_BLOCKS && numNonRef == 0)
	{
		//Replace random line in cache
		int random = (rand() % NUM_BLOCKS);
#ifdef DBG
		printf("Miss!\n");
#endif
		//Set recently referenced to 0 for all lines
		for (int x = 0; x < NUM_BLOCKS; x++)
		{
			cache->reference_field[x] = 0;
		}
		// Add Hit
		cache->hitMiss.misses += 1;
		//Replace random line
		cache->tag_field[random] = block_addr;
		//Recently Referenced
		cache->reference_field[random] = 1;
		cache->valid_field[random] = 1;
	}
}

void n_way_cache_access(struct n_way_cache *cache, uint64_t address, FILE *log, int n_sets)
{
	int x;
	// Seed Random function
	srand(time(NULL));
	// List of not recently referenced indices
	int listNonRef[NUM_BLOCKS];
	// Number of not recently referenced indices
	int numNonRef = 0;
	// number of sets
	// could not use constant WAY_SIZE and NUM_SETS since constant cannot be changed for different n  
	int numSets = NUM_BLOCKS / n_sets;
	uint64_t block_addr = address >> (unsigned)log2(BLOCK_SIZE);
	uint64_t index = block_addr % numSets;
	uint64_t tag = block_addr >> (unsigned)log2(numSets);

#ifdef DBG
	printf("Memory address: %llu, Block address: %llu, Set Number: %llu, Tag: %llu ", address, block_addr, index, tag);
#endif

	// Check if tag is valid and already in the cache.
	// Also, gather 
	// Check each member of a set
	for (x = 0; x < n_sets; x++)
	{
		// If tag is different from initialized state
		if (cache->tag_field[x + (index*n_sets)] != -1)
		{
			// If line is valid and line tag is equal to memory tag, then Hit
			if (cache->valid_field[x + (index*n_sets)] && cache->tag_field[x + (index*n_sets)] == tag)
			{
#ifdef DBG
				printf("Hit!\n");
#endif
				// Add Hit
				cache->hitMiss.hits += 1;
				// Set to recently referenced
				cache->reference_field[x + (index*n_sets)] = 1;
				cache->valid_field[x + (index*n_sets)] = 1;
				return;
			}
			//List of 0 reference for if hit detected 
			if (cache->reference_field[(index*n_sets) + x] == 0)
			{
				// add adjusted index to listNonRef
				listNonRef[numNonRef] = x + (index*n_sets);
				numNonRef++;
			}
		}
		//Handle a non full set. If tag is equal to initialized state, then insert
		else if (cache->tag_field[x + (index*n_sets)] == -1)
		{
#ifdef DBG
			printf("Miss!\n");
#endif
			// Add miss
			cache->hitMiss.misses += 1;
			// Recently Referenced
			cache->reference_field[x + (index*n_sets)] = 1;
			// Set to valid
			cache->valid_field[x + (index*n_sets)] = 1;
			//Insert
			cache->tag_field[x + (index*n_sets)] = tag;
			return;
		}
	}
	//Handle Full Set, If atleast 1 reference=0 was found, then replace random reference=0 line
	if (numNonRef != 0)
	{
		//Random number between 0 and number of 0 references found
		int random = (rand() % (numNonRef));
		int randReplace = listNonRef[random];
#ifdef DBG
		printf("Miss!\n ");
#endif
		//Add Miss
		cache->hitMiss.misses += 1;
		// Replace random member of set with new address
		cache->tag_field[randReplace] = tag;
		// Recently Referenced
		cache->reference_field[randReplace] = 1;
		cache->valid_field[randReplace] = 1;
	}
	//Handle Full Set, If all members are recently accessed. That is, reference=1 for all members of a set
	else if (numNonRef == 0)
	{
		//Replace random member of the set
		int random = (rand() % n_sets);
#ifdef DBG
		printf("Miss!\n");
#endif
		// Set the reference_field of all members of a set to 0
		for (int x = 0; x < n_sets; x++)
		{
			cache->reference_field[x + (index*n_sets)] = 0;
		}
		// Add Miss
		cache->hitMiss.misses += 1;
		// Random member of set is replaced
		cache->tag_field[random + (index*n_sets)] = tag;
		//Recently Referenced
		cache->reference_field[random + (index*n_sets)] = 1;
		cache->valid_field[random + (index*n_sets)] = 1;
	}
}

void printHitMiss(struct hit_miss *cache, FILE *log, int type)
{
	// Print Appropriate Header
	if (type == 0)
	{
		fprintf(log, "\n==================================\n");
		fprintf(log, "Cache type:    Fully Associative-Mapped Cache\n");
		fprintf(log, "==================================\n");
		printf("\n==================================\n");
		printf("Cache type:    Fully Associative-Mapped Cache\n");
		printf("==================================\n");
	}
	else if (type == -1)
	{
		fprintf(log, "\n==================================\n");
		fprintf(log, "Cache type:    Direct-Mapped Cache\n");
		fprintf(log, "==================================\n");
		printf("\n==================================\n");
		printf("Cache type:    Direct-Mapped Cache\n");
		printf("==================================\n");
	}
	else
	{
		fprintf(log, "\n==================================\n");
		fprintf(log, "Cache type:    %d-Way Mapped Cache\n", type);
		fprintf(log, "==================================\n");
		printf("\n==================================\n");
		printf("Cache type:    %d-Way Mapped Cache\n", type);
		printf("==================================\n");
	}

	//Print Hit and Miss with Rates 
	fprintf(log, "Cache Hits:    %d\n", cache->hits);
	fprintf(log, "Cache Misses:  %d\n", cache->misses);
	fprintf(log, "Cache Hit Rate:  %.3f\n", (double)cache->hits / (cache->hits + cache->misses));
	fprintf(log, "Cache Miss Rate:  %.3f\n", (double)cache->misses / (cache->hits + cache->misses));
	fprintf(log, "\n");
	printf("Cache Hits:    %d\n", cache->hits);
	printf("Cache Misses:  %d\n", cache->misses);
	printf("Cache Hit Rate:  %.3f\n", (double)cache->hits / (cache->hits + cache->misses));
	printf("Cache Miss Rate:  %.3f\n", (double)cache->misses / (cache->hits + cache->misses));
	printf("\n");
}

void rr_n_way_cache_access(struct n_way_cache *cache, uint64_t address, FILE *log, int n_sets)
{
	int x;
	srand(time(NULL));
	// % n_sets go through cache tag array
	int numSets = NUM_BLOCKS / n_sets;
	uint64_t block_addr = address >> (unsigned)log2(BLOCK_SIZE);
	uint64_t index = block_addr % numSets;
	uint64_t tag = block_addr >> (unsigned)log2(numSets);

#ifdef DBG
	printf("Memory address: %llu, Block address: %llu, Set Number: %llu, Tag: %llu ", address, block_addr, index, tag);
#endif

	//Handle Hit | check each member of the set and 
	for (x = 0; x < n_sets; x++)
	{
		if (cache->tag_field[x + (index*n_sets)] != -1)
		{
			if (cache->valid_field[x + (index*n_sets)] && cache->tag_field[x + (index*n_sets)] == tag)
			{
#ifdef DBG
				printf("Hit!\n");
#endif
				cache->hitMiss.hits += 1;
				cache->valid_field[x + (index*n_sets)] = 1;
				return;
			}
		}
		//Handle Miss
		else if (cache->tag_field[x + (index*n_sets)] == -1)
		{
#ifdef DBG
			printf("Miss!\n");
#endif
			cache->hitMiss.misses += 1;
			cache->valid_field[x + (index*n_sets)] = 1;
			cache->tag_field[x + (index*n_sets)] = tag;
			return;
		}
	}
	// Handle a full set: Replace a random block in the correct set

			int random = (rand() % n_sets);
#ifdef DBG
			printf("Miss!\n");
#endif
			cache->hitMiss.misses += 1;
			cache->valid_field[random + (index*n_sets)] = 1;
			cache->tag_field[random + (index*n_sets)] = tag;
			return;
}


//******************************
