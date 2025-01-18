//
// minimal_multipart_parser.h
//
// Copyright (c) 2024 Brian Khuu
// MIT licensed
//
// https://github.com/mofosyne/minimal-multipart-form-data-parser-c
//

#ifndef MINIMAL_MULTIPART_PARSER_H
#define MINIMAL_MULTIPART_PARSER_H

#include "minimal_multipart_parser.h"
#include <stdbool.h>

// Size of the full boundary string we are searching for as a multipart file divider
// e.g. `\r\n--BOUNDARY` where BOUNDARY is a user specified 70 bytes long printable ascii string
#define MINIMAL_MULTIPART_PARSER_USER_BOUNDARY_SIZE (70)
#define MINIMAL_MULTIPART_PARSER_FULL_BOUNDARY_BUFFER_MAX_CHAR (4 + MINIMAL_MULTIPART_PARSER_USER_BOUNDARY_SIZE)

#define MINIMAL_MULTIPART_PARSER_BOUNDARY_START_MARKER "\r\n--"
#define MINIMAL_MULTIPART_PARSER_BOUNDARY_START_MARKER_COUNT (sizeof(MINIMAL_MULTIPART_PARSER_BOUNDARY_START_MARKER) - 1)

#define MINIMAL_MULTIPART_PARSER_FILE_START_MARKER "\r\n\r\n"
#define MINIMAL_MULTIPART_PARSER_FILE_START_MARKER_COUNT (sizeof(MINIMAL_MULTIPART_PARSER_FILE_START_MARKER) - 1)

#define MINIMAL_MULTIPART_PARSER_MAX_CHAR (MINIMAL_MULTIPART_PARSER_FULL_BOUNDARY_BUFFER_MAX_CHAR)

typedef enum MultipartParserEvent
{
    MultipartParserEvent_None,
    MultipartParserEvent_FileStreamFound,
    MultipartParserEvent_FileStreamStarting,
    MultipartParserEvent_DataBufferAvailable,
    MultipartParserEvent_DataStreamCompleted,
    MultipartParserEvent_Error,
} MultipartParserEvent;

typedef enum MultipartParserPhase
{
    MultipartParserPhase_FindBoundaryStart_INIT,
    MultipartParserPhase_FindBoundaryStart_SKIP_LINE,
    MultipartParserPhase_FindBoundaryStart_CR,
    MultipartParserPhase_FindBoundaryStart_LF,
    MultipartParserPhase_FindBoundaryStart_D1,
    MultipartParserPhase_ReadBoundaryStart,
    MultipartParserPhase_SkipBoundaryStart_LF,
    MultipartParserPhase_SkipFileHeader,
    MultipartParserPhase_GetFileBytes,
    MultipartParserPhase_EndOfFile
} MultipartParserPhase;

typedef struct MinimalMultipartParserCharBuffer
{
    char buffer[MINIMAL_MULTIPART_PARSER_MAX_CHAR + 1];
    unsigned int count;
} MinimalMultipartParserCharBuffer;

typedef struct MinimalMultipartParserContext
{
    MultipartParserPhase phase;
    MinimalMultipartParserCharBuffer boundary;
    MinimalMultipartParserCharBuffer data;

    bool data_available;
} MinimalMultipartParserContext;

MultipartParserEvent minimal_multipart_parser_process(MinimalMultipartParserContext *context, const char c);
char *minimal_multipart_parser_received_data_buffer(MinimalMultipartParserContext *context);
unsigned int minimal_multipart_parser_received_data_count(MinimalMultipartParserContext *context);

#endif
