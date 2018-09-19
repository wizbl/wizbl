#!/bin/sh

TOPDIR=${TOPDIR:-$(git rev-parse --show-toplevel)}
SRCDIR=${SRCDIR:-$TOPDIR/src}
MANDIR=${MANDIR:-$TOPDIR/doc/man}

WIZBLCOIND=${WIZBLCOIND:-$SRCDIR/wizblcoind}
WIZBLCOINCLI=${WIZBLCOINCLI:-$SRCDIR/wizblcoin-cli}
WIZBLCOINTX=${WIZBLCOINTX:-$SRCDIR/wizblcoin-tx}
WIZBLCOINQT=${WIZBLCOINQT:-$SRCDIR/qt/wizblcoin-qt}

[ ! -x $WIZBLCOIND ] && echo "$WIZBLCOIND not found or not executable." && exit 1

# The autodetected version git tag can screw up manpage output a little bit
BTCVER=($($WIZBLCOINCLI --version | head -n1 | awk -F'[ -]' '{ print $6, $7 }'))

# Create a footer file with copyright content.
# This gets autodetected fine for wizblcoind if --version-string is not set,
# but has different outcomes for wizblcoin-qt and wizblcoin-cli.
echo "[COPYRIGHT]" > footer.h2m
$WIZBLCOIND --version | sed -n '1!p' >> footer.h2m

for cmd in $WIZBLCOIND $WIZBLCOINCLI $WIZBLCOINTX $WIZBLCOINQT; do
  cmdname="${cmd##*/}"
  help2man -N --version-string=${BTCVER[0]} --include=footer.h2m -o ${MANDIR}/${cmdname}.1 ${cmd}
  sed -i "s/\\\-${BTCVER[1]}//g" ${MANDIR}/${cmdname}.1
done

rm -f footer.h2m
