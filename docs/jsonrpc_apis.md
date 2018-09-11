Instructions
===============



#### getbestblockhash  
description: return the hash of the most recent block 

parameters: none

result: 

| name   | type | description |
| ------ | ---- | ----------- |
| blockhash | string | the hash of the most recent block | 

arguments sample:
```text
wizblcoin-cli getbestblockhash
```

result sample:
```text
4163a497a9c1d4f26f84bd82a27cb13b5902978ead0add2e2809e370ea2709ce
```

#### getblockhash  
description: return the hash of the blockchain height.

paramters: 

| name | type | description |
| ---- | ---- | ----------- |
| height | integer | the height of blockchain |

result:

| name   | type | description |
| ------ | ---- | ----------- |
| blockhash | string | the hash of the block | 

arguments sample:
```text
wizblcoin-cli getblockhash 241
```

result sample:
```text
4163a497a9c1d4f26f84bd82a27cb13b5902978ead0add2e2809e370ea2709ce
```

#### getblock  
description: return the block information of the blockchain hash.

parameters:

| name | type | description |
| ---- | ---- | ----------- |
| blockhash | string | the blockchain hash | 
| verbosity | int | the verbosity of result, can be 0, 1, 2 |

arguments sample:
```text
wizblcoin-cli getblock 4163a497a9c1d4f26f84bd82a27cb13b5902978ead0add2e2809e370ea2709ce 0
```

result:(verbosity=0)

raw hash

result sample:
``` text
03000020c56095d3a1999934fda06d5191a8b4b7171387453d98ad24ef53e2cdd2dc53d90f5f4f857236842cd4c3b7f814fd30efc8cf0f494c89d2ed197450b86a6a0792f100000000000000000000000000000000000000000000000000000000000000f203895bffff00210000000000000000000000000000000000000000000000000000000000000000000202000000010000000000000000000000000000000000000000000000000000000000000000ffffffff0502f1000101ffffffff02005c4919de0800001976a9141f20c44088e876e9144ae3dc56076bb42203d4c488ac0000000000000000266a24aa21a9edad18ab02ce1519382cec1813010171744736dcb9f17c259fb67a033d7262495b0000000002000000010c33808c62f45048491846a444bde4c33a639e20233a107a93eac83972f0cdfe010000006a473044022079dc6f0adc09dbc8e1cafd531bc2abaa9c5950f4dc9558013576f92ea2d80968022008333c4e63b49ad2eda174dfa376bec65f6be2dd9955d0a531454919d48c6d63412102776a525013fd8363ec2598deae9babcc797ddfd5fedfa79ba809f56078b65113feffffff0200c0d0d335a51a001976a91459a950b903dea6368e1bf58d675f2a7bccfa67d188aca0a07e0ca36486001976a9147857b1288e02eefcdf8d43185cab1cd8e0c0fb4488acf0000000
```

result:(verbosity=1)

| name | type | description |
| ---- | ---- | ----------- |
| hash | string | the blockchain hash |
| confirmations | integer | confirmations |
| strippedsize | integer | equals to size |
| size | integer | the size of a block in bytes |
| weight | integer | This block’s weight |
| height | integer | the height of block |
| version | integer | block header's version |
| versionhex | string | block header's version in hex format |
| merkleroot | string | the merkleroot hash of this block |
| tx | array[string] | transaction hashes of this block, in an array|
| time | integer | the Unix timestamp of this block |
| mediantime | integer | equals to time |
| GenerateID | integer | generator node id |
| VerificationLength | integer | node count that verify block |
| sig | string | block signature in hex format |
| chainwork | string | The estimated number of block header hashes miners had to check from the genesis block to this block, encoded as big-endian hex |
| previousblockhash | string | previous block hash |
| nextblockhash | string | next block hash |

arguments sample:
```text
wizblcoin-cli getblock 4163a497a9c1d4f26f84bd82a27cb13b5902978ead0add2e2809e370ea2709ce 1
```

