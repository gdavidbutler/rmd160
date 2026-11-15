## RMD160
A small C language implementation of RIPEMD-160 / RIPEMD-320

RIPEMD-160 is a member of the RIPEMD [(RIPE Message Digest)](https://en.wikipedia.org/wiki/RIPEMD) cryptographic hash functions developed in 1996 generating a 160 bit digest.
See [pdf](https://homes.esat.kuleuven.be/~bosselae/ripemd160/pdf/AB-9601/AB-9601.pdf) and [pseudo-code](https://homes.esat.kuleuven.be/~bosselae/ripemd/rmd160.txt).

This implementation was created to provide small code to fit in a memory constrained 32 bit microcontroller.
If your compiler does not implement "unsigned int" as 32 bits, change "unsigned int" in rmd160.c @ typedef unsigned int rmd160_bt; to, perhaps, "unsigned long".

Included is an example driver program, main.c, that reads standard input till end-of-file and writes on standard output a hex representation of the hash.

Built on the same hash core, with no dynamic allocation, are keyed primitives: HMAC ([RFC 2104](https://www.rfc-editor.org/rfc/rfc2104)) and HKDF ([RFC 5869](https://www.rfc-editor.org/rfc/rfc5869)).
HKDF feeds T(n-1) | info | counter through the hash a piece at a time rather than assembling that concatenation in a buffer, which is what lets it avoid allocation.
HMAC also comes keyed: the key's two padded blocks are hashed once into a state the caller allocates and wipes, so a caller authenticating many messages under one key -- a pairwise link -- pays for each message's own blocks and the outer digest, and not again for the key's two blocks every message.
Before returning, every function zeroes, with volatile stores, each object it declares that held key or message material; it promises nothing about the copies a compiler makes in registers, spill slots or temporaries, which C source cannot reach, so the promise is checked by reading the source, not by any one compiler's output.
