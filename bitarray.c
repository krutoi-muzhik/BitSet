#include "bitarray.h"

#define COVER_TEST

#ifndef COVER_TEST
#define RELEASE
#endif

// RELEASE // RELEASE // RELEASE // RELEASE // RELEASE // RELEASE // RELEASE // RELEASE // RELEASE // RELEASE // RELEASE // RELEASE //
#ifdef RELEASE

#undef COVER_TEST

void *Calloc (size_t num, size_t size) {return calloc (num, size);}
void *Malloc (size_t size) {return malloc (size);}
void *Realloc (void *ptr, size_t size) {return realloc (ptr, size);}

#endif // RELEASE
//-------------------------------------------------------------------------------------------------------------------------------------


// COVERAGE TEST // COVERAGE TEST // COVERAGE TEST // COVERAGE TEST // COVERAGE TEST // COVERAGE TEST // COVERAGE TEST // COVERAGE TEST
#ifdef COVER_TEST

static int alloc_call = 0;

void *Calloc (size_t num, size_t size) {
	if (alloc_call == 5) {
		alloc_call = 0;
		return NULL;
	} else {
		alloc_call ++;
		return calloc (num, size);
	}

	return NULL;
}

// void *Malloc (size_t size) {
// 	if (alloc_call == 5) {
// 		alloc_call = 0;
// 		return 0;
// 	} else {
// 		alloc_call ++;
// 		return malloc (size);
// 	}

// 	return NULL;
// }

void *Realloc (void *ptr, size_t size) {
	if (alloc_call == 5) {
		alloc_call = 0;
		return NULL;
	} else {
		alloc_call ++;
		return realloc (ptr, size);
	}

	return NULL;
}
#endif // COVER_TEST
//-------------------------------------------------------------------------------------------------------------------------------------


bitarr_t *Bitarr (int capacity) {
	if (capacity < 1)
		return NULL;

	void *container = Calloc (1, sizeof (bitarr_t));
	if (container == NULL)
		container = Calloc (1, sizeof (bitarr_t));
	bitarr_t *bitarr = (bitarr_t *) container;

	size_t num = 0;
	if (capacity % ELEMENT_SIZE == 0)
		num = capacity / ELEMENT_SIZE;
	else
		num = capacity / ELEMENT_SIZE + 1;

	void *arr = Calloc (num, sizeof (uint64_t));
	if (arr == NULL)
		arr = Calloc (num, sizeof (uint64_t));

	bitarr->array = (uint64_t *) arr;
	bitarr->capacity = num * ELEMENT_SIZE;

	return bitarr;
}

error_t Destruct (bitarr_t *bitarr) {
	if (bitarr == NULL)
		return INVALID_CONTAINER;

	if (bitarr->array == NULL) {
		free (bitarr);
		return INVALID_ARRAY;
	}

	free (bitarr->array);
	bitarr->array = NULL;
	bitarr->capacity = 0;
	free (bitarr);

	return VSE_OK;
}

error_t All (bitarr_t *bitarr) {
	error_t err = Check (bitarr);
	if (err != VSE_OK)
		return err;

	for (ssize_t i = 0; i < bitarr->capacity / ELEMENT_SIZE; i ++) {
		if (bitarr->array[i] != ULLONG_MAX)
			return 0;
	}

	return 1;
}

error_t Any (bitarr_t *bitarr) {
	error_t err = Check (bitarr);
	if (err != VSE_OK)
		return err;

	for (ssize_t i = 0; i < bitarr->capacity / ELEMENT_SIZE; i ++) {
		if (bitarr->array[i])
			return 1;
	}

	return 0;
}

error_t None (bitarr_t *bitarr) {
	error_t err = Check (bitarr);
	if (err != VSE_OK)
		return err;

	for (ssize_t i = 0; i < bitarr->capacity / ELEMENT_SIZE; i ++) {
		if (bitarr->array[i])
			return 0;
	}

	return 1;
}

error_t Reset (bitarr_t *bitarr) {
	error_t err = Check (bitarr);
	if (err != VSE_OK)
		return err;

	for (ssize_t i = 0; i < bitarr->capacity / ELEMENT_SIZE; i ++) {
		bitarr->array[i] = 0;
	}

	return VSE_OK;
}

