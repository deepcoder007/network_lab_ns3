#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/ipv4-global-routing-helper.h"
#include<iostream>
#include<string>

// Dumbell network topology
using namespace ns3;
using namespace std;

NS_LOG_COMPONENT_DEFINE ("Assignment4");



/********************************************************************************
	TCP Application: This class will be used for our tcp application
********************************************************************************/
class TcpApp : public Application
{
public:
	TcpApp ();
	virtual ~TcpApp ();

	/*
		Used to setup the application parameters
		socket -> the socket on the host of the application
		address -> the address of the destination
		packetSize -> the size of the packets
		dtaRate -> the rate of data transfer
	*/
	void Setup (Ptr<Socket> socket, Address address, uint32_t packetSize,  DataRate dataRate);

private:
	virtual void StartApplication (void);    
	virtual void StopApplication (void);

	void ScheduleTx (void);
	void SendPacket (void);

	Ptr<Socket>     m_socket;
	Address         m_peer;
	uint32_t        m_packetSize;
//	uint32_t        m_nPackets;
	DataRate        m_dataRate;
	EventId         m_sendEvent;
	bool            m_running;
	uint32_t        m_packetsSent;
};

/*********************************************
	Initializes teh TCP variables
*********************************************/
TcpApp::TcpApp ()
	: m_socket (0),
	m_peer (),
   	m_packetSize (0),
 // 	m_nPackets (0),
    	m_dataRate (0),
    	m_sendEvent (),
    	m_running (false),
    	m_packetsSent (0)
{
}

TcpApp::~TcpApp ()
{
	m_socket = 0;
}

/*****************************************************************
	Initializes the parameters of the application
******************************************************************/
void TcpApp::Setup (Ptr<Socket> socket, Address address, uint32_t packetSize,  DataRate dataRate)
{
	m_socket = socket;
	m_peer = address;
	m_packetSize = packetSize;
//	m_nPackets = nPackets;
	m_dataRate = dataRate;
}

/**************************************************************
	To start the application
***************************************************************/
void TcpApp::StartApplication (void)
{
	m_running = true;
	m_packetsSent = 0;
	m_socket->Bind ();
	m_socket->Connect (m_peer);
	SendPacket ();
}

/************************************************************
	The application destructor
**************************************************************/
void TcpApp::StopApplication (void)
{
	m_running = false;

	if (m_sendEvent.IsRunning ())
	{
	      Simulator::Cancel (m_sendEvent);
	}

	if (m_socket)
	{
	      m_socket->Close ();
	}
}

/**********************************************************
	To send packet -> This is a dual recursion procedure , packets will be sent continusourly until the application is stopped
*************************************************************/
void TcpApp::SendPacket (void)
{
	Ptr<Packet> packet = Create<Packet> (m_packetSize);
	m_socket->Send (packet);

//	cout<<m_stopTime<<endl;
	if (m_running)
	{
//		cout<<Simulator::Now()<<endl;
		Time tNext (Seconds (m_packetSize * 8 / static_cast<double> (m_dataRate.GetBitRate ())));
 		m_sendEvent = Simulator::Schedule (tNext, &TcpApp::SendPacket, this);
	}


//	size_t total_pack = m_stopTime.GetSeconds()*static_cast<double>(m_dataRate.GetBitRate())/(8*m_packetSize) ;   // total no. of packets
//	double time_slice = (8*m_packetSize)/(static_cast<double>(m_dataRate.GetBitRate()))
/*
	for( size_t i = 0 ; i<total_pack; i++)
	{
		Simulator::Schedule(i*time_slice, 
	}
*/

}

void TcpApp::ScheduleTx (void)
{
	if (m_running)
	{
		cout<<Simulator::Now()<<endl;
		Time tNext (Seconds (m_packetSize * 8 / static_cast<double> (m_dataRate.GetBitRate ())));
 		m_sendEvent = Simulator::Schedule (tNext, &TcpApp::SendPacket, this);
	}
}

