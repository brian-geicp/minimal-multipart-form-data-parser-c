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

#define clamp_upper(value, max) ((value) < (max) ? (value) : (max))
#define clamp_lower(value, min) ((value) > (min) ? (value) : (min))
#define clamp_range(value, min, max) clamp_lower(min, clamp_upper(value, max))

static inline unsigned int buffer_count(MinimalMultipartParserCharBuffer *context) { return context->count; }

static void buffer_reset(MinimalMultipartParserCharBuffer *context)
{
    context->buffer[0] = '\0';
    context->count = 0;
}

static bool buffer_add(MinimalMultipartParserCharBuffer *context, const char c)
{
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

// Return true if data available to be picked up from data buffer
MultipartParserEvent minimal_multipart_parser_process(MinimalMultipartParserContext *context, const char c)
{
    MinimalMultipartParserCharBuffer *boundaryBuffer = &(context->boundary);
    MinimalMultipartParserCharBuffer *dataBuffer = &(context->data);

    if (context->data_available)
    {
        buffer_reset(dataBuffer);
        context->data_available = false;
    }

    if (context->phase == MultipartParserPhase_FindBoundaryStart_INIT)
    {
        switch (c)
        {
            case '\r':
                context->phase = MultipartParserPhase_FindBoundaryStart_CR;
                return MultipartParserEvent_None;
            case '\n':
                context->phase = MultipartParserPhase_FindBoundaryStart_LF;
                return MultipartParserEvent_None;
            case '-':
                context->phase = MultipartParserPhase_FindBoundaryStart_D1;
                return MultipartParserEvent_None;
            default:
                context->phase = MultipartParserPhase_FindBoundaryStart_SKIP_LINE;
                return MultipartParserEvent_None;
        }
    }

    if (context->phase == MultipartParserPhase_FindBoundaryStart_SKIP_LINE)
    {
        switch (c)
        {
            case '\r':
                context->phase = MultipartParserPhase_FindBoundaryStart_CR;
                return MultipartParserEvent_None;
            case '\n':
                context->phase = MultipartParserPhase_FindBoundaryStart_LF;
                return MultipartParserEvent_None;
            default:
                return MultipartParserEvent_None;
        }
    }

    if (context->phase == MultipartParserPhase_FindBoundaryStart_CR)
    {
        switch (c)
        {
            case '\n':
                context->phase = MultipartParserPhase_FindBoundaryStart_LF;
                return MultipartParserEvent_None;
            default:
                context->phase = MultipartParserPhase_FindBoundaryStart_SKIP_LINE;
                return MultipartParserEvent_None;
        }
    }

    if (context->phase == MultipartParserPhase_FindBoundaryStart_LF)
    {
        switch (c)
        {
            case '-':
                context->phase = MultipartParserPhase_FindBoundaryStart_D1;
                return MultipartParserEvent_None;
            default:
                context->phase = MultipartParserPhase_FindBoundaryStart_SKIP_LINE;
                return MultipartParserEvent_None;
        }
    }

    if (context->phase == MultipartParserPhase_FindBoundaryStart_D1)
    {
        // Previously got a dash in '\r\n--', seeking another dash
        switch (c)
        {
            case '-':
                context->phase = MultipartParserPhase_ReadBoundaryStart;
                buffer_add(boundaryBuffer, '\r');
                buffer_add(boundaryBuffer, '\n');
                buffer_add(boundaryBuffer, '-');
                buffer_add(boundaryBuffer, '-');
                return MultipartParserEvent_None;
            default:
                context->phase = MultipartParserPhase_FindBoundaryStart_SKIP_LINE;
                return MultipartParserEvent_None;
        }
    }

    if (context->phase == MultipartParserPhase_ReadBoundaryStart)
    {
        switch (c)
        {
            case '\r':
                context->phase = MultipartParserPhase_SkipBoundaryStart_LF;
                return MultipartParserEvent_None;
            case '\n':
                context->phase = MultipartParserPhase_SkipFileHeader;
                return MultipartParserEvent_FileStreamFound;
            default:
                buffer_add(boundaryBuffer, c);
                return MultipartParserEvent_None;
        }
    }

    if (context->phase == MultipartParserPhase_SkipBoundaryStart_LF)
    {
        switch (c)
        {
            case '\n':
                context->phase = MultipartParserPhase_SkipFileHeader;
                return MultipartParserEvent_FileStreamFound;
            default:
                return MultipartParserEvent_None;
        }
    }

    if (context->phase == MultipartParserPhase_SkipFileHeader)
    {
        const unsigned expected_index = buffer_count(dataBuffer);
        const char expected_char = (MINIMAL_MULTIPART_PARSER_FILE_START_MARKER)[expected_index];
        if (c != expected_char)
        {
            buffer_reset(dataBuffer);
            return MultipartParserEvent_None;
        }

        buffer_add(dataBuffer, c);
        if (buffer_count(dataBuffer) >= MINIMAL_MULTIPART_PARSER_FILE_START_MARKER_COUNT)
        {
            context->phase = MultipartParserPhase_GetFileBytes;
            buffer_reset(dataBuffer);
            return MultipartParserEvent_FileStreamStarting;
        }

        return MultipartParserEvent_None;
    }

    if (context->phase == MultipartParserPhase_GetFileBytes)
    {
        const unsigned char full_boundary_size = buffer_count(boundaryBuffer);
        const char *full_boundary_string = context->boundary.buffer;

        const unsigned expected_index = buffer_count(dataBuffer);
        const char expected_char = full_boundary_string[expected_index];

        buffer_add(dataBuffer, c);

        if (c != expected_char)
        {
            context->data_available = true;
            return MultipartParserEvent_DataBufferAvailable;
        }

        if (buffer_count(dataBuffer) >= full_boundary_size)
        {
            context->phase = MultipartParserPhase_EndOfFile;
            return MultipartParserEvent_DataStreamCompleted;
        }

        return MultipartParserEvent_None;
    }

    if (context->phase == MultipartParserPhase_EndOfFile)
    {
        // Do nothing... We already got the file now
        return MultipartParserEvent_None;
    }

    return MultipartParserEvent_Error;
}
