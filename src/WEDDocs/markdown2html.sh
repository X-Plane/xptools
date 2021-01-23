#!/bin/sh

# unfortunately multimarkdown uses an upper case class name for the index,
# while Laminars css expects a lower case. So we replace this with sed:
multimarkdown -f toc.mmd index.markdown | sed 's/\"TOC\"/\"toc\"/' >index.html

# pandoc uses a different style to generate heading identifiers - which replaces
# spaces by dashes, breaking all document internal links
# pandoc -s --toc index.markdown -o index.html

