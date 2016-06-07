# chtml
Little (hacky) program that will compile a CHTML document to C.

## Usage:
An example CHTML document is provided (index.chtml). To compile the file:
./main index.chtml

It will output some information regarding the passing of the file, and there
will now be a temp.c file that can be compiled. The executable from temp.c
will output the HTML.