error_t Set (bitarr_t *bitarr) {
	error_t err = Check (bitarr);
	if (err != VSE_OK)
		return err;

	for (ssize_t i = 0; i < bitarr->capacity / ELEMENT_SIZE; i ++) {
		bitarr->array[i] = ULLONG_MAX;
	}

	return VSE_OK;
}

error_t SetVal (bitarr_t *bitarr, bit_t bit) {
	error_t err = Check (bitarr);
	if (err != VSE_OK)
		return err;

	switch (bit) {
		case SET:
			return Set (bitarr);
		case UNSET:
			return Reset (bitarr);
		default:
			return INVALID_VAL;
	}

	return UNKNOWN_ERROR;
}

error_t Flip (bitarr_t *bitarr) {
	error_t err = Check (bitarr);
	if (err != VSE_OK)
		return err;

	for (ssize_t i = 0; i < bitarr->capacity / ELEMENT_SIZE; i ++) {
		bitarr->array[i] = ~bitarr->array[i];
	}

	return VSE_OK;
}

error_t ResizeUp (bitarr_t *bitarr, int extra) {
	if (extra < 1)
		return INVALID_CAPACITY;

	error_t err = Check (bitarr);
	if (err != VSE_OK)
		return err;

	size_t newsize = (bitarr->capacity + extra) / ELEMENT_SIZE + 1;

	void *ptr = Realloc (bitarr->array, newsize * sizeof (uint64_t));
	if (ptr == NULL) 
		return ALLOC_ERR;

	bitarr->array = (uint64_t *) ptr;
	bitarr->capacity = newsize * ELEMENT_SIZE;

	return VSE_OK;
}

// error_t ResizeDown (bitarr_t *bitarr, size_t extra) {
// 	bitarr->array = (uint64_t *) Realloc (bitarr->array, (bitarr->capacity - extra) / ELEMENT_SIZE + 1);
// 	bitarr->capacity -= extra;

// 	return VSE_OK;
// }

int GetBit (bitarr_t *bitarr, int pos) {
	if (pos < 0)
		return INVALID_POS;

	error_t err = Check (bitarr);
	if (err != VSE_OK)
		return err;

	if (pos >= bitarr->capacity) {
		error_t err = ResizeUp (bitarr, pos - bitarr->capacity);
		if (err == VSE_OK)
			return UNSET;
		return err;
	}

	uint64_t mask = 1;
	mask = mask << pos % ELEMENT_SIZE;
	if (bitarr->array[pos / ELEMENT_SIZE] & mask)
		return SET;

	return UNSET;
}

error_t SetBit (bitarr_t *bitarr, int pos) {
	if (pos < 0)
		return INVALID_POS;

	error_t err = Check (bitarr);
	if (err != VSE_OK)
		return err;

	if (pos >= bitarr->capacity) {
		err = ResizeUp (bitarr, pos - bitarr->capacity + 1);
		if (err != VSE_OK)
			return err;
	}

	uint64_t *point = bitarr->array + pos / ELEMENT_SIZE; 
	uint64_t mask = 1;
	mask = mask << (pos % ELEMENT_SIZE);

	*point = *point | mask;

	return VSE_OK;
}

error_t UnsetBit (bitarr_t *bitarr, int pos) {
	if (pos < 0)
		return INVALID_POS;

	error_t err = Check (bitarr);
	if (err != VSE_OK)
		return err;

	if (pos >= bitarr->capacity) {
		err = ResizeUp (bitarr, pos - bitarr->capacity + 1);
		return err;
	}

	uint64_t *point = bitarr->array + pos / ELEMENT_SIZE; 
	uint64_t mask = 1;
	mask = mask << (pos % ELEMENT_SIZE);

	mask = ~mask;
	*point = *point & mask;

	return VSE_OK;
}

