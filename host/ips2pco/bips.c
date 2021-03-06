/* bips - A IPS patcher tool */

#include "blib.h"
#include "ips_fmt.h"

#define MIN(a,b) (((a)<(b))?(a):(b))

uint8_t* ips_buffer = NULL;

int simulate  = 0; // Don't actually apply; only show the results.
int plaintext = 0; // Decompile IPS to a human-readable listing.
int splitmini = 0; // Split IPS to one chunk mini-ips patches in a folder.

// On success, returns the number of IPS records read. On failure,
// returns -1.
int load_ips(__READ char* ips_filename, __WRITE ips_record_t** ips_structs) {
	// We load the entire thing to memory.

	FILE* f = fopen(ips_filename, "r");
	fseek(f, 0, SEEK_END);
	uint64_t pos = ftell(f);
	rewind(f);

	ips_buffer = (uint8_t*)malloc(pos);

	fread(ips_buffer, 1, pos, f);

	fclose(f);

	printf("Loaded file to memory successfully. Size: %lu\n", pos);

	// Loaded. Begin by checking shit.
	if( strncmp((const char*)ips_buffer, IPS_MAGIC, IPS_MAGIC_LENGTH) ) {
		// Invalid signature. Not an IPS.
		free(ips_buffer);
		ips_buffer = NULL;
		return -1;
	}

	// Seems legit. Begin record calculation.
	int ips_count = 0;
	uint64_t offset_in = 5;
	ips_record_t* ips_data = NULL;
	while( offset_in < pos && strncmp((char*)&ips_buffer[offset_in], IPS_TAIL, IPS_TAIL_LENGTH) ) {
		// Increment.
		ips_count++;

		// Reallocate.
		ips_data = (ips_record_t*)realloc(ips_data, sizeof(ips_record_t) * ips_count);

		ips_data[ips_count-1].info = (ips_record_com_t*)&ips_buffer[offset_in];

		offset_in += sizeof(ips_record_com_t);

		ips_data[ips_count-1].data = (void*)&ips_buffer[offset_in];

		if(ips_data[ips_count-1].info->size[0] == 0x00 && ips_data[ips_count-1].info->size[1] == 0x00) // Zero is zero regardless of byte order, no casting needed
			// RLE. Add the size of an RLE struct.
			offset_in += sizeof(ips_record_rle_t);
		else
			offset_in += BYTE2_TO_UINT16(ips_data[ips_count-1].info->size);

		// Aaand onto the next.
	}

	printf("Read in IPS data. Record count: %d\n", ips_count);

	ips_structs[0] = ips_data;

	return ips_count;
}

int conv_ips(__READ char* filename, __READ ips_record_t* records, __READ int record_count) {
	FILE* out = stdout;
	if (filename != NULL) {
		out = fopen(filename, "wb");
		if (!out) {
			fprintf(stderr, "Can't open. Writing to stdout.\n");
			out = stdout;
		}
	}

	srand(time(NULL));
	uint16_t uuid = (0x6600 + (rand() % 0x4400));

	fprintf(out, "# $name  EDITME\n");
	fprintf(out, "# $desc  Converted IPS Patch\n");
	fprintf(out, "# $title EDITME\n");
	fprintf(out, "# $ver   01\n");
	fprintf(out, "# $uuid  %04hx\n", uuid);
	fprintf(out, "# This file was automatically generated by ips2pco\n");

	for (int i=0; i < record_count; i++) {
		uint32_t offset = BYTE3_TO_UINT32(records[i].info->offset);
		uint16_t size = BYTE2_TO_UINT16(records[i].info->size);

		fprintf(out, "seek %08x\n", offset);

		if (size == 0) { // RLE
			ips_record_rle_t* rle = (ips_record_rle_t*)records[i].data;

			size = BYTE2_TO_UINT16(rle->rle_size);

			for (int i=0; i < size; i += 0x20) {
				uint32_t max_l = (size - i > 0x20) ?
					0x20 :
					size - i;
				fprintf(out, "set  ");

				for(uint32_t j=0; j < max_l; j++)
					fprintf(out, "%02hhx", rle->byte);
				fprintf(out, "\n");
			}
		} else { // Normal
			uint8_t* bytes = (uint8_t*)records[i].data;

			for (int i=0; i < size; i += 0x20) {
				uint32_t max_l = (size - i > 0x20) ?
					0x20 :
					size - i;
				fprintf(out, "set  ");

				for(uint32_t j=0; j < max_l; j++)
					fprintf(out, "%02hhx", bytes[i+j]);
				fprintf(out, "\n");
			}
		}
	}

	return 0;
}