result sample
```json
{
  "hash": "7a1048142449de846cf365899b7276844357d064badc33fefb2ebe5aadc19ab4",
  "confirmations": 1,
  "strippedsize": 779,
  "size": 779,
  "weight": 3116,
  "height": 900,
  "version": 536870912,
  "versionHex": "20000000",
  "merkleroot": "c16daffe0428846fa4263fbbc2bad597e90678f9e4f97ac4420b5e4af0ffc951",
  "tx": [
    "467d6f8feceb0fc6dd8aa630e79ebd46e9c5ed172559eab6f793a6f8a466c8b7", 
    "1864e8e83584be9b105e11bc20606319bc648077c1fc5e223a89d3af8530a8de"
  ],
  "time": 1536547908,
  "mediantime": 1536541972,
  "GenerateID": 0,
  "VerificationLength": 3,
  "sig": "6f4a8ac3e6c1dc3653c3d62d74a1c88460b9ae3ad0ff75e4d680f5d4f6bc0fce24d785da90deed8b9d5abaffc7e2db2ff77c845054e9f24e2f4ec37e6064470037961d98e38a91c669ef52d69e3e0bb885977117287c80944b5b0a6102753def87c689a824857692c38f41a9ce8e278944e9d218a9878e429ed2df5da309ae0b61f1d20ac30e7a255f9b9703ba1ab757ce40a8bf604a9d6a297e286509abe0182758f376d61c37fa2bf14fe7fdd9a7c8ab454919117dc0ff050bdea539cf7a06",
  "chainwork": "0000000000000000000000000000000000000000000000000000000000000385",
  "previousblockhash": "d97a6d960a896c2413d0df03e470bd41b68d0dc8729600949d63ff508730d39b"
}
```

result:(verbosity=2)

result format except 'tx' is the same as it is when verbosity=1

result format in 'tx' please see interface 'getrawtransaction'

arguments sample:
```text
wizblcoin-cli getblock 4163a497a9c1d4f26f84bd82a27cb13b5902978ead0add2e2809e370ea2709ce 2
```

