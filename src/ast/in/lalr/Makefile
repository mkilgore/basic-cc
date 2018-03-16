
clean-list-y += parser.tab.c
clean-list-y += parser.output
CLEAN_LIST += $(srctree)/parser.tab.h

# Add extra dependency information
#
# This forces make to generate the necessary parser headers before attempting
# to create the dependecy files.

$(srctree)/parser.tab.c: $(srctree)/parser.y
$(srctree)/parser.tab.h: $(srctree)/parser.y
$(srctree)/lexer.c: $(srctree)/lexer.l
$(srctree)/lexer.l: $(srctree)/parser.tab.h

objs-y += parser.tab.o
objs-y += lexer.o

clean-list-y += lexer.c

# Tell make not to delete the temporary parser files.
.SECONDARY: $(patsubst %,$(srctree)/%,$(clean-list-y))
