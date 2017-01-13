OS := $(shell uname)
ifeq ($(OS),Darwin)
STATIC := 
else
STATIC := -static
endif

static:
	gcc -std=gnu99 $(STATIC) -o wavesvanity -O3 -Wall -Iblake2b/sse -pthread -msse2 -m64 -Icurve25519-donna -Ibase58 curve25519-donna/curve25519.c blake2b/sse/blake2b.c base58/base58.c sha256.c waves_vanity.c
upx:
	upx --brute wavesvanity
all:
	gcc -std=gnu99 -o wavesvanity -O3 -Wall -Iblake2b/sse -pthread -msse2 -m64 -Icurve25519-donna -Ibase58 curve25519-donna/curve25519.c blake2b/sse/blake2b.c base58/base58.c sha256.c waves_vanity.c
ref:
	gcc -std=gnu99 -o wavesvanity -O3 -Wall -Iblake2b/ref -pthread -Ibase58 -Icurve25519-donna curve25519-donna/curve25519.c blake2b/ref/blake2b-ref.c base58/base58.c sha256.c waves_vanity.c
clean:
	rm -fR *.s waves_vanity activate env