result sample
```json
{
  "hash": "7a1048142449de846cf365899b7276844357d064badc33fefb2ebe5aadc19ab4",
  "confirmations": 1,
  "strippedsize": 779,
  "size": 779,
  "weight": 3116,
  "height": 900,
  "version": 536870912,
  "versionHex": "20000000",
  "merkleroot": "c16daffe0428846fa4263fbbc2bad597e90678f9e4f97ac4420b5e4af0ffc951",
  "tx": [
    {
      "txid": "467d6f8feceb0fc6dd8aa630e79ebd46e9c5ed172559eab6f793a6f8a466c8b7",
      "hash": "467d6f8feceb0fc6dd8aa630e79ebd46e9c5ed172559eab6f793a6f8a466c8b7",
      "version": 2,
      "size": 137,
      "vsize": 137,
      "locktime": 0,
      "vin": [
        {
          "coinbase": "0284030101",
          "sequence": 4294967295
        }
      ],
      "vout": [
        {
          "value": 39.00000000,
          "n": 0,
          "scriptPubKey": {
            "asm": "OP_DUP OP_HASH160 9491b0bb3a490544f879cd96250ced62134b6894 OP_EQUALVERIFY OP_CHECKSIG",
            "hex": "76a9149491b0bb3a490544f879cd96250ced62134b689488ac",
            "reqSigs": 1,
            "type": "pubkeyhash",
            "addresses": [
              "wZAzTo2tGk7L44w4vdVyfFCUf4uM1puwus"
            ]
          }
        }, 
        {
          "value": 0.00000000,
          "n": 1,
          "scriptPubKey": {
            "asm": "OP_RETURN aa21a9ed8040e057e0814fa4867731242ffd85e6609188667bddeeba67a0f52dde632182",
            "hex": "6a24aa21a9ed8040e057e0814fa4867731242ffd85e6609188667bddeeba67a0f52dde632182",
            "type": "nulldata"
          }
        }
      ],
      "hex": "02000000010000000000000000000000000000000000000000000000000000000000000000ffffffff050284030101ffffffff02004775e8000000001976a9149491b0bb3a490544f879cd96250ced62134b689488ac0000000000000000266a24aa21a9ed8040e057e0814fa4867731242ffd85e6609188667bddeeba67a0f52dde63218200000000"
    }, 
    {
      "txid": "1864e8e83584be9b105e11bc20606319bc648077c1fc5e223a89d3af8530a8de",
      "hash": "1864e8e83584be9b105e11bc20606319bc648077c1fc5e223a89d3af8530a8de",
      "version": 2,
      "size": 373,
      "vsize": 373,
      "locktime": 899,
      "vin": [
        {
          "txid": "89d74d42a7146aeac7ed59a1778186d4664f10b1dc7f64c32baa0105c8846e09",
          "vout": 0,
          "scriptSig": {
            "asm": "3045022100d17b6d581ac5cbe521880d8817697622f143b1319a86c4e64a038d13ddc8f3d1022065f948e81c1fbf072dad3158f12881915be8ad5740188475d0dbdac4dc944525[ALL|FORKID] 03e5cf76c0e0331a9b0cb261862a91ff3a726a486d28867e8f9b2190eb88de9cd7",
            "hex": "483045022100d17b6d581ac5cbe521880d8817697622f143b1319a86c4e64a038d13ddc8f3d1022065f948e81c1fbf072dad3158f12881915be8ad5740188475d0dbdac4dc944525412103e5cf76c0e0331a9b0cb261862a91ff3a726a486d28867e8f9b2190eb88de9cd7"
          },
          "sequence": 4294967294
        }, 
        {
          "txid": "eac386b822bf89aa16d60e862f6a8ec51cb4a449d1e7ca92cb2ae0fad3a8fd4c",
          "vout": 1,
          "scriptSig": {
            "asm": "30440220021e9276879e62db5d5adcb6af06fe1484f89196777bfd05bb7c75a35d5cd767022050f3a64543b1314a3730e8845e2f67b6df42db0e55fe30683888d053c3952eaf[ALL|FORKID] 021e8f3169f53b00d05ee2993b9ccf42f5b11333025a7fd95234b47fc30e74b9df",
            "hex": "4730440220021e9276879e62db5d5adcb6af06fe1484f89196777bfd05bb7c75a35d5cd767022050f3a64543b1314a3730e8845e2f67b6df42db0e55fe30683888d053c3952eaf4121021e8f3169f53b00d05ee2993b9ccf42f5b11333025a7fd95234b47fc30e74b9df"
          },
          "sequence": 4294967294
        }
      ],
      "vout": [
        {
          "value": 30000.00000000,
          "n": 0,
          "scriptPubKey": {
            "asm": "OP_DUP OP_HASH160 d13b6f2f4b765e28dc2c9d944abeeb65b89c5205 OP_EQUALVERIFY OP_CHECKSIG",
            "hex": "76a914d13b6f2f4b765e28dc2c9d944abeeb65b89c520588ac",
            "reqSigs": 1,
            "type": "pubkeyhash",
            "addresses": [
              "wehkMKb8hC6qUyPumx8ScAh7ua6nBjoGSc"
            ]
          }
        }, 
        {
          "value": 110.00000000,
          "n": 1,
          "scriptPubKey": {
            "asm": "OP_DUP OP_HASH160 b96d796d1e0e195019fdc9a26b48f58a9bd335d9 OP_EQUALVERIFY OP_CHECKSIG",
            "hex": "76a914b96d796d1e0e195019fdc9a26b48f58a9bd335d988ac",
            "reqSigs": 1,
            "type": "pubkeyhash",
            "addresses": [
              "wcXt5ZK9nr4vuTsmGH2UzxCmBzBW4RmwWa"
            ]
          }
        }
      ],
      "hex": "0200000002096e84c80501aa2bc3647fdcb1104f66d4868177a159edc7ea6a14a7424dd789000000006b483045022100d17b6d581ac5cbe521880d8817697622f143b1319a86c4e64a038d13ddc8f3d1022065f948e81c1fbf072dad3158f12881915be8ad5740188475d0dbdac4dc944525412103e5cf76c0e0331a9b0cb261862a91ff3a726a486d28867e8f9b2190eb88de9cd7feffffff4cfda8d3fae02acb92cae7d149a4b41cc58e6a2f860ed616aa89bf22b886c3ea010000006a4730440220021e9276879e62db5d5adcb6af06fe1484f89196777bfd05bb7c75a35d5cd767022050f3a64543b1314a3730e8845e2f67b6df42db0e55fe30683888d053c3952eaf4121021e8f3169f53b00d05ee2993b9ccf42f5b11333025a7fd95234b47fc30e74b9dffeffffff020030ef7dba0200001976a914d13b6f2f4b765e28dc2c9d944abeeb65b89c520588ac00aea68f020000001976a914b96d796d1e0e195019fdc9a26b48f58a9bd335d988ac83030000"
    }
  ],
  "time": 1536547908,
  "mediantime": 1536541972,
  "GenerateID": 0,
  "VerificationLength": 3,
  "sig": "6f4a8ac3e6c1dc3653c3d62d74a1c88460b9ae3ad0ff75e4d680f5d4f6bc0fce24d785da90deed8b9d5abaffc7e2db2ff77c845054e9f24e2f4ec37e6064470037961d98e38a91c669ef52d69e3e0bb885977117287c80944b5b0a6102753def87c689a824857692c38f41a9ce8e278944e9d218a9878e429ed2df5da309ae0b61f1d20ac30e7a255f9b9703ba1ab757ce40a8bf604a9d6a297e286509abe0182758f376d61c37fa2bf14fe7fdd9a7c8ab454919117dc0ff050bdea539cf7a06",
  "chainwork": "0000000000000000000000000000000000000000000000000000000000000385",
  "previousblockhash": "d97a6d960a896c2413d0df03e470bd41b68d0dc8729600949d63ff508730d39b"
}
```

