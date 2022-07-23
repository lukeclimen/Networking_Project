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

    //Creating the two networks and the routers connecting them
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

    pointToPoint.setDeviceAttribute("DataRate", StringValue("30Mbps"));
    pointToPoint.setDeviceAttribute("Delay", StringValue("10ms"));
    
    //Installing the LANs with their data transmission stats
    NetDeviceContainer link1, link2;

    //link1 is comprised of the router from LAN1 and the "linking router", {r0, r1}
    link1 = pointToPoint.Install(routers.Get(0), routers.Get(1));
    //link2 is comprised of the router from LAN2 and the "linking router", {r1, r2}
    link2 = pointToPoint.Install(routers.Get(1), routers.Get(2));

    

}