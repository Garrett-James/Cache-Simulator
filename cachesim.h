/*
 * CS3375 Computer Architecture
 * Course Project
 * Cache Simulator Design and Development
 * FALL 2017
 */

#define BLOCK_SIZE  64        /* Cache block size (or cache line size) in bytes \\
                                 (must be power of 2). 4 Bytes = 1 Word */
#define WAY_SIZE    1         /* Associativity; 1-way = direct-mapped */
#define CACHE_SIZE  32768     /* Cache capacity in bytes (must be power of 2)*/
#define NUM_BLOCKS  (CACHE_SIZE / BLOCK_SIZE)
#define NUM_SETS    (NUM_BLOCKS / WAY_SIZE)
#define DBGSETTINGS
//#define DBG

/* The hit miss structure. Contains hit and miss to allow for one print function in cachesim.c */
struct hit_miss
{
	int hits;							/* Hit count */
	int misses;							/* Miss count */
};

/*The data structure of direct-mapped cache*/
struct direct_mapped_cache {
	struct hit_miss hitMiss;	/* Abstract Cache. Gives access to hit and miss */
    unsigned valid_field[NUM_BLOCKS];   /* Valid field */
    unsigned dirty_field[NUM_BLOCKS];   /* Dirty field; since we don't distinguish writes and \\
                                           reads in this project yet, this field doesn't really matter */
    uint64_t tag_field[NUM_BLOCKS];     /* Tag field */
    char data_field[NUM_BLOCKS][BLOCK_SIZE];  /* Data field; since we don't really fetch data, \\
                                                 this field doesn't really matter */
};

struct fully_associative_cache {
	unsigned valid_field[NUM_BLOCKS]; /* Valid Field */
	unsigned reference_field[NUM_BLOCKS]; /* Reference Field */
	unsigned dirty_field[NUM_BLOCKS];   /* Dirty field; since we don't distinguish writes and \\
										reads in this project yet, this field doesn't really matter */
	uint64_t tag_field[NUM_BLOCKS];     /* Tag field */
	char data_field[NUM_BLOCKS][BLOCK_SIZE];  /* Data field; since we don't really fetch data, \\
											  this field doesn't really matter */
	struct hit_miss hitMiss;			/* Abstract Cache. Gives access to hit and miss */
};

struct n_way_cache {
	unsigned valid_field[NUM_BLOCKS];   /* Valid field */
	unsigned dirty_field[NUM_BLOCKS];   /* Dirty field; since we don't distinguish writes and \\
										reads in this project yet, this field doesn't really matter */
	uint64_t reference_field[NUM_BLOCKS];
	uint64_t tag_field[NUM_BLOCKS];
	// uint64_t tag_field[NUM_BLOCKS/];     /* Tag field */
	char data_field[NUM_BLOCKS][BLOCK_SIZE];  /* Data field; since we don't really fetch data, \\
											  this field doesn't really matter */
	struct hit_miss hitMiss;			/* Abstract Cache. Gives access to hit and miss */
};


/* Print the given cache's Hit and Miss rate to both console and file */
void printHitMiss(struct hit_miss *cache, FILE *log, int type);
/*Read the memory traces and convert it to binary*/
uint64_t convert_address(char memory[], FILE *log);
/*Simulate the direct-mapped cache*/
void direct_mapped_cache_access(struct direct_mapped_cache *cache, uint64_t address, FILE *log);
/*Simulate the fully associative cache*/
void fully_associative_cache_access(struct fully_associative_cache *cache, uint64_t address, FILE *log);
/*Simulate the n-way associative cache*/
void n_way_cache_access(struct n_way_cache *cache, uint64_t address, FILE *log,int n_sets);
void rr_n_way_cache_access(struct n_way_cache *cache, uint64_t address, FILE *log, int n_sets);