#### getblockcount

description: get block count

parameters: none

arguments sample:
```text
wizblcoin-cli getblockcount
```
result sample:
```text
241
```

#### getrawtransaction

description: get transaction infomation of given transaction hash.

parameters:

| name | type | description |
| ---- | ---- | ----------- |
| txid | string | transaction hash |
| verbose | bool | verbose of result |

results:

| name | type | description |
| ---- | ---- | ----------- |
| txid | string | transaction id |
| hash | string | transaction id |
| version | integer | ??????????????? | 
| size | integer | transaction size |
| vsize | integer | The virtual transaction size, equals to size || version | integer | The transaction format version number |
| locktime | integer | The transaction’s locktime |
| vin | array | input utxo vector of this transaction |
| vout | array | output utxo vector of this transaction |
| sequence | integer | The transaction’s sequence number |
| n   | integer | index of utxo outputs |
| hex | string | raw transaction in hex format |
| blockhash | string | blockhash contains txid |
| time | long | tx time |
| blocktime | long | block time |

argument sample:
```text
wizblcoin-cli getrawtransaction 85fbb772ed71c7d4081ed16df46436d7acdc58c0832c765278c19c5684621369 1
```
result sample:(verbose=1)
```json
{
  "txid": "85fbb772ed71c7d4081ed16df46436d7acdc58c0832c765278c19c5684621369",
  "hash": "85fbb772ed71c7d4081ed16df46436d7acdc58c0832c765278c19c5684621369",
  "version": 2,
  "size": 137,
  "vsize": 137,
  "locktime": 0,
  "vin": [
    {
      "coinbase": "02f1000101",
      "sequence": 4294967295
    }
  ],
  "vout": [
    {
      "value": 97500.00000000,
      "n": 0,
      "scriptPubKey": {
        "asm": "OP_DUP OP_HASH160 1f20c44088e876e9144ae3dc56076bb42203d4c4 OP_EQUALVERIFY OP_CHECKSIG",
        "hex": "76a9141f20c44088e876e9144ae3dc56076bb42203d4c488ac",
        "reqSigs": 1,
        "type": "pubkeyhash",
        "addresses": [
          "MAjkS3KoEYTnr8wTBdufaGwGSorYb8fwnP"
        ]
      }
    }, 
    {
      "value": 0.00000000,
      "n": 1,
      "scriptPubKey": {
        "asm": "OP_RETURN aa21a9edad18ab02ce1519382cec1813010171744736dcb9f17c259fb67a033d7262495b",
        "hex": "6a24aa21a9edad18ab02ce1519382cec1813010171744736dcb9f17c259fb67a033d7262495b",
        "type": "nulldata"
      }
    }
  ],
  "hex": "02000000010000000000000000000000000000000000000000000000000000000000000000ffffffff0502f1000101ffffffff02005c4919de0800001976a9141f20c44088e876e9144ae3dc56076bb42203d4c488ac0000000000000000266a24aa21a9edad18ab02ce1519382cec1813010171744736dcb9f17c259fb67a033d7262495b00000000",
  "blockhash": "4163a497a9c1d4f26f84bd82a27cb13b5902978ead0add2e2809e370ea2709ce",
  "confirmations": 1,
  "time": 1535706098,
  "blocktime": 1535706098
}
```
argument sample:
```text
wizblcoin-cli getrawtransaction 85fbb772ed71c7d4081ed16df46436d7acdc58c0832c765278c19c5684621369 0
```
result sample:(verbose=false)

