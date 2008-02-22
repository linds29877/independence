/*
 *  PNGHelper.cpp
 *  libPhoneInteraction
 *
 *  Created by The Operator on 31/08/07.
 *  Copyright 2007 The Operator. All rights reserved.
 *
 * This software is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License version 2 for more details 
 */

#include "PNGHelper.h"
#include <zlib.h>

#define CHUNKBUFSIZE 262144 // 256k buffer size


typedef unsigned int uint32;

typedef struct png_chunk_t {
	uint32 length;
	unsigned char name[4];
	unsigned char *data;
	uint32 crc;
} PNGChunk;


static unsigned char g_PNGHeader[8] = { 137, 80, 78, 71, 13, 10, 26, 10 };
static unsigned char g_datachunk[4] = { 0x49, 0x44, 0x41, 0x54 }; // IDAT
static unsigned char g_endchunk[4] = { 0x49, 0x45, 0x4e, 0x44 }; // IEND
static unsigned char g_cgbichunk[4] = { 0x43, 0x67, 0x42, 0x49 }; // CgBI


static unsigned long PNGCRC(unsigned char *name, unsigned char *buf, int len)
{
	uint32 crc = crc32(0, name, 4);
	return crc32(crc, buf, len);
}

static bool readPNGChunks(unsigned char* buf, int size, PNGChunk **chunks, int *numChunks)
{

	// sanity check
	if (size < 8) return false;

	// skip header
	unsigned char *bufptr = buf + 8;
	int bytecount = 8, chunkcount = 0;
	PNGChunk tmpChunk;

	// first count the number of chunks
	while ((bytecount + 4) < size) {

		// get length
		memcpy(&(tmpChunk.length), bufptr, 4);
		tmpChunk.length = ntohl(tmpChunk.length);
		bufptr += 4;
		bytecount += 4;

		// another sanity check
		if ( (bytecount + tmpChunk.length + 8) > size ) return false;

		// get name
		memcpy(tmpChunk.name, bufptr, 4);
		bufptr += 4;
		bytecount += 4;

		// skip over the rest
		bufptr += tmpChunk.length + 4;
		bytecount += tmpChunk.length + 4;

		chunkcount++;

		if (!memcmp(tmpChunk.name, g_endchunk, 4)) {
			// End of img
			break;
		}

	}

	// yet another sanity check
	if (bytecount > size) return false;

	*chunks = (PNGChunk*)malloc(chunkcount * sizeof(PNGChunk));
	*numChunks = chunkcount;

	bufptr = buf + 8;

	PNGChunk *chunkPtr;

	for (int i = 0; i < chunkcount; i++) {
		chunkPtr = (*chunks) + i;

		memcpy(&(chunkPtr->length), bufptr, 4);
		bufptr += 4;

		chunkPtr->length = ntohl(chunkPtr->length);

		memcpy(chunkPtr->name, bufptr, 4);
		bufptr += 4;

		chunkPtr->data = (unsigned char *)malloc(chunkPtr->length);
		memcpy(chunkPtr->data, bufptr, chunkPtr->length);
		bufptr += chunkPtr->length;

		memcpy(&(chunkPtr->crc), bufptr, 4);
		bufptr += 4;
		chunkPtr->crc = ntohl(chunkPtr->crc);
	}

	return true;
}

