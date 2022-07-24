/*
 * This program uses the examples virtual-net-device.cc, 
 * udp-echo.cc and simple-global-routing.cc as a basis. A diagram 
 * can be found below describing the physical network that this 
 * simulation aims to replicate. The simulation will provide 
 * insight to the benefits of the IP security protocol to create 
 * a virtual private network (VPN). In WireShark, it will be 
 * shown that packets sent and received by nodes outside of the 
 * VPN are visible to packet sniffers, but traffic at nodes 
 * within the VPN are not visible.
 */


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
    lanCSMA.SetChannelAttribute("Delay", TimeValue (MilliSeconds (2)));

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
    pointToPoint.SetChannelAttribute("Delay", TimeValue(MilliSeconds(2)));
    
    //Installing the LANs with their data transmission stats
    NetDeviceContainer link1, link2;

    //link1 is comprised of the router from LAN1 and the "linking router", {r0, r1}
    link1 = pointToPoint.Install(routers.Get(0), routers.Get(1));
    //link2 is comprised of the router from LAN2 and the "linking router", {r1, r2}
    link2 = pointToPoint.Install(routers.Get(1), routers.Get(2));

    /*
     * SECTION 2:
     * Setting up the IP addresses of the different nodes and aggregating IP/TCP/UDP 
     * functionality. Also add sockets for sending and receiving UDP packets
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

    //Create routing tables for all of the nodes in the network
    Ipv4GlobalRoutingHelper :: PopulateRoutingTables();

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

    //We will set up n0 from LAN #1 to be a server for UDP datagrams
    Address serverAddress = Address(lan1Subnet.GetAddress(0));
    uint16_t serverListenerPort = 9;  // Echo port number from RFC 863

    UdpEchoServerHelper server(serverListenerPort);
    ApplicationContainer apps = server.Install(network1.Get(0));
    
    apps.Start(Seconds(1.0));
    apps.Stop(Seconds(10.0));

    //We will set up n3 from LAN #2 to be a client sending UDP datagrams
    uint32_t packetSize = 1024;
    uint32_t maxPacketCount = 1;
    Time interPacketInterval = Seconds(1.);

    UdpEchoClientHelper client(serverAddress, serverListenerPort);
    client.SetAttribute ("MaxPackets", UintegerValue (maxPacketCount));
    client.SetAttribute ("Interval", TimeValue (interPacketInterval));
    client.SetAttribute ("PacketSize", UintegerValue (packetSize));
    apps = client.Install(network2.Get(0));
    apps.Start(Seconds(2.0));
    apps.Stop(Seconds(10.0));

    /*
     * SECTION 3:
     * Creating a mock VPN using IPsec. We are treating LAN #1 and LAN #2 as two entities
     * remote from each other that wish to be connected via VPN. In this project, the router
     * r1 that connects the two LAN networks can represent the internet, and there can be n
     * point-to-point connections that are abstracted into r1.
     * 
     * UDP packets will be sent and received from two of the 6 nodes, one from each network
     * and secured as they travel through their LAN's router (r0 or r2), and then decrypted 
     * once the other LAN's router receives it. The router representing the internet (r1) 
     * will be unable to see the contents of the packets using the IPsec ESP protocol.
     * 
     * In order for this to work, both LAN routers must maintain state information about their
     * two security associations (SA) with the internet (one for each direction).
     */



}