return raw data
```text
02000000010000000000000000000000000000000000000000000000000000000000000000ffffffff0502f1000101ffffffff02005c4919de0800001976a9141f20c44088e876e9144ae3dc56076bb42203d4c488ac0000000000000000266a24aa21a9edad18ab02ce1519382cec1813010171744736dcb9f17c259fb67a033d7262495b00000000
```

#### getrawmempool

description: return hashes of transactions in memory pool.

parameters: none

argument sample:
```text
wizblcoin-cli getrawmempool
```

result sample:

```json
[
  "2b333dd0d96eec93bd043f7bf262c6d63781ff758ce2134c75d8a547394779d9", 
  "7e2c26e75e50b2d6f953ed1416c9f3a94327a162f727e0fb9872e01567cde76b"
]
```

#### getaccountaddress  
description: return the address about created account
if account is existed then return already existing address

paramters: 

| name | type | description |
| ---- | ---- | ----------- |
| account | string | the account you want to create in this wallet  |

result:

| name   | type | description |
| ------ | ---- | ----------- |
| address | string | the address created by your account | 

arguments sample:
```text
wizblcoin-cli getaccountaddress wizbl-test.com
```

result sample:
```text
wE5NLPUFcVLCD7xTmzxHMYYsc47tLD2gcA
```

#### getnewaddress  
description: return the address for an existing account

paramters: 

| name | type | description |
| ---- | ---- | ----------- |
| account | string | the account you want to create a new address  |

result:

| name   | type | description |
| ------ | ---- | ----------- |
| address | string | the address created by account | 

arguments sample:
```text
wizblcoin-cli getnewaddress wizbl-test.com
```

result sample:
```text
wVicsJpoiRVd8GbzKqmCR5NDjdN5o4eHAD
```

#### getaddressesbyaccount  
description: return the addresses for an existing account

paramters: 

| name | type | description |
| ---- | ---- | ----------- |
| account | string | the account you want to check  |

result:

| name   | type | description |
| ------ | ---- | ----------- |
| address | array[string] | the addresses included in account | 

arguments sample:
```text
wizblcoin-cli getaddressesbyaccount wizbl-test.com
```

result sample:
```json
[
  "wE5NLPUFcVLCD7xTmzxHMYYsc47tLD2gcA", 
  "wEAuDiE2XTTtk9JXDCB2k3d4ECE1UKo6iP", 
  "wSKCithw8Vnrcqo8xJ1WedWnc9LDsgqvAY", 
  "wVicsJpoiRVd8GbzKqmCR5NDjdN5o4eHAD"
]
```

#### getaccount  
description: return the account associated with address

paramters: 

| name | type | description |
| ---- | ---- | ----------- |
| address | string | the address you want to know account  |

result:

| name   | type | description |
| ------ | ---- | ----------- |
| account | string | the account associated with address | 

arguments sample:
```text
wizblcoin-cli getaccount wE5NLPUFcVLCD7xTmzxHMYYsc47tLD2gcA
```

result sample:
```text
wizbl-test.com
```

#### listaccounts  
description: returns object that has account names as keys, account balances as values.

paramters: none

result:

