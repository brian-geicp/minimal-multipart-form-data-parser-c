
test: test.c minimal_multipart_parser.c
	@$(CC) -std=c99 $^ -o $@
	@./test

.PHONY: test
