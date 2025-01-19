//
// test.c
//
// Copyright (c) 2024 Brian Khuu
// MIT licensed
//

// This is explicitly written to be very minimal and to use lots of
// global static variables to allow for gauging the embedded
// memory size requirement of this library

#include "minimal_multipart_parser.h"
#include <stdbool.h>
#include <stdio.h>

static MinimalMultipartParserContext state = {0};

int main(void)
{
    int c;
    while ((c = getc(stdin)) != EOF)
    {
        // processor handles incoming stream character by character
        const MultipartParserEvent event = minimal_multipart_parser_process(&state, (char)c);

        // Special Events That Needs Handling
        if (event == MultipartParserEvent_DataBufferAvailable)
        {
            // Data Avaliable To Receive
            for (unsigned int j = 0; j < minimal_multipart_parser_get_data_size(&state); j++)
            {
                const char rx = minimal_multipart_parser_get_data_buffer(&state)[j];
                putc(rx, stdout);
            }
        }
        else if (event == MultipartParserEvent_DataStreamCompleted)
        {
            // Datastream Finished
            break;
        }
    }

    // Stream ended without file?
    return minimal_multipart_parser_is_file_received(&state) ? 0 : 1;
}
