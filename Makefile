
test: test.c minimal_multipart_parser.c
	@$(CC) -std=c99 $^ -o $@
	@./test

.PHONY: test

.PHONY: format
format:
	# pip install clang-format
	clang-format -i *.c
	clang-format -i *.h
