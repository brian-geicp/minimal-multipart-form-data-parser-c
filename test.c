//
// test.c
//
// Copyright (c) 2024 Brian Khuu
// MIT licensed
//

#include "minimal_multipart_parser.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

bool test_case1(void)
{
    const char input_multipart_payload[] = "POST / HTTP/1.1\r\n"
                                           "Host: localhost:8000\r\n"
                                           "User-Agent: Mozilla/5.0 (X11; Ubuntu; Linux i686; rv:29.0) Gecko/20100101 Firefox/29.0\r\n"
                                           "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n"
                                           "Accept-Language: en-US,en;q=0.5\r\n"
                                           "Accept-Encoding: gzip, deflate\r\n"
                                           "Cookie: __atuvc=34%7C7; permanent=0; _gitlab_session=226ad8a0be43681acf38c2fab9497240; __profilin=p%3Dt; request_method=GET\r\n"
                                           "Connection: keep-alive\r\n"
                                           "Content-Type: multipart/form-data; boundary=---------------------------9051914041544843365972754266\r\n"
                                           "Content-Length: 554\r\n"
                                           "\r\n"
                                           "-----------------------------9051914041544843365972754266\r\n"
                                           "Content-Disposition: form-data; name=\"text\"\r\n"
                                           "\r\n"
                                           "text default\r\n"
                                           "-----------------------------9051914041544843365972754266\r\n"
                                           "Content-Disposition: form-data; name=\"file1\"; filename=\"a.txt\"\r\n"
                                           "Content-Type: text/plain\r\n"
                                           "\r\n"
                                           "Content of a.txt.\r\n"
                                           "\r\n"
                                           "-----------------------------9051914041544843365972754266\r\n"
                                           "Content-Disposition: form-data; name=\"file2\"; filename=\"a.html\"\r\n"
                                           "Content-Type: text/html\r\n"
                                           "\r\n"
                                           "<!DOCTYPE html><title>Content of a.html.</title>\r\n"
                                           "\r\n"
                                           "-----------------------------9051914041544843365972754266--\r\n";
    const unsigned int input_multipart_payload_byte_count = sizeof(input_multipart_payload) - 1;

    const char expected_response[] = "Content of a.txt.";
    const unsigned int expected_response_byte_count = sizeof(expected_response) - 1;

    char received_file_buffer[1000] = {0};
    unsigned int received_file_byte_count = 0;

    MinimalMultipartParserContext state = {0};

    for (int i = 0; i < input_multipart_payload_byte_count; i++)
    {
        const char c = input_multipart_payload[i];
        if (minimal_multipart_parser_process(&state, c))
        {
            char *received_buffer = minimal_multipart_parser_received_data_buffer(&state);
            unsigned int received_bytes = minimal_multipart_parser_received_data_count(&state);
            for (int j = 0; j < received_bytes; j++)
            {
                received_file_buffer[received_file_byte_count++] = received_buffer[j];
            }
        }
    }

    // Check if match
    if (memcmp(expected_response, received_file_buffer, expected_response_byte_count) != 0 && expected_response_byte_count == received_file_byte_count)
    {
        printf("Case 1 Failed\n");
        printf("Expected: %s\n", expected_response);
        printf("Got: %s\n", received_file_buffer);
        return false;
    }

    return true;
}

int main(int argc, char **argv)
{
    if (!test_case1())
    {
        return 1;
    }

    printf("TEST OK\n");
    return 0;
}
