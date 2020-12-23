# blockchain-simulator

Some simulations of blockchain consensus based on [NS3](https://www.nsnam.org/). It aims to simulate the relationship between **blockchain performance** and consensus protocol, network scale, block size, bandwidth, delay and other factors.


## Quick start

#### 1.Download ns3

The [3.29](https://www.nsnam.org/releases/ns-3-29/download/) or [3.25](https://www.nsnam.org/releases/ns-3-25/download/) version of NS3 is recommended.

#### 2.Plcae code

Next, you need to place all the files from **blockchain-simulator** to the respective folders under `ns-allinone-3.xx/ns-3.xx`.

#### 3.Update the wscript file

The path of the wscript file is `ns-allinone-3.xx/ns-3.xx/src/applications/wscript`

+ Add the following lines in module.source:

```
'model/raft-node.cc',
'helper/network-helper.cc',
```

+ Add the following lines in headers.source:

```
'model/raft-node.h', 
'helper/network-helper.h', 
```

#### 4.Build and run the simulator

Under path `ns-allinone-3.xx/ns-3.xx`, execute the following commands in sequence:

```sh
CXXFLAGS="-std=c++11 -Wno-unused-parameter" ./waf configure
./waf
./waf --run scratch/blockchain-simulator
```

You can set the network scale and other factors in `blockchain-simulator.cc`.

More detailed usage use of NS3 can be referred to
<https://github.com/arthurgervais/Bitcoin-Simulator>


## Switch consensus

There are three consensus protocols to choose from, including raft, paxos and pbft, The default is raft.

If you need to switch the consensus protocol, such as from raft to pbft, You need to modify some files.

#### 1.Update the blockchain-simulator.cc

Modify the class name "RaftNode" to "PbftNode":

```c++
LogComponentEnable ("PbftNode", LOG_LEVEL_INFO);
```

#### 2.Update the network-helper.cc

Modify all "RaftNode" to "PbftNode", and switch the reference of the header file.


#### 3.Update the wscript

Modify the "raft-node" to "pbft-node" in `module.source` and `headers.source`. The two cannot coexist, otherwise an error will be reported.

After modification, you need to recompile and run.