error_t SetBitVal (bitarr_t *bitarr, int pos, bit_t bit) {
	if (pos < 0)
		return INVALID_POS;

	error_t err = Check (bitarr);
	if (err != VSE_OK)
		return err;

	switch (bit) {
		case SET:
			return SetBit (bitarr, pos);
		case UNSET:
			return UnsetBit (bitarr, pos);
		default:
			return INVALID_VAL;
	}

	return UNKNOWN_ERROR;
}

int Count (bitarr_t *bitarr) {
	error_t err = Check (bitarr);
	if (err != VSE_OK)
		return err;

	int result = 0;

	for (size_t i = 0; i < bitarr->capacity / ELEMENT_SIZE; i ++) {
		if (!bitarr->array[i])
			continue;

		for (ssize_t offset = 0; offset < ELEMENT_SIZE; offset ++) {
			if (bitarr->array[i] & ((uint64_t) 1 << offset)) {
				result ++;
			}
		}
	}

	return result;
}

int FindSetPos (bitarr_t *bitarr, int num) {
	if (num < 1)
		return INVALID_NUM;

	error_t err = Check (bitarr);
	if (err != VSE_OK)
		return err;

	int pos = 0;
	int count = 0;

	for (ssize_t i = 0; i < bitarr->capacity / ELEMENT_SIZE; i ++) {
		if (!bitarr->array[i])
			continue;

		for (ssize_t offset = 0; offset < ELEMENT_SIZE; offset ++) {
			if (bitarr->array[i] & ((uint64_t) 1 << offset))
				count ++;
			
			if (count == num)
				return pos + offset;
		}

		pos += ELEMENT_SIZE;
	}

	return -1;
}

int FindUnsetPos (bitarr_t *bitarr, int num) {
	if (num < 1)
		return INVALID_NUM;

	error_t err = Check (bitarr);
	if (err != VSE_OK)
		return err;

	int pos = 0;
	int count = 0;

	for (ssize_t i = 0; i < bitarr->capacity / ELEMENT_SIZE; i ++) {
		if (bitarr->array[i] == ULLONG_MAX)
			continue;

		for (ssize_t offset = 0; offset < ELEMENT_SIZE; offset ++) {
			if (!(bitarr->array[i] & ((uint64_t) 1 << offset))) {
				count ++;
			}
			
			if (count == num)
				return pos + offset;
		}

		pos += ELEMENT_SIZE;
	}

	return -1;
}

int FindFirstSet (bitarr_t *bitarr, size_t offset) {
	error_t err = Check (bitarr);
	if (err != VSE_OK)
		return err;

	if (offset >= bitarr->capacity)
		return -1;

	size_t i = offset / ELEMENT_SIZE;
	int result = offset;

	for (size_t j = offset % ELEMENT_SIZE; j < ELEMENT_SIZE; j ++) {
		if (bitarr->array[i] & ((uint64_t) 1 << j))
			return result;
		result ++;
	}

	i ++;

	for (i; i <= bitarr->capacity / ELEMENT_SIZE - 1; i ++) {
		if (bitarr->array[i])
			break;
		result += ELEMENT_SIZE;
	}

	if (result == bitarr->capacity)
		return -1;
	
	for (size_t j = 0; j < ELEMENT_SIZE; j ++) {
		if (bitarr->array[i] & ((uint64_t) 1 << j))
			break;
		result ++;
	}

	return result;
}

int FindFirstUnset (bitarr_t *bitarr, size_t offset) {
	error_t err = Check (bitarr);
	if (err != VSE_OK)
		return err;

	if (offset >= bitarr->capacity)
		return -1;

	size_t i = offset / ELEMENT_SIZE;
	int result = offset;


	for (size_t j = offset % ELEMENT_SIZE; j < ELEMENT_SIZE; j ++) {
		if (!(bitarr->array[i] & ((uint64_t) 1 << j)))
			return result;
		result ++;
	}

	i ++;

	for (i; i <= bitarr->capacity / ELEMENT_SIZE; i ++) {
		if (bitarr->array[i] != ULLONG_MAX)
			break;
		result += ELEMENT_SIZE;
	}

	if (result == bitarr->capacity)
		return -1;

	uint64_t current = bitarr->array[i];

	for (size_t j = 0; j < ELEMENT_SIZE; j ++) {
		if (!(current & (uint64_t) 1 << j))
			break;
		result ++;
	}

	return result;
}

