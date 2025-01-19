
CFLAGS += -Wall -std=c99 -pedantic

.PHONY: all
all: readme_update test

# Dev Note: $ is used by both make and AWK. Must escape $ for use in AWK within makefile.
.PHONY: readme_update
readme_update: multipart_extract
	# Library Version (From clib package metadata)
	jq -r '.version' clib.json | xargs -I{} sed -i 's|<version>.*</version>|<version>{}</version>|' README.md
	jq -r '.version' clib.json | xargs -I{} sed -i 's|<versionBadge>.*</versionBadge>|<versionBadge>![Version {}](https://img.shields.io/badge/version-{}-blue.svg)</versionBadge>|' README.md

	size multipart_extract | awk 'NR==2 {print $$1}' | xargs -I{} sed -i 's|<dotTextSize>.*</dotTextSize>|<dotTextSize>{}</dotTextSize>|' README.md
	size multipart_extract | awk 'NR==2 {print $$2}' | xargs -I{} sed -i 's|<dotDataSize>.*</dotDataSize>|<dotDataSize>{}</dotDataSize>|' README.md
	size multipart_extract | awk 'NR==2 {print $$3}' | xargs -I{} sed -i 's|<dotBSSSize>.*</dotBSSSize>|<dotBSSSize>{}</dotBSSSize>|' README.md

	# Embedded flash data usage based on size of text + data
	size multipart_extract | awk 'NR==2 {print $$1 + $$2}' | xargs -I{} sed -i 's|<flashSizeUsage>.*</flashSizeUsage>|<flashSizeUsage>{}</flashSizeUsage>|' README.md
	# Embedded flash data usage based on size of text + data + bss
	size multipart_extract | awk 'NR==2 {print $$2 + $$3}' | xargs -I{} sed -i 's|<ramSizeUsage>.*</ramSizeUsage>|<ramSizeUsage>{}</ramSizeUsage>|' README.md

.PHONY: multipart_extract
multipart_extract: multipart_extract.c minimal_multipart_parser_embedded.o
	@$(CC) $(CFLAGS) $(LDFLAGS) -g0 -Os $^ -o $@
	size multipart_extract
	./multipart_extract_test.sh

.PHONY: test
test: test.c minimal_multipart_parser_with_debug.o
	@$(CC) $(CFLAGS) $(LDFLAGS) -g2 -O0 $^ -o $@
	size test
	@./test

.PHONY: format
format:
	# pip install clang-format
	clang-format -i *.c
	clang-format -i *.h

.PHONY: clean
clean:
	rm -f *.o *.so
	rm -f multipart_extract
	rm -f test

# Static Library - Standard
minimal_multipart_parser.o: minimal_multipart_parser.c
	@$(CC) $(CFLAGS) $(LDFLAGS) -c $^ -o $@

# Static Library - Embedded - No debug (-g0) and optimize for size (-Os)
minimal_multipart_parser_embedded.o: minimal_multipart_parser.c
	@$(CC) $(CFLAGS) $(LDFLAGS) -c -g0 -Os $^ -o $@

# Static Library - Development - Max debug (-g2) and optimize for compile speed and debuggability (-O0)
minimal_multipart_parser_with_debug.o: minimal_multipart_parser.c
	@$(CC) $(CFLAGS) $(LDFLAGS) -c -g2 -O0 $^ -o $@
