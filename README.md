dflow
=====

How to compile/install:
just do 'make', and then copy 'dflow' binary to directory like /usr/local/bin

Usage examples:
a. watching dd progres
    # dd if=/dev/sda count=8K bs=4K| dflow -b 4096 -p 33292288 | dd of=/dev/sdb bs=4K
      - 33292288 is 8128 * 4096
