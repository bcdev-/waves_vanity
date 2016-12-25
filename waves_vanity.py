#!/usr/bin/env python3
from pyblake2 import blake2b
from sha3 import sha3_256
from base58 import b58decode, b58encode
import curve25519, base58


base58_public_key_max_length = 50


class BlockchainParameters:
    def __init__(self, testnet):
        self.testnet = testnet
        self.AddressVersion = b'\x01'
        self.ChainId = b'T' if testnet else b'W'
        self.HashLength = 20
        self.ChecksumLength = 4


def blake2b256_keccak256(data):
	b = blake2b(digest_size=32)
	b.update(data)
	lol = b.digest()
	return sha3_256(lol).digest()


def public_key_to_account(public_key, params=BlockchainParameters(testnet=False)):

    assert(len(public_key) <= base58_public_key_max_length)
    public_key = b58decode(public_key)
    out = ""
    for i in public_key:
        out += "0x%.2x, " % i
    print(out, params.testnet)
    public_key_hash = blake2b256_keccak256(public_key)[:params.HashLength]
    without_checksum = params.AddressVersion + params.ChainId + public_key_hash
    print(b58encode(without_checksum))
    return b58encode(without_checksum + blake2b256_keccak256(without_checksum)[:params.ChecksumLength])


def lol(public_key, params=BlockchainParameters(testnet=False)):

    public_key_hash = blake2b256_keccak256(public_key)[:params.HashLength]
    without_checksum = params.AddressVersion + params.ChainId + public_key_hash
    print(b58encode(without_checksum))
    return b58encode(without_checksum + blake2b256_keccak256(without_checksum)[:params.ChecksumLength])


print(public_key_to_account("FkoFqtAeibv2E6Y86ZDRfAkZz61LwUMjLAP2gmS1j7xe",BlockchainParameters(True)))
print("Should be", "3Mv61qe6egMSjRDZiiuvJDnf3Q1qW9tTZDB")
print(public_key_to_account("FZZi4z9TVmev2zh6GyxLQWsieXDaeGthzWhgrQYcv6Ci",BlockchainParameters(False)))
print("Should be", "3PAtGGSLnHJ3wuK8jWPvAA487pKamvQHyQw")
'''
b = blake2b(digest_size=32)
b.update(b"a")
print(b.digest())

digest = blake2b256_keccak256(b"A nice, long test to make the day great! :-)")
'''
import hashlib
from array import array

key = b"b"
k = blake2b256_keccak256(b"\x00\x00\x00\x00" + key)
print("k", k)

privkey = array('B', hashlib.sha256(k).digest())
privkey[0] &= 248;
privkey[31] &= 127;
privkey[31] |= 64;
print("pk", privkey)
gateway_public_key = curve25519.public(privkey.tobytes())
print(lol(gateway_public_key, BlockchainParameters(True)))
'''
out = ""
for i in b'\x92\xf2\xc1q\xcb`x\xe6\x05P\xcb\x99S\xfc?\x11\x801\xd61L\xb6@\r\xfdr\x11\xf6\x01\x8d\x1d+':
	out += "0x%.2x, " % i
print(out)

'''