// On success, returns the number of IPS records read. On failure,
// returns -1.
int load_ips32(__READ char* ips_filename, __WRITE ips32_record_t** ips_structs) {
	// We load the entire thing to memory.

	FILE* f = fopen(ips_filename, "r");
	fseek(f, 0, SEEK_END);
	uint64_t pos = ftell(f);
	rewind(f);

	ips_buffer = (uint8_t*)malloc(pos);

	fread(ips_buffer, 1, pos, f);

	fclose(f);

	printf("Loaded file to memory successfully. Size: %lu\n", pos);

	// Loaded. Begin by checking shit.
	if( strncmp((const char*)ips_buffer, IPS32_MAGIC, IPS32_MAGIC_LENGTH) ) {
		// Invalid signature. Not an IPS.
		free(ips_buffer);
		ips_buffer = NULL;
		return -1;
	}

	// Seems legit. Begin record calculation.
	int ips_count = 0;
	uint64_t offset_in = 5;
	ips32_record_t* ips_data = NULL;
	while( offset_in < pos && strncmp((char*)&ips_buffer[offset_in], IPS32_TAIL, IPS32_TAIL_LENGTH) ) {
		// Increment.
		ips_count++;

		// Reallocate.
		ips_data = (ips32_record_t*)realloc(ips_data, sizeof(ips32_record_t) * ips_count);

		ips_data[ips_count-1].info = (ips32_record_com_t*)&ips_buffer[offset_in];

		offset_in += sizeof(ips32_record_com_t);

		ips_data[ips_count-1].data = (void*)&ips_buffer[offset_in];

		if(ips_data[ips_count-1].info->size[0] == 0x00 && ips_data[ips_count-1].info->size[1] == 0x00) { // Zero is zero regardless of byte order, no casting needed
			// RLE. Add the size of an RLE struct.
			offset_in += sizeof(ips32_record_rle_t);
		} else {
			offset_in += BYTE2_TO_UINT16(ips_data[ips_count-1].info->size);
		}

		// Aaand onto the next.
	}

	printf("Read in IPS data. Record count: %d\n", ips_count);

	ips_structs[0] = ips_data;

	return ips_count;
}

int conv_ips32(__READ char* filename, __READ ips32_record_t* records, __READ int record_count) {
	FILE* out = stdout;
	if (filename != NULL) {
		out = fopen(filename, "wb");
		if (!out) {
			fprintf(stderr, "Can't open. Writing to stdout.\n");
			out = stdout;
		}
	}
	
	srand(time(NULL));
	uint16_t uuid = (0x6600 + (rand() % 0x4400));
	
	fprintf(out, "# $name  EDITME\n");
	fprintf(out, "# $desc  Converted IPS Patch\n");
	fprintf(out, "# $title EDITME\n");
	fprintf(out, "# $ver   01\n");
	fprintf(out, "# $uuid  %04hx\n", uuid);
	fprintf(out, "# This file was automatically generated by ips2pco\n");
	
	for (int i=0; i < record_count; i++) {
		uint32_t offset = BYTE4_TO_UINT32(records[i].info->offset);
		uint16_t size = BYTE2_TO_UINT16(records[i].info->size);
		
		fprintf(out, "seek %08x\n", offset);

		if (size == 0) { // RLE
			ips32_record_rle_t* rle = (ips32_record_rle_t*)records[i].data;

			size = BYTE2_TO_UINT16(rle->rle_size);

			for (int i=0; i < size; i += 0x20) {
				uint32_t max_l = (size - i > 0x20) ?
					0x20 :
					size - i;
				fprintf(out, "set  ");

				for(uint32_t j=0; j < max_l; j++)
					fprintf(out, "%02hhx", rle->byte);
				fprintf(out, "\n");
			}
		} else { // Normal
			uint8_t* bytes = (uint8_t*)records[i].data;
			
			for (int i=0; i < size; i += 0x20) {
				uint32_t max_l = (size - i > 0x20) ?
					0x20 :
					size - i;
				fprintf(out, "set  ");

				for(uint32_t j=0; j < max_l; j++)
					fprintf(out, "%02hhx", bytes[i+j]);
				fprintf(out, "\n");
			}
		}
	}
	
	if (out != stdout)
		fclose(out);

	return 0;
}


