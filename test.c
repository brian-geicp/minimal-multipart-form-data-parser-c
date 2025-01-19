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

// #define DEBUG

char *MultipartParserEvent_To_Str(MultipartParserEvent event)
{
    switch (event)
    {
        case MultipartParserEvent_None:
            return "None";
        case MultipartParserEvent_FileStreamFound:
            return "File Stream Found";
        case MultipartParserEvent_FileStreamStarting:
            return "File Stream Starting";
        case MultipartParserEvent_DataBufferAvailable:
            return "Data Buffer Available";
        case MultipartParserEvent_DataStreamCompleted:
            return "Data Stream Completed";
        case MultipartParserEvent_Error:
            return "Error";
        default:
            return "?";
    }
}

bool test_case(const char *title, const char *input, const unsigned int input_size, const char *expected, const unsigned int expected_byte_count)
{
    bool passed = true;
    char received_file_buffer[1000] = {0};
    unsigned int received_file_byte_count = 0;

    MinimalMultipartParserContext state = {0};
    for (int i = 0; i < input_size; i++)
    {
        const char c = input[i];
#ifdef DEBUG
        printf("[%x : %c]\n", c, isprint(c) ? c : c == '\r' ? 'r' : c == '\n' ? 'n' : '?');
#endif
        const MultipartParserEvent event = minimal_multipart_parser_process(&state, c);
        if (event != MultipartParserEvent_None)
        {
#ifdef DEBUG
            printf("Event: %s\n", MultipartParserEvent_To_Str(event));
#endif
            if (event == MultipartParserEvent_DataBufferAvailable)
            {
                const char *received_buffer = minimal_multipart_parser_received_data_buffer(&state);
                const unsigned int received_bytes = minimal_multipart_parser_received_data_count(&state);
                for (int j = 0; j < received_bytes; j++)
                {
                    received_file_buffer[received_file_byte_count++] = received_buffer[j];
                }
            }
            else if (event == MultipartParserEvent_DataStreamCompleted)
            {
                // Finished
                break;
            }
        }
    }

    if (received_file_byte_count != expected_byte_count)
    {
        passed = false;
    }

    if (memcmp(expected, received_file_buffer, expected_byte_count) != 0)
    {
        passed = false;
    }

    if (!passed)
    {
        printf("Case '%s' Failed\n", title);
        printf("Expected (%d): '%s'\n", expected_byte_count, expected);
        printf("Got (%d): '%s'\n", received_file_byte_count, received_file_buffer);
        printf("\n");
    }
    else
    {
        printf("Case '%s' Passed\n", title);
    }
    return passed;
}

bool test_case1(void)
{
    const char input[] = "POST / HTTP/1.1\r\n"
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
                         "-----------------------------9051914041544843365972754266--\r\n";

    const char expected[] = "text default";

    return test_case("full http standard style", input, strlen(input), expected, strlen(expected));
}

bool test_case2(void)
{
    const char input[] = "POST / HTTP/1.1\r\n"
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

    const char expected[] = "text default";

    return test_case("multipayload", input, strlen(input), expected, strlen(expected));
}

bool test_case3(void)
{
    const char input[] = "-----------------------------9051914041544843365972754266\r\n"
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

    const char expected[] = "text default";

    return test_case("cgi style body only", input, strlen(input), expected, strlen(expected));
}

bool test_case4(void)
{
    const char input[] = "-----------------------------9051914041544843365972754266\r\n"
                         "Content-Disposition: form-data; name=\"text\"\r\n"
                         "\r\n"
                         "\x00"
                         "\x01"
                         "\x02"
                         "\x03"
                         "\r\n"
                         "\x00"
                         "\r\n"
                         "-----------------------------9051914041544843365972754266--\r\n";

    const char expected[] = {0x00, 0x01, 0x02, 0x03, '\r', '\n', 0x00};

    return test_case("binary payload", input, sizeof(input) - 1, expected, sizeof(expected));
}

int main(int argc, char **argv)
{
    printf("Testing Minimal Multipart Form Data Parser\n");
    printf("GCC Version: v%s\n", __VERSION__);

    if (!test_case1())
    {
        return 1;
    }

    if (!test_case2())
    {
        return 1;
    }

    if (!test_case3())
    {
        return 1;
    }

    if (!test_case4())
    {
        return 1;
    }

    printf("PASSED\n");
    return 0;
}
