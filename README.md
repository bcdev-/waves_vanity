# Waves vanity generator
<img src="https://wavesplatform.com/images/logo.svg" width=100 height=100/>

A vanity address generator for the Waves Platform.

Windows binary: https://github.com/bcdev-/waves_vanity/raw/master/releases/wavesvanity-v1.0.2-win64.zip

Linux binary: https://github.com/bcdev-/waves_vanity/raw/master/releases/wavesvanity-v1.0.2-linux64

## Example
```
bc@d:~/repo/waves_vanity$ ./wavesvanity -m ______________________________waves -c ______________________________xxxxx
Vanity miner settings:
CPU threads: 4
Network: Mainnet
Char mask: ______________________________waves
Case mask: ______________________________xxxxx

Iterations expected: 10365377
Starting workers...
Iterations: 12566448   Elapsed time: 0 d 0 h 2 m 52 s   Speed: 73000.48 keys/second   50% chance of finding: 0 d 0 h 2 m 21 s          
Overall iterations: 12602522
Address: 3P3KV14kmZRTueqxHXKbgzNCfv3HMsWAVEs
Password: cSLYycKbec4VMh1m9RE1E4WVB6k
```

## Usage options
```
Usage:
  ./wavesvanity [OPTION...]

Help Options:
  -h                Show help options

Waves Options:
  -n {t or m}         t - Testnet, m - Mainnet [default]

Mask Options:
  -m {mask}         Char mask. _ is 'any character at this position'.
  Any other character means 'this specific character at this position'.
Example: './wavesvanity -m ____eaaa' may generate address 3MvMeaaaLm32f5JzsQQxYhqKL2fbrEQStCs.

  -c {mask}         Case mask. ? means 'any case for a character'
  _ means 'this specific case for a character'
The default case mask is: ___________________________________
Case mask can consist only of _, u, l, n, x and p characters.
They mean:
  n - any number
  x - a character from character mask of any case [ex. a or A, c or C]
  u - any uppercase character
  l - any lowercase character
  p - any uppercase character or a number
  _ - exactly the same character as in character mask.

Example: './wavesvanity -m _____________________________N____N -c _____________________________xnnnnx'
         A sample result: 3PN7C7rasDZr4C48SWeiQbHjPmM4xN2892n
Example: './wavesvanity -c ppppppppppppppppppppppppppppppppppp -n t'
         A sample result: 3NCL45V25RUUXQ7YKB41G3ACW3KTUFPVYUV
```

## How to compile

```
make
./wavesvanity -h
```

## Changelog

1.0.1:
- Added 95% chance calculation.
- Fixed `_x` mask probability calculations when the letters were l, o or i.

