# wizblcoincore.conf explanation

* The configuration file is a list of setting=value pairs, one per line, with optional comments starting with the '#' character.
* The configuration file is not automatically created.
* By default, wizblcoind(wizblcoin-cli) will look for a file named 'wizblcoincore.conf' in the wizblcoincore data directory, but both the data directory and the configuration file path may be changed using the -datadir and -conf command-line arguments.

| <center>Operating System</center>    | <center>Default wizblcoin datadir</center> | <center>Typical path to configuration file</center> |
| ------------------------------------ | ------------------------- | ---------------------------------- |
| Windows | %APPDATA%\wizblcoincore| C:\Users\username\AppData\Roaming\wizblcoincore\wizblcoincore.conf  |
| Linux   | $HOME/.wizblcoincore/ | /home/username/.wizblcoincore/wizblcoincore.conf |

* Note: if running wizblcoind in testnet mode, the sub-folder "testnet" will be appended to the data directory automatically.

## Sample wizblcoincore.conf

```text
##
## wizblcoincore.conf configuration file. Lines beginning with # are comments.
##
 
# Network-related settings:

# Run on the test network instead of the real wizbl network.
# testnet=0   // To use testnet : testnet=1

# Run a regression test network
#regtest=0

# Bind to given address and always listen on it. Use [host]:port notation for IPv6
#bind=<addr>

# Bind to given address and whitelist peers connecting to it. Use [host]:port notation for IPv6
#whitebind=<addr>

# Use as many addnode= settings as you like to connect to specific peers
#addnode=69.164.218.197
#addnode=10.0.0.2:8333

# Alternatively use as many connect= settings as you like to connect ONLY to specific peers
#connect=69.164.218.197
#connect=10.0.0.1:8333

# Listening mode, enabled by default except when 'connect' is being used
#listen=1

# Maximum number of inbound+outbound connections.
#maxconnections=


#
# JSON-RPC options (for controlling a running wizblcoind process)
#

# server=1 tells wizblcoin-Qt and wizblcoind to accept JSON-RPC commands
#server=0

# Bind to given address to listen for JSON-RPC connections. Use [host]:port notation for IPv6.
# This option can be specified multiple times (default: bind to all interfaces)
#rpcbind=<addr>

# If no rpcpassword is set, rpc cookie auth is sought. The default `-rpccookiefile` name
# is .cookie and found in the `-datadir` being used for wizblcoind. This option is typically used
# when the server and client are run as the same user.
#
# If not, you must set rpcuser and rpcpassword to secure the JSON-RPC api. 
# you must set this pair for the server and client:
#rpcuser=wizbl_test
#rpcpassword=YourSuperGreatPasswordNumber_DO_NOT_USE_THIS_OR_YOU_WILL_GET_ROBBED_385593
#
# On client-side, you add the normal user/password pair to send commands:
#rpcuser=wizbl_test
#rpcpassword=DONT_USE_THIS_YOU_WILL_GET_ROBBED_8ak1gI25KFTvjovL3gAM967mies3E=
#

# How many seconds wizblcoin will wait for a complete RPC HTTP request.
# after the HTTP connection is established. 
#rpcclienttimeout=30

# By default, only RPC connections from localhost are allowed.
# Specify as many rpcallowip= settings as you like to allow connections from other hosts,
# either as a single IPv4/IPv6 or with a subnet specification.

# NOTE: opening up the RPC port to hosts outside your local trusted network is NOT RECOMMENDED,
# because the rpcpassword is transmitted over the network unencrypted.

# server=1 tells wizblcoin-Qt to accept JSON-RPC commands.
# it is also read by wizblcoind to determine if RPC should be enabled 
#rpcallowip=10.1.1.34/255.255.255.0
#rpcallowip=1.2.3.4/24
#rpcallowip=2001:db8:85a3:0:0:8a2e:370:7334/96

# Listen for RPC connections on this TCP port:
#rpcport=8724

# You can use wizblcoin-cli to send commands to wizblcoind
# running on another host using this option:
#rpcconnect=127.0.0.1

# Miscellaneous options

# Pre-generate this many public/private key pairs, so wallet backups will be valid for
# both prior transactions and several dozen future transactions.
#keypool=100

# Enable pruning to reduce storage requirements by deleting old blocks. 
# This mode is incompatible with -txindex and -rescan.
# 0 = default (no pruning).
# 1 = allows manual pruning via RPC.
# >=550 = target to stay under in MiB. 
#prune=550

```
