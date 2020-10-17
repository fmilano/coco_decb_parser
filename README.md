# coco_decb_parser
TRS-80 CoCo DECB file parser. 

From the LWTOOLS manual:

"Each binary starts with a preamble. Each preamble is five bytes long. The first byte is zero. The next two
bytes specify the number of bytes to load and the last two bytes specify the address to load the bytes at.
Then, a string of bytes follows. After this string of bytes, there may be another preamble or a postamble.
A postamble is also five bytes in length. The first byte of the postamble is $FF, the next two are zero, and
the last two are the execution address for the binary."

This parser generates a raw binary file for each section found in the original DECB file.

