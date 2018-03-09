
objs-y += parser.o
objs-y += lexer.o

clean-list-y += lexer.c

# Tell make not to delete the temporary parser files.
.SECONDARY: $(patsubst %,$(srctree)/%,$(clean-list-y))