int FindLastSet (bitarr_t *bitarr, size_t offset) {
	error_t err = Check (bitarr);
	if (err != VSE_OK)
		return err;

	if (offset >= bitarr->capacity)
		return -1;

	size_t i = bitarr->capacity / ELEMENT_SIZE - offset / ELEMENT_SIZE - 1;
	int result = bitarr->capacity - offset - 1;

	for (ssize_t j = ELEMENT_SIZE - offset % ELEMENT_SIZE; j >= 0; j --) {
		if (bitarr->array[i] & ((uint64_t) 1 << j))
			break;
		result --;
	}

	i --;

	for (i = bitarr->capacity / ELEMENT_SIZE - 1; i >= 0; i --) {
		if (bitarr->array[i])
			break;
		result -= ELEMENT_SIZE;
	}

	if (result < 0)
		return -1;

	uint64_t current = bitarr->array[i];
	
	for (ssize_t j = ELEMENT_SIZE - 1; j >= 0; j --) {
		if (current & ((uint64_t) 1 << j))
			break;
		result --;
	}

	return result;
}

int FindLastUnset (bitarr_t * bitarr, size_t offset) {
	error_t err = Check (bitarr);
	if (err != VSE_OK)
		return err;

	if (offset >= bitarr->capacity)
		return -1;

	size_t i = bitarr->capacity / ELEMENT_SIZE - offset / ELEMENT_SIZE - 1;
	int result = bitarr->capacity - offset - 1;

	for (ssize_t j = ELEMENT_SIZE - offset % ELEMENT_SIZE; j >= 0; j --) {
		if (!(bitarr->array[i] & ((uint64_t) 1 << j)))
			break;
		result --;
	}

	i --;

	for (i = bitarr->capacity / ELEMENT_SIZE - 1; i >= 0; i --) {
		if (bitarr->array[i] != ULLONG_MAX)
			break;
		result -= ELEMENT_SIZE;
	}

	if (result < 0)
		return -1;

	uint64_t current = bitarr->array[i];

	for (ssize_t j = ELEMENT_SIZE - 1; j >= 0; j --) {
		if (!(current & ((uint64_t) 1 << j)))
			break;
		result --;
	}

	return result;
}

error_t Check (bitarr_t *bitarr) {
	if (bitarr == NULL)
		return INVALID_CONTAINER;

	if (bitarr->array == NULL)
		return INVALID_ARRAY;

	if ((bitarr->capacity % ELEMENT_SIZE) || (bitarr->capacity < 1))
		return INVALID_CAPACITY;

	return VSE_OK;
}


// LOGS DUMP // LOGS DUMP // LOGS DUMP // LOGS DUMP // LOGS DUMP // LOGS DUMP // LOGS DUMP // LOGS DUMP // LOGS DUMP // LOGS DUMP // 
error_t Dump (bitarr_t *bitarr, const char *pathname) {
	error_t err = Check (bitarr);
	if (err != VSE_OK)
		return err;
	FILE *dumpfile = fopen (pathname, "w+");
	fprintf (dumpfile, "BitArray %s\n", TimeNow ());
	uint64_t mask = 1;
	for (size_t i = 0; i < bitarr->capacity / ELEMENT_SIZE; i++) {
		fprintf (dumpfile, "[%ld] =", i);
		for (size_t offset = 0; offset < ELEMENT_SIZE; offset++) {
			if (!(offset % 4)) fprintf (dumpfile, " ");
			if (bitarr->array[i] & (mask << offset)) fprintf (dumpfile, "1");
			else fprintf (dumpfile, "0");
		}
		fprintf (dumpfile, "\n");
	}
	return VSE_OK;
}

char *TimeNow () {
	time_t t = time (NULL);
	return asctime (localtime (&t));
}

size_t Capacity (bitarr_t *bitarr) {
	error_t err = Check (bitarr);
	if (err != VSE_OK)
		return 0;

	return bitarr->capacity;
}
//-------------------------------------------------------------------------------------------------------------------------------------