int identify_patch(__READ char* filename) {
	char test[8];
	FILE* f = fopen(filename, "r");
	fseek(f, 0, SEEK_END);
	if (ftell(f) < 8) {
		// Wrong. No patch is smaller than this. Die.
		return TYPE_INVALID;
	}
	rewind(f);
	fread(test, 1, 8, f);

	fclose(f);

	if ( !strncmp(test, IPS_MAGIC, IPS_MAGIC_LENGTH) )
		return TYPE_IPS;
	if ( !strncmp(test, IPS32_MAGIC, IPS32_MAGIC_LENGTH) )
		return TYPE_IPS32;

	return TYPE_INVALID;
}

void help(char* name) {
	printf("corbenik ips2pco v0.1\n");
	printf("(C) 2015 Jon Feldman (@chaoskagami) <kagami@chaos.moe>\n");
	printf("Usage:\n");
	printf("   %s [options] patch.ips [out.pco]\n", name);
	printf("Options:\n");
	printf("   -h        Print this help text.\n");
	printf("Patch is dumped to stdout if a output file is not specified.\n");
	printf("Report bugs to <https://github.com/chaoskagami/corbenik>\n");
	printf("This software is licensed under the MIT license.\n");

}

int main(int argc, char** argv) {
	ips_record_t*   ips   = NULL;
	ips32_record_t* ips32 = NULL;
	int record_count = 0;
	int opt;
	int prog_ret = 0;

	while ( (opt = getopt(argc, argv, "hs")) != -1) {
		switch(opt) {
			case 'h':
				help(argv[0]);
				return 1;
			case 's':
				simulate = 1;
				break;
			case '?':
				fprintf(stderr, "error: unknown option. Run with -h for more info\n");
				return 1;
			default:
				fprintf(stderr, "error: unknown option. Run with -h for more info\n");
				return 1;
		}
	}

	if (argc - optind < 1) {
		fprintf(stderr, "error: requires more arguments. Run with -h for more info\n");
		return 1;
	}

	char* patch  = argv[optind];
	int type = identify_patch(patch);
	char *out_file = NULL;
	if (argc - optind >= 2) {
		out_file  = argv[optind+1];
	}

	switch(type) {
		case TYPE_IPS:
			fprintf(stderr, "Patch format: IPS (24-bit offsets)\n");
			record_count = load_ips(patch, &ips);
			conv_ips(out_file, ips, record_count);
			break;
		case TYPE_IPS32:
			fprintf(stderr, "Patch format: IPS32 (IPS derivative w/ 32-bit offsets)\n");
			record_count = load_ips32(patch, &ips32);
			conv_ips32(out_file, ips32, record_count);
			break;
		default:
			fprintf(stderr, "Patch format not understood or invalid.\n");
			prog_ret = -2;
			break;
	}
		
	free(ips);
	free(ips32);
	free(ips_buffer);
	
	return prog_ret;
}
