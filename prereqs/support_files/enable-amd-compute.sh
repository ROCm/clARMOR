#!/bin/bash
# Mechanism adapted from code at https://wiki.tiker.net/OpenCLHowTo
XAUTHPORT=$(echo $DISPLAY | cut -d'.' -f1 |  cut -d':' -f2)
xauth extract /tmp/x11-auth-file ":$XAUTHPORT"
chmod 660 /tmp/x11-auth-file
chgrp amdgpu /tmp/x11-auth-file

cat > /tmp/enable-amd-compute.sh <<EOF
export COMPUTE=$DISPLAY
export XAUTHORITY=/tmp/x11-auth-file
xauth extract /tmp/temp_xauth :$XAUTHPORT
cur_name=\`whoami\`
fake_home=\`eval echo ~\$cur_name\`
export XAUTHORITY=\$fake_home/.Xauthority
xauth merge /tmp/temp_xauth
rm /tmp/temp_xauth
EOF