int main (int argc, char *argv[])
{
	string tcp_variant = "TcpNewReno";
	string bandwidth = "20Mbps";
	string delay = "5ms";
//	double error_rate = 0.000001;
	int queuesize = 10; //packets
	int simulation_time = 10; //seconds


	/*  To set various parameters */
  	Config::SetDefault ("ns3::Ipv4GlobalRouting::RespondToInterfaceEvents", BooleanValue (true));   // For global routing
	Config::SetDefault ("ns3::OnOffApplication::PacketSize", UintegerValue(1536) );              // Packet size of UDP CBR application
	Config::SetDefault ("ns3::OnOffApplication::DataRate", StringValue("20Mbps") );              // Bandwidth of UDP CBR application
	Config::SetDefault("ns3::TcpL4Protocol::SocketType", TypeIdValue (TcpNewReno::GetTypeId()));  // TCP socket type set to New Reno
	Config::SetDefault ("ns3::DropTailQueue::MaxPackets", UintegerValue(uint32_t(queuesize)));    // Queue size of router set 

	NS_LOG_INFO("Create Nodes");
	NodeContainer c;
	c.Create(8);   
	/*
		Following is the mapping of node index
		0  -> R1 ( Router 1) 
		1  -> R1 ( Router 2 )
		2  -> H1
		3  -> H2
		4  -> H3
		5  -> H4
		6  -> H5
		7  -> H6
	*/

	NodeContainer r1h1 = NodeContainer( c.Get(0), c.Get(2) );
	NodeContainer r1h2 = NodeContainer( c.Get(0), c.Get(3) );
	NodeContainer r1h3 = NodeContainer( c.Get(0), c.Get(4) );

	NodeContainer r2h4 = NodeContainer( c.Get(1), c.Get(5) );
	NodeContainer r2h5 = NodeContainer( c.Get(1), c.Get(6) );
	NodeContainer r2h6 = NodeContainer( c.Get(1), c.Get(7) );

	NodeContainer r1r2 = NodeContainer( c.Get(0), c.Get(1) );

	/*
		To setup internet stack on all the nodes 
	*/
	InternetStackHelper internet;
	internet.Install(c);	

	/* 
		Create point to point channels 
	*/
	NS_LOG_INFO("Create Channels. " );
	PointToPointHelper p2p;
//	p2p.SetQueue("ns3::DropTailQueue", UintegerValue(uint32_t(1000)));
	p2p.SetDeviceAttribute("DataRate",StringValue("100Mbps"));
	p2p.SetChannelAttribute("Delay", StringValue("10ms"));

	NetDeviceContainer d_r1h1 = p2p.Install(r1h1);
	NetDeviceContainer d_r1h2 = p2p.Install(r1h2);
	NetDeviceContainer d_r1h3 = p2p.Install(r1h3);
	 
	NetDeviceContainer d_r2h4 = p2p.Install(r2h4);
	NetDeviceContainer d_r2h5 = p2p.Install(r2h5);
	NetDeviceContainer d_r2h6 = p2p.Install(r2h6);

//	p2p.SetQueue("ns3::DropTailQueue", UintegerValue(uint32_t(1000)));
	p2p.SetDeviceAttribute("DataRate",StringValue("100Mbps"));
	p2p.SetDeviceAttribute("DataRate",StringValue("10Mbps"));
	p2p.SetChannelAttribute("Delay", StringValue("100ms"));
	NetDeviceContainer d_r1r2 = p2p.Install(r1r2);


	/*
		Assign IP addresses to various nodes 
	*/
	NS_LOG_INFO( "Assign IP Address" );
	Ipv4AddressHelper ipv4;
	ipv4.SetBase( "10.1.1.0", "255.255.255.0" );
	Ipv4InterfaceContainer addr_interface_r1h1 = ipv4.Assign(d_r1h1);
	Ipv4InterfaceContainer addr_interface_r1h2 = ipv4.Assign(d_r1h2);
	Ipv4InterfaceContainer addr_interface_r1h3 = ipv4.Assign(d_r1h3);

	ipv4.SetBase( "10.1.2.0", "255.255.255.0" );
	Ipv4InterfaceContainer addr_interface_r2h4 = ipv4.Assign(d_r2h4);
	Ipv4InterfaceContainer addr_interface_r2h5 = ipv4.Assign(d_r2h5);
	Ipv4InterfaceContainer addr_interface_r2h6 = ipv4.Assign(d_r2h6);

	ipv4.SetBase( "10.1.3.0", "255.255.255.0" );
	Ipv4InterfaceContainer addr_interface_r1r2 = ipv4.Assign(d_r1r2);


	// Fill the routing table of all the nodes
	Ipv4GlobalRoutingHelper::PopulateRoutingTables() ;

	// Start setting up the applications 
	/*
		There will be following applications
		1. 4 TCP New Reno flow applications
		2. 2 UDP CBR applications
	*/
	NS_LOG_INFO ( "Create Applications");
	uint16_t port=9;      // Port for Udp application
	uint16_t sinkPort = 800;    // Port for TCP applications

	// TCP connection 1 
	Address sinkAddress1 (InetSocketAddress (addr_interface_r2h4.GetAddress (1), sinkPort));  // Sink is H4
	PacketSinkHelper packetSinkHelper1 ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), sinkPort));
	ApplicationContainer sinkApps1 = packetSinkHelper1.Install (c.Get (5));
	sinkApps1.Start (Seconds (0.));
	sinkApps1.Stop (Seconds (simulation_time));

	Ptr<Socket> ns3TcpSocket1 = Socket::CreateSocket (c.Get (0), TcpSocketFactory::GetTypeId ());

	Ptr<TcpApp> app1 = CreateObject<TcpApp> ();
	app1->Setup (ns3TcpSocket1, sinkAddress1, 1526,  DataRate ("20Mbps"));
	c.Get (0)->AddApplication (app1);
	app1->SetStartTime (Seconds (1.));
	app1->SetStopTime (Seconds (simulation_time));


	// TCP connection 2
	Address sinkAddress2 (InetSocketAddress (addr_interface_r2h5.GetAddress (1), sinkPort));  // Sink is H4
	PacketSinkHelper packetSinkHelper2 ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), sinkPort));
	ApplicationContainer sinkApps2 = packetSinkHelper2.Install (c.Get (6));
	sinkApps2.Start (Seconds (0.));
	sinkApps2.Stop (Seconds (simulation_time));

	Ptr<Socket> ns3TcpSocket2 = Socket::CreateSocket (c.Get (2), TcpSocketFactory::GetTypeId ());

	Ptr<TcpApp> app2 = CreateObject<TcpApp> ();
	app2->Setup (ns3TcpSocket2, sinkAddress2, 1526,  DataRate ("20Mbps"));
	c.Get (2)->AddApplication (app2);
	app2->SetStartTime (Seconds (1.));
	app2->SetStopTime (Seconds (simulation_time));


	// TCP connection 3
	Address sinkAddress3 (InetSocketAddress (addr_interface_r1h2.GetAddress (1), sinkPort));  // Sink is H4
	PacketSinkHelper packetSinkHelper3 ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), sinkPort));
	ApplicationContainer sinkApps3 = packetSinkHelper3.Install (c.Get (3));
	sinkApps3.Start (Seconds (0.));
	sinkApps3.Stop (Seconds (simulation_time));

	Ptr<Socket> ns3TcpSocket3 = Socket::CreateSocket (c.Get (4), TcpSocketFactory::GetTypeId ());

	Ptr<TcpApp> app3 = CreateObject<TcpApp> ();
	app3->Setup (ns3TcpSocket3, sinkAddress3, 1526,  DataRate ("20Mbps"));
	c.Get (4)->AddApplication (app3);
	app3->SetStartTime (Seconds (1.));
	app3->SetStopTime (Seconds (simulation_time));
	
	// TCP connection 4
	Address sinkAddress4 (InetSocketAddress (addr_interface_r1r2.GetAddress (1), sinkPort));  // Sink is H4
	PacketSinkHelper packetSinkHelper4 ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), sinkPort));
	ApplicationContainer sinkApps4 = packetSinkHelper4.Install (c.Get (1));
	sinkApps4.Start (Seconds (0.));
	sinkApps4.Stop (Seconds (simulation_time));

	Ptr<Socket> ns3TcpSocket4 = Socket::CreateSocket (c.Get (0), TcpSocketFactory::GetTypeId ());

	Ptr<TcpApp> app4 = CreateObject<TcpApp> ();
	app4->Setup (ns3TcpSocket4, sinkAddress4, 1526,  DataRate ("20Mbps"));
	c.Get (0)->AddApplication (app4);
	app4->SetStartTime (Seconds (1.));
	app4->SetStopTime (Seconds (simulation_time));


	// UDP CBR connection 1
	OnOffHelper onoff1 ("ns3::UdpSocketFactory", Address (InetSocketAddress ( addr_interface_r1r2.GetAddress (1), port)));
	onoff1.SetConstantRate (DataRate ("20Mbps"));
	ApplicationContainer apps1 = onoff1.Install (c.Get (0));
	apps1.Start (Seconds (1.0));
	apps1.Stop (Seconds (10.0));

	// Create a packet sink to receive these packets
	PacketSinkHelper sink1 ("ns3::UdpSocketFactory", Address (InetSocketAddress (Ipv4Address::GetAny (), port)));
	apps1 = sink1.Install (c.Get (1));
	apps1.Start (Seconds (1.0));
	apps1.Stop (Seconds (10.0));


	// UDP CBR connection 2
	OnOffHelper onoff2 ("ns3::UdpSocketFactory", Address (InetSocketAddress ( addr_interface_r1r2.GetAddress (1), port)));
	onoff2.SetConstantRate (DataRate ("20Mbps"));
	ApplicationContainer apps2 = onoff2.Install (c.Get (2));
	apps2.Start (Seconds (1.0));
	apps2.Stop (Seconds (10.0));

	// Create a packet sink to receive these packets
	PacketSinkHelper sink2 ("ns3::UdpSocketFactory", Address (InetSocketAddress (Ipv4Address::GetAny (), port)));
	apps2 = sink2.Install (c.Get (1));
	apps2.Start (Seconds (1.0));
	apps2.Stop (Seconds (10.0));



	
