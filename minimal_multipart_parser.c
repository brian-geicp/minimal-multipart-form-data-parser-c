//
// minimal_multipart_parser.c
//
// Copyright (c) 2024 Brian Khuu
// MIT licensed
//
// https://github.com/mofosyne/minimal-multipart-form-data-parser-c
//

#include "minimal_multipart_parser.h"
#include <stdbool.h>

#include <stdio.h>

static inline unsigned int buffer_count(MinimalMultipartParserCharBuffer *context) { return context->count; }

static bool buffer_reset(MinimalMultipartParserCharBuffer *context)
{
    context->buffer[0] = '\0';
    context->count = 0;
}

static bool buffer_add(MinimalMultipartParserCharBuffer *context, const char c)
{
    if (c == '\0')
    {
        return false;
    }

    if (context->count >= MINIMAL_MULTIPART_PARSER_FULL_BOUNDARY_BUFFER_MAX_CHAR)
    {
        return false;
    }

    context->buffer[context->count] = c;
    context->buffer[context->count + 1] = '\0';
    context->count++;
    return true;
}

char *minimal_multipart_parser_received_data_buffer(MinimalMultipartParserContext *context) { return context->data.buffer; }

unsigned int minimal_multipart_parser_received_data_count(MinimalMultipartParserContext *context) { return context->data.count; }

// Return true if data avaliable to be picked up from data buffer
bool minimal_multipart_parser_process(MinimalMultipartParserContext *context, const char c)
{
    MinimalMultipartParserCharBuffer *boundaryBuffer = &(context->boundary);
    MinimalMultipartParserCharBuffer *dataBuffer = &(context->data);

    if (context->data_avaliable)
    {
        buffer_reset(dataBuffer);
        context->data_avaliable = false;
    }

    if (context->phase == MultipartParserPhase_FindBoundaryStart)
    {
        const unsigned expected_index = buffer_count(boundaryBuffer);
        const char expected_char = (MINIMAL_MULTIPART_PARSER_BOUNDARY_START_MARKER)[expected_index];
        if (c != expected_char)
        {
            buffer_reset(boundaryBuffer);
            return false;
        }

        buffer_add(boundaryBuffer, c);
        if (buffer_count(boundaryBuffer) >= MINIMAL_MULTIPART_PARSER_BOUNDARY_START_MARKER_COUNT)
        {
            context->phase = MultipartParserPhase_ReadBoundaryStart;
            return false;
        }

        return false;
    }

    if (context->phase == MultipartParserPhase_ReadBoundaryStart)
    {
        if (c == '\r')
        {
            context->phase = MultipartParserPhase_SkipBoundaryStart_LF;
            return false;
        }

        buffer_add(boundaryBuffer, c);
        return false;
    }

    if (context->phase == MultipartParserPhase_SkipBoundaryStart_LF)
    {
        if (c == '\n')
        {
            context->phase = MultipartParserPhase_SkipFileHeader;
            return false;
        }

        return false;
    }

    if (context->phase == MultipartParserPhase_SkipFileHeader)
    {
        const unsigned expected_index = buffer_count(dataBuffer);
        const char expected_char = (MINIMAL_MULTIPART_PARSER_FILE_START_MARKER)[expected_index];
        if (c != expected_char)
        {
            buffer_reset(dataBuffer);
            return false;
        }

        buffer_add(dataBuffer, c);
        if (buffer_count(dataBuffer) >= MINIMAL_MULTIPART_PARSER_FILE_START_MARKER_COUNT)
        {
            context->phase = MultipartParserPhase_GetFileBytes;
            buffer_reset(dataBuffer);
            return false;
        }

        return false;
    }

    if (context->phase == MultipartParserPhase_GetFileBytes)
    {
        const unsigned char full_boundary_size = buffer_count(&(context->boundary));
        const char *full_boundary_string = context->boundary.buffer;

        const unsigned expected_index = buffer_count(dataBuffer);
        const char expected_char = full_boundary_string[expected_index];

        buffer_add(dataBuffer, c);

        if (c == expected_char)
        {
            if (buffer_count(dataBuffer) >= MINIMAL_MULTIPART_PARSER_FILE_START_MARKER_COUNT)
            {
                context->phase = MultipartParserPhase_EndOfFile;
                buffer_reset(dataBuffer);
            }

            return false;
        }

        context->data_avaliable = true;
        return true;
    }

    if (context->phase == MultipartParserPhase_EndOfFile)
    {
        // Do nothing... We already got the file now
        return false;
    }

    return false;
}
