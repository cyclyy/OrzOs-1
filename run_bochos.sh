#!/bin/bash

# run_bochs.sh
# mounts the correct loopback device, runs bochs, then unmounts.

sudo ./update_image.sh
bochs -q -f bochsrc.txt
