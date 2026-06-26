#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("DataCenterDigitalTwin");

int main (int argc, char *argv[])
{
    // Enable command line arguments if you want to pass parameters later
    CommandLine cmd (__FILE__);
    cmd.Parse (argc, argv);

    Time::SetResolution (Time::NS);
    
    // Enable logging to see application behavior in the terminal
    LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);

    // ----------------------------------------------------------------
    // 1. CREATE NODES
    // ----------------------------------------------------------------
    NodeContainer spineSwitches;
    spineSwitches.Create (2); // 2 Core Spine Switches

    NodeContainer leafSwitches;
    leafSwitches.Create (2);  // 2 Access Leaf Switches

    NodeContainer servers;
    servers.Create (4);       // 4 Data Center Servers (Production Environment)

    NodeContainer attacker;
    attacker.Create (1);      // Rogue Node / Insider Threat Entry point

    // ----------------------------------------------------------------
    // 2. CONFIGURE CHANNELS (High-Speed Data Center Links)
    // ----------------------------------------------------------------
    // Fabric Interconnect: Spine to Leaf (e.g., 40 Gbps fabric)
    PointToPointHelper fabricLink;
    fabricLink.SetDeviceAttribute ("DataRate", StringValue ("40Gbps"));
    fabricLink.SetChannelAttribute ("Delay", StringValue ("2us"));

    // Access Link: Leaf to Servers/Attacker (e.g., 10 Gbps access)
    PointToPointHelper accessLink;
    accessLink.SetDeviceAttribute ("DataRate", StringValue ("10Gbps"));
    accessLink.SetChannelAttribute ("Delay", StringValue ("5us"));

    // ----------------------------------------------------------------
    // 3. BUILD THE TOPOLOGY (Full-Mesh Spine-Leaf)
    // ----------------------------------------------------------------
    // Connect Leaf 0 to Spines
    NetDeviceContainer dLeaf0Spine0 = fabricLink.Install (leafSwitches.Get (0), spineSwitches.Get (0));
    NetDeviceContainer dLeaf0Spine1 = fabricLink.Install (leafSwitches.Get (0), spineSwitches.Get (1));
    
    // Connect Leaf 1 to Spines
    NetDeviceContainer dLeaf1Spine0 = fabricLink.Install (leafSwitches.Get (1), spineSwitches.Get (0));
    NetDeviceContainer dLeaf1Spine1 = fabricLink.Install (leafSwitches.Get (1), spineSwitches.Get (1));

    // Connect Production Servers to Leaf Switches
    NetDeviceContainer dServ0Leaf0 = accessLink.Install (servers.Get (0), leafSwitches.Get (0));
    NetDeviceContainer dServ1Leaf0 = accessLink.Install (servers.Get (1), leafSwitches.Get (0));
    NetDeviceContainer dServ2Leaf1 = accessLink.Install (servers.Get (2), leafSwitches.Get (1));
    NetDeviceContainer dServ3Leaf1 = accessLink.Install (servers.Get (3), leafSwitches.Get (1));

    // Connect the Attacker node to Leaf 0 (representing a breached segment)
    NetDeviceContainer dAttackerLeaf0 = accessLink.Install (attacker.Get (0), leafSwitches.Get (0));

    // ----------------------------------------------------------------
    // 4. INSTALL INTERNET STACK & ROUTING
    // ----------------------------------------------------------------
    InternetStackHelper stack;
    stack.Install (spineSwitches);
    stack.Install (leafSwitches);
    stack.Install (servers);
    stack.Install (attacker);

    // ----------------------------------------------------------------
    // 5. ASSIGN IP ADDRESSES
    // ----------------------------------------------------------------
    Ipv4AddressHelper address;
    
    // Fabric IPs (Point-to-Point subnets)
    address.SetBase ("10.1.1.0", "255.255.255.252");
    address.Assign (dLeaf0Spine0);
    address.SetBase ("10.1.2.0", "255.255.255.252");
    address.Assign (dLeaf0Spine1);
    address.SetBase ("10.1.3.0", "255.255.255.252");
    address.Assign (dLeaf1Spine0);
    address.SetBase ("10.1.4.0", "255.255.255.252");
    address.Assign (dLeaf1Spine1);

    // Production Server IPs (Server 0 gets index 0, Leaf gets index 1)
    address.SetBase ("192.168.1.0", "255.255.255.0");
    Ipv4InterfaceContainer iServ0 = address.Assign (dServ0Leaf0);
    address.SetBase ("192.168.2.0", "255.255.255.0");
    Ipv4InterfaceContainer iServ1 = address.Assign (dServ1Leaf0);
    address.SetBase ("192.168.3.0", "255.255.255.0");
    Ipv4InterfaceContainer iServ2 = address.Assign (dServ2Leaf1);
    address.SetBase ("192.168.4.0", "255.255.255.0");
    Ipv4InterfaceContainer iServ3 = address.Assign (dServ3Leaf1);

    // Attacker IP
    address.SetBase ("172.16.1.0", "255.255.255.0");
    Ipv4InterfaceContainer iAttacker = address.Assign (dAttackerLeaf0);

    // Populate global routing tables so everyone can reach everyone else
    Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

    // ----------------------------------------------------------------
    // 6. SETUP NORMAL DATA CENTER TRAFFIC (Baseline)
    // ----------------------------------------------------------------
    uint16_t targetPort = 9; // Discard/Echo port
    
    // Setup Echo Server on Server 3 (Target node)
    UdpEchoServerHelper echoServer (targetPort);
    ApplicationContainer serverApps = echoServer.Install (servers.Get (3));
    serverApps.Start (Seconds (1.0));
    serverApps.Stop (Seconds (15.0));

    // Setup Legitimate Client on Server 0 sending normal requests
    UdpEchoClientHelper echoClient (iServ3.GetAddress (0), targetPort);
    echoClient.SetAttribute ("MaxPackets", UintegerValue (10));
    echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
    echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

    ApplicationContainer clientApps = echoClient.Install (servers.Get (0));
    clientApps.Start (Seconds (2.0));
    clientApps.Stop (Seconds (15.0));

    // ----------------------------------------------------------------
    // 7. SETUP THE ATTACK TRAFFIC (The Threat Presentation Element)
    // ----------------------------------------------------------------
    // Using OnOffHelper to simulate a high-volume UDP Flood (DDoS/DoS) 
    // Target is Server 3's IP address.
    OnOffHelper attackFlood ("ns3::UdpSocketFactory", Address (InetSocketAddress (iServ3.GetAddress (0), targetPort)));
    attackFlood.SetAttribute ("DataRate", StringValue ("8Gbps")); // Floods almost the entire access link capacity!
    attackFlood.SetAttribute ("PacketSize", UintegerValue (1400));
    
    ApplicationContainer attackApps = attackFlood.Install (attacker.Get (0));
    
    // The attack happens right in the middle of standard operations to observe impact
    attackApps.Start (Seconds (5.0));  
    attackApps.Stop (Seconds (10.0));   

    // ----------------------------------------------------------------
    // 8. EXECUTE SIMULATION
    // ----------------------------------------------------------------
    NS_LOG_UNCOND ("Starting digital twin data center simulation...");
    Simulator::Stop (Seconds (15.0));
    Simulator::Run ();
    Simulator::Destroy ();
    NS_LOG_UNCOND ("Simulation completed.");

    return 0;
}
