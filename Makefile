all:
	gcc -o waves_vanity -O3 -Wall -Iblake2b/sse -Ibase58 curve25519-donna-c64.c blake2b/sse/blake2b.c base58/base58.c sha256.c waves_vanity.c
ref:
	gcc -o waves_vanity -O3 -Wall -Iblake2b/ref -Ibase58 curve25519-donna-c64.c blake2b/ref/blake2b-ref.c base58/base58.c sha256.c waves_vanity.c
clean:
	rm -fR waves_vanity activate env
