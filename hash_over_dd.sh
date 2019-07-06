#!/bin/sh


# dd if=to-pack.db | openssl dgst -md5 -binary  |  openssl enc -base64
# will produce the same result as:
# gsutil hash -m to-pack.db


# so run this like so to get part of a file:
#	dd if=/export/sdb/home/vsrinivas/to-upload/1.img \
#		bs=1G \
#		count=4 \
#		skip=448 | ./hash_over_dd.sh

openssl dgst -md5 -binary | openssl enc -base64
