set -xe

fold -sw 80 "$1" > "$1.tmp"
mv $1.tmp $1
