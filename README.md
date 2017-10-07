# Token-based Distributed Monitor

Token-based distributed monitor written in C++. Provides almost fully transparent API for writting applications requiring distirbuted monitor objects. 

## Compilation
Uses qmake building system. Doesn't require any other dependencies.

`
qmake tdmonitor.pro
`

`
make qmake_all
`

`
make
`

## Running example app
In addition to library I have included an example application solving producer consumer problem.


`
prodconsumer [listen_addr] [node_id_from0] [node0_addr] [node1_addr] ...
`

`
prodconsumer 0.0.0.0:31410 0 127.0.0.1:31410 127.0.0.1:31411 127.0.0.1:31412
`