| name   | type | description |
| ------ | ---- | ----------- |
| account | string | the account includes in the wallet |
| balance | double | the balance  associated with account |


arguments sample:
```text
wizblcoin-cli listaccounts
```

result sample:
```json
{
 "wizbl-test.wizbl.com": 0.00000000,
  "wizbl-test1.wizbl.com": 100.00000000
}
```

#### getbalance  
description: returns the balance in the account.

paramters: 

| name | type | description |
| ---- | ---- | ----------- |
| account | string | the account you want to know total balance in wallet  |

result:

| name   | type | description |
| ------ | ---- | ----------- |
| balance | double | the balance  associated with account in wallet |


arguments sample:
```text
wizblcoin-cli getbalance wizbl-test.com
```

result sample:
```text
0.00000000
```

#### sendfrom  
description: sent an amount from an account to a address.

paramters: 

| name | type | description |
| ---- | ---- | ----------- |
| account | string | the name of the account to send funds from  |
| address | string | the coin address to send funds to  |
| amount | double | The amount in send to address |

result:

| name   | type | description |
| ------ | ---- | ----------- |
| txid | string | the transaction id |


arguments sample:
```text
wizblcoin-cli sendfrom wizbl-test.com MSKCithw8Vnrcqo8xJ1WedWnc9LDsgqvAY 10
```

result sample:
```text
6735e9f16cab2aafb9b36f75d60963124cd1a63ad516bc75c3ec96e50e90c716
```

#### gettransaction  
description: get detailed information about in-wallet transaction

paramters: 

| name | type | description |
| ---- | ---- | ----------- |
| txid | string | the transaction id in-wallet transaction|

result:

| name   | type | description |
| ------ | ---- | ----------- |
| amount | double | The transaction amount |
| fee | double | The amount of the fee, This is negative and only available for the 'send' category of transactions. |
| confirmations | integer | the number of confimations |
| trusted | boolean | ??????????????? |
| txid | string | the transaction id |
| walletconflicts | array[string] | ???????????? |
| time | long | the transaction time in seconds since epoch |
| timereceived | long | the time received in seconds sincd epoch |
| bip125-replaceable | [yes/no/unknown] | Whether this transaction could be replaced due to BIP125 (replace-by-fee) |
| details | jsonarray | detail information about this transaction |
| hex | string | raw data for transaction in hex format |



arguments sample:
```text
wizblcoin-cli gettransaction 6735e9f16cab2aafb9b36f75d60963124cd1a63ad516bc75c3ec96e50e90c716
```

result sample:
```json
{
  "amount": 0.00000000,
  "fee": -0.01300000,
  "confirmations": 0,
  "trusted": true,
  "txid": "6735e9f16cab2aafb9b36f75d60963124cd1a63ad516bc75c3ec96e50e90c716",
  "walletconflicts": [
  ],
  "time": 1536552896,
  "timereceived": 1536552896,
  "bip125-replaceable": "no",
  "details": [
    {
      "account": "wizbl-test.com",
      "address": "MSKCithw8Vnrcqo8xJ1WedWnc9LDsgqvAY",
      "category": "send",
      "amount": -10.00000000,
      "label": "wizbl-test.com",
      "vout": 1,
      "fee": -0.01300000,
      "abandoned": false
    }, 
    {
      "account": "wizbl-test.com",
      "address": "MSKCithw8Vnrcqo8xJ1WedWnc9LDsgqvAY",
      "category": "receive",
      "amount": 10.00000000,
      "label": "wizbl-test.com",
      "vout": 1
    }
  ],
  "hex": "02000000026be7cd6715e07298fbe027f762a12743a9f3c91614ed53f9d6b2505ee7262c7e010000006b483045022100b9f35e696c5b73c0d921aa754f04cdcebbcdb1095e36efec7f9474f0bc6caf4a0220755ffed78e3e1a83e1973427101235116f3ad7780b7faaf84590fd5c780c7f8c41210342ee65368f0206d5689a3f41c333c30861f63ffe7a6583d836a8c884191cd36afeffffffccbd4518c54fce14c468ffdf7771af87810ebbab497059fe4e66e0e39e2a193d010000006a4730440220793f38bd145c8d816a5cbc86e12c5585af9c71daeab2a95a1ded63b160bce94d022074fa1699779cafb1b174f14b3241ada57965d6666374e276544fba0e0949570f4121036f48f178c303b120ad94d1df5362bed60c8b916e4e675ffc09f6b5a75acff775feffffff0200162001000000001976a9143310507a0bb654a8e134758c949cca97780a7a1088ac00ca9a3b000000001976a914c9fe538b4c90073362d79289ba134ee64d9ebf3588acf2000000"
}
```

