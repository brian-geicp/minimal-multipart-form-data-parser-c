# minimal-multipart-form-data-parser-c

<versionBadge>![Version 0.3.0](https://img.shields.io/badge/version-0.3.0-blue.svg)</versionBadge>
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C](https://img.shields.io/badge/Language-C-blue.svg)](https://en.wikipedia.org/wiki/C_(programming_language))
[![CI/CD Status Badge](https://github.com/mofosyne/minimal-multipart-form-data-parser-c/actions/workflows/ci.yml/badge.svg)](https://github.com/mofosyne/minimal-multipart-form-data-parser-c/actions)

Minimal multipart/form-data Parser in C. Handles only one file, no validation.
Targeting embedded systems, so aiming for small code size over speed or features.

In short it reads in a http stream for example ([example lifted from here](https://stackoverflow.com/questions/4238809/example-of-multipart-form-data)):

```bash
POST / HTTP/1.1
Host: localhost:8000
... shorted for brevity ...
Content-Type: multipart/form-data; boundary=---------------------------9051914041544843365972754266
Content-Length: 554

-----------------------------9051914041544843365972754266
Content-Disposition: form-data; name="text"

text default
-----------------------------9051914041544843365972754266
... shorted for brevity ...
```

And outputs just the content of the first file it sees. Ergo... `text default`.


## Usage

We expose the following function for your usage:

```c
MultipartParserEvent minimal_multipart_parser_process(MinimalMultipartParserContext *context, const char c);
```

This `minimal_multipart_parser_process` function returns one of the following `MultipartParserEvent` events

* `MultipartParserEvent_None` : No event has occurred; unsure whether a file stream is incoming.
* `MultipartParserEvent_FileStreamFound` : A multipart file stream has been detected; the file is expected.
* `MultipartParserEvent_FileStreamStarting` : A multipart file stream is starting; you may want to open a file handler to write the data.
* `MultipartParserEvent_DataBufferAvailable` : Incoming data available. This is typically a byte, but can sometimes be more.
* `MultipartParserEvent_DataStreamCompleted` : The file stream has ended. You may want to close the file handler.

These are to be used when `MultipartParserEvent_DataBufferAvailable` is found:

```c
unsigned int minimal_multipart_parser_get_data_size(const MinimalMultipartParserContext *context);
char *minimal_multipart_parser_get_data_buffer(const MinimalMultipartParserContext *context);
```

This is to be used when the incoming stream has ended to check if the file is ready to use.

```c
bool minimal_multipart_parser_is_file_received(const MinimalMultipartParserContext *context);
```

Usage Example:

```c
int c;
while ((c = getc(stdin)) != EOF)
{
    // Processor handles incoming stream character by character
    const MultipartParserEvent event = minimal_multipart_parser_process(&state, (char)c);

    // Handle Special Events
    if (event == MultipartParserEvent_DataBufferAvailable)
    {
        // Data Available To Receive
        for (unsigned int j = 0; j < minimal_multipart_parser_get_data_size(&state); j++)
        {
            const char rx = minimal_multipart_parser_get_data_buffer(&state)[j];
            // Output received data
            putc(rx, stdout);
        }
    }
    else if (event == MultipartParserEvent_DataStreamCompleted)
    {
        // Data Stream Finished
        break;
    }
}

// Check if file has been received
if (minimal_multipart_parser_is_file_received(&state))
{
    // File Received Successfully
}
else
{
    // File Reception Failed
}

```

## Size

A small micro utility program was written `multipart_extract` to find
out the minimal expected program size on disk and in ram.

Based on that case study, you can expect this library to consume around <flashSizeUsage>2841</flashSizeUsage> bytes in flash/disk memory storage and <ramSizeUsage>808</ramSizeUsage> bytes in ram usage.

Heres a breakdown of the program sections size usage:

| `.text` | `.data` | `.bss` |
| ---     | ---     | ---    |
| <dotTextSize>2233</dotTextSize> B | <dotDataSize>608</dotDataSize> B | <dotBSSSize>200</dotBSSSize> B |


## Purpose For Existence

For use in very constrant devices

* Use in bootloader (e.g. uboot)
* Use in CGI scripts (e.g. busybox)
* Use in embedded devices (e.g. esp32)

For this purpose these are the restrictions to this code:

* Stick to C99 standard
* Keep standard library usage to minimum to control code size to a predictable level
* Must be streamable, so don't expect users to read in the whole file first.
* Minimise size of code so it can fit in embedded systems
* Minimise validation checks to keep code size low (aside from security considerations)
* Have no external dependencies. This makes it easier to integrate.

These are not considerations I am taking:

* Speed and cpu efficency is not of concern here. You would not typically be using this because you care about speed.
* Will not be tolerant of only `\n` even if the spec allows for receiving it, in order to minimise code size. 
    - Will only follow CRLF (`\r\n`), because most browsers follow RFC2616.

To that end I was able to:

* Only need `stdbool.h` from the standard C99 library or higher
* No malloc was required
* Inputs can be streamed byte by byte
* Buffering size kept to around the size of the boundary. Ergo `\r\n--` plus up to 70 characters.

For all other cases, recommend using an actual webserver.

## Other Similar Repositories To Consider

All these are C based repositories that are more developed with more features than our 
minimal implementation that you may want to consider if you need more features than this repo.

* [iafonov/multipart-parser-c](https://github.com/iafonov/multipart-parser-c)
    - Uses malloc
    - Uses callback functions on each data reception
    - You need to parse `Content-Type` from the response head yourself to get the boundary
        - This is because the init function of that implementation requires it.
        - Ours simply assumes that the first line of this form `\r\n--BOUNDARY\r\n` is the boundary which is a safe assumption to make

* [francoiscolas/multipart-parser](https://github.com/francoiscolas/multipart-parser)
    - Does not appear to use malloc
    - Uses callback functions on each data reception
    - Validates and throws an error if not of the exact form. Ours doesn't not for code size consideration.
    - You need to parse `Content-Type` from the response head yourself to get the boundary
        - This is because the init function of that implementation requires it.
        - Ours simply assumes that the first line of this form `\r\n--BOUNDARY\r\n` is the boundary which is a safe assumption to make

* [abiiranathan/libmultipart](https://github.com/abiiranathan/libmultipart)
    - Uses malloc
    - Assumes you have already stored the whole request body in memory, ours will process the http request byte by byte.
    - But has extra features and validation for processing filenames, mimetype, etc...

## Assumptions

* You only care about one file
* You don't care about the metadata
* You will inspect the file type by inspecting the file itself
    - In most cases that can be done via the magic number and checking the file structure etc...
* That http multipart-form boundary will always start with `\r\n--` followed by a sequence of printable ASCII character (7bit range ascii only) and ending with a `\r\n` or `--\r\n` to indicate end of transmission.
    - This gives us the ability to take a shortcut and not bother with parsing `Content-Type`
    - This also means if it's missing, then we can assume this
    - We only care about one file... so we can skip the logic of finding the `\r\n` marker and stop search immediately.

## Reference:

* Test Case 1 taken from <https://stackoverflow.com/questions/4238809/example-of-multipart-form-data>
* Good reading <https://stackoverflow.com/questions/27993445/is-this-a-well-formed-multipart-form-data-request>
    - Start of multipart boundary is `\r\n--BOUNDARY` where BOUNDARY is specified in `Content-Type`
        - But this marker is very distinctive, so don't really need to check `Content-Type`
