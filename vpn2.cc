/* Network topology
 *                       
 *
 *      n0-------                           ------n3
 *               \                         /
 *                \                       /
 *      n1--------r0---------r1----------r2-------n4
 *                /                       \
 *               /                         \
 *      n2-------                           ------n5
 *
 * 
 *  {n0, n1, n2, r0} are the set of nodes comprising LAN #1
 *  {n3, n4, n5, r2} are the set of nodes comrpising LAN #2
 *  LAN #1 is connected to LAN #2 through the routers {r0, r1, r2}
 *  where r1 is the link between the two subnets in this network
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

    /*
     * SECTION 1:
     * Creating the two networks and the routers connecting them
     * structurally to one another, governing which connect to which
     */

    NodeContainer network1, network2, routers;

    //Initialize each of the 3 "networks" as having 3 nodes (see above diagram)
    network1.Create(3);
    network2.Create(3);
    routers.Create(3);

    //Using a Carrier-sense multiple access (CSMA) protocol for the subnets 1 & 2
    CsmaHelper lanCSMA;

    //Treating network 1 and network2 as LANs, we can set up their networks to
    //have the same data transmission values
    lanCSMA.SetChannelAttribute("DataRate", StringValue("100Mbps"));
    lanCSMA.SetChannelAttribute("Delay", StringValue("2ms"));

    //Adding the routers on each end into their respective LANs
    network1.Add(routers.Get(0));
    network2.Add(routers.Get(2));

    //Installing the LANs with their data transmission stats
    NetDeviceContainer lan1, lan2;

    //From the Network Topology above:
    //lan1 is comprised of the set {n0, n1, n2, r0}
    lan1 = lanCSMA.Install(network1);
    //lan2 is comprised of the set {n3, n4, n5, r2}
    lan2 = lanCSMA.Install(network2);

    //Using Point-to-Point for the routers that are linking the two subnets
    PointToPointHelper pointToPoint;

    pointToPoint.SetDeviceAttribute("DataRate", StringValue("30Mbps"));
    pointToPoint.SetDeviceAttribute("Delay", StringValue("10ms"));
    
    //Installing the LANs with their data transmission stats
    NetDeviceContainer link1, link2;

    //link1 is comprised of the router from LAN1 and the "linking router", {r0, r1}
    link1 = pointToPoint.Install(routers.Get(0), routers.Get(1));
    //link2 is comprised of the router from LAN2 and the "linking router", {r1, r2}
    link2 = pointToPoint.Install(routers.Get(1), routers.Get(2));

    /*
     * SECTION 2:
     * Setting up the IP addresses of the different nodes
     * and aggregating IP/TCP/UDP functionality
     * 
     */

    InternetStackHelper iStackHelp;

    iStackHelp.Install(network1);
    iStackHelp.Install(network2);
    iStackHelp.Install(routers.Get(1));

    Ipv4AddressHelper ipv4;
    Ipv4InterfaceContainer lan1Subnet, lan2Subnet, link1Subnet, link2Subnet;
    
    //Setting up the IP addresses for the two LAN subnets
    ipv4.SetBase("10.1.1.0", "255.255.255.0");
    lan1Subnet = ipv4.Assign(lan1);

    ipv4.SetBase("10.1.2.0", "255.255.255.0");
    lan2Subnet = ipv4.Assign(lan2);

    //Setting up the IP addresses for the two router link subnets
    //Note these subnets have less specific IP address prefixes
    ipv4.SetBase("10.1.100.0", "255.255.255.0");
    link1Subnet = ipv4.Assign(link1);

    ipv4.SetBase("10.1.200.0", "255.255.255.0");
    link2Subnet = ipv4.Assign(link2);

    /*
     * Because Ipv4AddressHelper simply increments the address numbers, 
     * our nodes should have the following addresses:
     * 
     * n0: 10.1.1.1
     * n1: 10.1.1.2
     * n2: 10.1.1.3
     * 
     * n3: 10.1.2.1
     * n4: 10.1.2.2
     * n5: 10.1.2.3
     *  
     * r0: 10.1.1.4,     10.1.100.1
     * r1: 10.1.100.2,   10.1.200.1
     * r2: 10.1.2.4,     10.1.200.2
     */



}