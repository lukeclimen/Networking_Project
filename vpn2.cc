/* Network topology
 *                       
 *
 *      n0-------                           ------n3
 *               \                         /
 *                \                       /
 *      n1--------r0---------r1----------r3-------n4
 *                /                       \
 *               /                         \
 *      n2-------                           ------n5
 *
 */

#include <iostream>
#include <fstream>
#include <string>
#include <cassert>
#include "ns3/csma-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"


using namespace ns3;

int main (int argc, char *argv[]) {

    //First network
    NodeContainer network1;

    //Second network
    NodeContainer network2;

    //Routers in between networks
    NodeContainer routers;

    //Initialize each of the 3 "networks" as having 3 nodes (see above diagram)
    network1.Create(3);
    network2.Create(3);
    routers.Create(3);

    //Treating network 1 and network2 as LANs, we can set up their 
    CsmaHelper lanCSMA;
    lanCSMA.SetChannelAttribute("DataRate", StringValue("100Mbps"));
    lanCSMA.SetChannelAttribute("Delay", TimeValue(NanoSeconds(6560)));

    //Adding the routers on each end into their respective LANs
    network1.Add(routers.Get(0));
    network2.Add(routers.Get(2));

    NetDeviceContainer lan1;
    lan1 = lanCSMA.Install(network1);
}