/*
	OnOffHelper onoff ("ns3::UdpSocketFactory", Address (InetSocketAddress ( addr_interface_r2h6.GetAddress (1), port)));
	onoff.SetConstantRate (DataRate ("100Mbps"));
	ApplicationContainer apps = onoff.Install (c.Get (2));
	apps.Start (Seconds (1.0));
	apps.Stop (Seconds (10.0));

	// Create a packet sink to receive these packets
	PacketSinkHelper sink ("ns3::UdpSocketFactory", Address (InetSocketAddress (Ipv4Address::GetAny (), port)));
	apps = sink.Install (c.Get (7));
	apps.Start (Seconds (1.0));
	apps.Stop (Seconds (10.0));
*/

/*
 	onoff.SetAttribute ("Remote", AddressValue (InetSocketAddress (addr_interface_r2h6.GetAddress (0), port)));
	apps = onoff.Install (c.Get (2));
	apps.Start (Seconds (1.1));
	apps.Stop (Seconds (10.0));

	apps = sink.Install (c.Get (7));
	apps.Start (Seconds (1.1));
	apps.Stop (Seconds (10.0));
*/


	AsciiTraceHelper ascii;
	p2p.EnableAsciiAll (ascii.CreateFileStream ("ass4.tr"));
	p2p.EnablePcapAll ("ass4");

	// Flow Monitor
	bool enableFlowMonitor=true;
	FlowMonitorHelper flowmonHelper;
	if (enableFlowMonitor)
	{
		flowmonHelper.InstallAll ();
	}

	NS_LOG_INFO ("Run Simulation");
  	Simulator::Stop (Seconds (11));
	Simulator::Run ();
	Simulator::Destroy ();
	NS_LOG_INFO ("Done.");

	if (enableFlowMonitor)
   	{
     		flowmonHelper.SerializeToXmlFile ("ass4.flowmon", false, false);
   	}
	return 0;
}
