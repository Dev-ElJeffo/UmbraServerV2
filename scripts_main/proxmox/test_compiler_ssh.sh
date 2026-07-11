#!/bin/bash
for p in '' '123456789' '123456' 'root'; do
  echo -n "pass=[$p] -> "
  sshpass -p "$p" ssh -o StrictHostKeyChecking=no -o ConnectTimeout=5 \
    -o PreferredAuthentications=password -o PubkeyAuthentication=no \
    root@192.168.3.10 'echo OK' 2>&1 | tail -1
done