static bool processPNGChunks(PNGChunk *chunks, int numChunks)
{
	int i;
	unsigned char* outbuf;
	PNGChunk *chunkPtr;

	// Poke at any IDAT chunks and de/recompress them
	for (i = 0; i < numChunks; i++) {
		chunkPtr = chunks + i;

		if (!memcmp(chunkPtr->name, g_datachunk, 4)) {
			unsigned char inflatedbuf[CHUNKBUFSIZE];
			unsigned char *fullinfoutput;
			unsigned char deflatedbuf[CHUNKBUFSIZE];
			unsigned char *fulldefoutput;
			z_stream infstrm, defstrm;
			int fullinfoutputsize = 0, fulldefoutputsize = 0, ret;
			unsigned int have;

			infstrm.zalloc = Z_NULL;
			infstrm.zfree = Z_NULL;
			infstrm.opaque = Z_NULL;
			infstrm.avail_in = chunkPtr->length;
			infstrm.next_in = chunkPtr->data;

			// Inflate using raw inflation
			if (inflateInit2(&infstrm, -8) != Z_OK) {
				free(inflatedbuf);
				return false;
			}

			do {
				infstrm.next_out = inflatedbuf;
				infstrm.avail_out = CHUNKBUFSIZE;
				ret = inflate(&infstrm, Z_NO_FLUSH);

				switch (ret) {
					case Z_NEED_DICT:
						ret = Z_DATA_ERROR;     /* and fall through */
					case Z_DATA_ERROR:
					case Z_MEM_ERROR:
						inflateEnd(&infstrm);

						if (fullinfoutputsize > 0) free(fullinfoutput);

						return false;
				}

				have = CHUNKBUFSIZE - infstrm.avail_out;

				if (have > 0) {

					if (fullinfoutputsize == 0) {
						fullinfoutput = (unsigned char*)malloc(have);
					}
					else {
						fullinfoutput = (unsigned char*)realloc(fullinfoutput, fullinfoutputsize + have);
					}

					memcpy(fullinfoutput + fullinfoutputsize, inflatedbuf, have);
					fullinfoutputsize += have;
				}

			} while (infstrm.avail_out == 0);

			inflateEnd(&infstrm);
			
			// Now deflate again, the regular, PNG-compatible, way
			defstrm.zalloc = Z_NULL;
			defstrm.zfree = Z_NULL;
			defstrm.opaque = Z_NULL;
			defstrm.avail_in = fullinfoutputsize;
			defstrm.next_in = fullinfoutput;

			if (deflateInit(&defstrm, Z_DEFAULT_COMPRESSION) != Z_OK) {
				free(fullinfoutput);
				return false;
			}

			do {
				defstrm.next_out = deflatedbuf;
				defstrm.avail_out = CHUNKBUFSIZE;
				ret = deflate(&defstrm, Z_FINISH);

				if (ret == Z_STREAM_ERROR) {
					free(fullinfoutput);

					if (fulldefoutputsize > 0) {
						free(fulldefoutput);
					}

					return false;
				}

				have = CHUNKBUFSIZE - defstrm.avail_out;

				if (have > 0) {

					if (fulldefoutputsize == 0) {
						fulldefoutput = (unsigned char*)malloc(have);
					}
					else {
						fulldefoutput = (unsigned char*)realloc(fulldefoutput, fulldefoutputsize + have);
					}
				
					memcpy(fulldefoutput + fulldefoutputsize, deflatedbuf, have);
					fulldefoutputsize += have;
				}

			} while (defstrm.avail_out == 0);

			deflateEnd(&defstrm);
			free(fullinfoutput);

			free(chunkPtr->data);
			chunkPtr->data = fulldefoutput;
			chunkPtr->length = fulldefoutputsize;
			chunkPtr->crc = PNGCRC(chunkPtr->name, chunkPtr->data, chunkPtr->length);
		}

	}

	return true;
}

static void writePNGChunks(PNGChunk *chunks, int numChunks, unsigned char **buf, int *bufsize)
{
	int numBytes = 8;

	for (int i = 0; i < numChunks; i++) {

		if (memcmp(chunks[i].name, g_cgbichunk, 4)) {
			numBytes += chunks[i].length + 12;
		}

	}

	*buf = (unsigned char*)malloc(numBytes);
	*bufsize = numBytes;

	unsigned char *bufptr = *buf;

	memcpy(bufptr, g_PNGHeader, 8);
	bufptr += 8;

	for (int i = 0; i < numChunks; i++) {

		if (memcmp(chunks[i].name, g_cgbichunk, 4)) {
			uint32 newlen = htonl(chunks[i].length);
			uint32 newcrc = htonl(chunks[i].crc);

			memcpy(bufptr, &newlen, 4);
			bufptr += 4;
			memcpy(bufptr, chunks[i].name, 4);
			bufptr += 4;
			memcpy(bufptr, chunks[i].data, chunks[i].length);
			bufptr += chunks[i].length;
			memcpy(bufptr, &newcrc, 4);
			bufptr += 4;
		}

	}

}

static void cleanUp(PNGChunk **chunks, int numChunks)
{
	PNGChunk *chunk;

	for (int i = 0; i < numChunks; i++) {
		chunk = (*chunks) + i;
		free(chunk->data);
	}

	free(*chunks);
}

bool PNGHelper::convertPNGToUseful(unsigned char *buf, int size,
								   unsigned char **newbuf, int *newbufsize)
{
	PNGChunk *chunks;
	int numChunks;

	if (!readPNGChunks(buf, size, &chunks, &numChunks)) {
		return false;
	}

	if (!processPNGChunks(chunks, numChunks)) {
		cleanUp(&chunks, numChunks);
		return false;
	}

	writePNGChunks(chunks, numChunks, newbuf, newbufsize);
	cleanUp(&chunks, numChunks);
	return true;
}
