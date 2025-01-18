# minimal-multipart-form-data-parser-c

![CI/CD Status Badge](https://github.com/mofosyne/minimal-multipart-form-data-parser-c/actions/workflows/ci.yml/badge.svg)

Minimal multipart/form-data Parser in C. Handles only one file, no validation.
Targeting embedded systems, so aiming for small code size over speed or features.

* library version: <version>0.2.0</version>

## Usage

We have just three functions exposed for your usage:

```c
bool minimal_multipart_parser_process(MinimalMultipartParserContext *context, const char c);
char *minimal_multipart_parser_received_data_buffer(MinimalMultipartParserContext *context);
unsigned int minimal_multipart_parser_received_data_count(MinimalMultipartParserContext *context);
```

Usage Example:

```c
    MinimalMultipartParserContext state = {0};
    for (int i = 0; i < input_multipart_payload_byte_count; i++)
    {
        const char c = input_multipart_payload[i];

        // processor handles incoming stream character by character
        if (minimal_multipart_parser_process(&state, c))
        {
            // On data avaliable this is triggered
            char *received_buffer = minimal_multipart_parser_received_data_buffer(&state);
            unsigned int received_bytes = minimal_multipart_parser_received_data_count(&state);
            // ... 
        }
    }
```

## Size

A small micro utility program was written `multipart_extract` to find
out the minimal expected program size on disk and in ram.

Based on that case study, you can expect this library to consume around <flashSizeUsage>2912</flashSizeUsage> bytes in flash/disk memory storage and <ramSizeUsage>808</ramSizeUsage> bytes in ram usage.

Heres a breakdown of the program sections size usage:

| `.text` | `.data` | `.bss` |
| ---     | ---     | ---    |
| <dotTextSize>2304</dotTextSize> B | <dotDataSize>608</dotDataSize> B | <dotBSSSize>200</dotBSSSize> B |


## Purpose For Existance

For use in very constrant devices

* Use in bootloader (e.g. uboot)
* Use in CGI scripts (e.g. busybox)
* Use in embedded devices (e.g. esp32)

For the purpose these are the restrictions to this code:

* Must be streamable, so don't expect users to read in the whole file first.
* Minimise size of code so it can fit in embedded systems
* Minimise validation checks to keep code size low (aside from security considerations)
* Have no external dependencies. This makes it easier to integrate.

These are not considerations I am taking:

* Speed and cpu efficency is not of concern here. You would not typically be using this because you care about speed.
* Will not be tolerant of only `\n` even if the spec allows for receiving it, in order to minimise code size. 
    - Will only follow CRLF (`\r\n`), because most browsers follow RFC2616.

For all other cases, recommend using an actual webserver.

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