#### listsinceblock  
description: get all transactions in blocks since block [blockhash], or all transactions if omitted.

paramters: 

| name | type | description |
| ---- | ---- | ----------- |
| blockhash | string | The block hash to list transactions since|
| target_confirmations | integer | return the nth block hash from the main chain. e.g. 1 would mean the best block hash. Note: this is not used as a filter, but only affects [lastblock] in the return value (defalut=1) |

result:

| name   | type | description |
| ------ | ---- | ----------- |
| transactions | array[JsonObject] | The transaction list |
| account | string | the account name associated with the transaction |
| address | string | the coin address of the transaction |
| category | [send/received] | The transaction category. 'send' has negative amounts, 'receive' has positive amounts. |
| amount | double | the amount |
| vout | integer | the vout value |
| fee | double | The amount of the fee.  This is negative and only available for the 'send' category of transactions. |
| confirmations | integer | The number of confirmations for the transaction. |
| blockhash | string | The block hash containing the transaction. Available for 'send' and 'receive' category of transactions. |
| blockindex | integer | The index of the transaction in the block that includes it. Available for 'send' and 'receive' category of transactions. |
| blocktime | long | The block time in seconds since epoch |
| txid | string | The transaction id. Available for 'send' and 'receive' category of transactions. |
| time | long | The transaction time in seconds since epoch |
| timereceived | long | The time received in seconds since epoch |
| bip125-replaceable | [yes/no/unknown] | Whether this transaction could be replaced due to BIP125 (replace-by-fee) |
| abandoned | boolean | 'true' if the transaction has been abandoned (inputs are respendable). Only available for the 'send' category of transactions. |
| removed | Deprecated | - |
| lastblock | string | The hash of the block (target_confirmations-1) from the best block on the main chain. This is typically used to feed back into listsinceblock the next time you call it. So you would generally use a target_confirmations of say 6, so you will be continually re-notified of transactions until they've reached 6 confirmations plus any new ones |

arguments sample:
```text
wizblcoin-cli listsinceblock 4163a497a9c1d4f26f84bd82a27cb13b5902978ead0add2e2809e370ea2709ce
```

result sample:
```json
{
  "transactions": [
    {
      "account": "wizbl-test.com",
      "address": "MSKCithw8Vnrcqo8xJ1WedWnc9LDsgqvAY",
      "category": "send",
      "amount": -10.00000000,
      "label": "wizbl-test.com",
      "vout": 1,
      "fee": -0.01300000,
      "confirmations": 0,
      "trusted": true,
      "txid": "6735e9f16cab2aafb9b36f75d60963124cd1a63ad516bc75c3ec96e50e90c716",
      "walletconflicts": [
      ],
      "time": 1536552896,
      "timereceived": 1536552896,
      "bip125-replaceable": "no",
      "abandoned": false
    }, 
    {
      "account": "wizbl-test.com",
      "address": "MSKCithw8Vnrcqo8xJ1WedWnc9LDsgqvAY",
      "category": "receive",
      "amount": 10.00000000,
      "label": "wizbl-test.com",
      "vout": 1,
      "confirmations": 0,
      "trusted": true,
      "txid": "6735e9f16cab2aafb9b36f75d60963124cd1a63ad516bc75c3ec96e50e90c716",
      "walletconflicts": [
      ],
      "time": 1536552896,
      "timereceived": 1536552896,
      "bip125-replaceable": "no"
    }
  ],
  "removed": [
  ],
  "lastblock": "2638df37d41a53e351cad1b7300f9ca74deb6a44e5db092e16effb17854cb31d"
}
```



