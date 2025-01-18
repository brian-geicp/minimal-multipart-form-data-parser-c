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
        const MultipartParserEvent event = minimal_multipart_parser_process(&state, (char)c);
        if (event == MultipartParserEvent_DataBufferAvailable) {
            const char *received_buffer = minimal_multipart_parser_received_data_buffer(&state);
            unsigned int received_bytes = minimal_multipart_parser_received_data_count(&state);
            for (int j = 0; j < received_bytes; j++)
            {
                putc(received_buffer[j], stdout);
            }
        }
    }

    return 0;
}
