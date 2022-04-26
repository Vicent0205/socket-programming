from mininet.topo import Topo
from mininet.net import Mininet
from mininet.node import CPULimitedHost
from mininet.link import TCLink
from mininet.util import dumpNodeConnections
from mininet.log import setLogLevel, info
from mininet.cli import CLI

class hw_topo(Topo):
    def build(self):
	switch1=self.addSwitch('s1')
	switch2=self.addSwitch('s2')
	switch3=self.addSwitch('s3')

	self.addLink(switch1,switch3)
	self.addLink(switch2,switch1)
	
	host1=self.addHost('h1',ip='10.0.0.1')
	host3=self.addHost('h3',ip='10.0.0.3')
	host2=self.addHost('h2',ip='10.0.0.2')
	host4=self.addHost('h4',ip='10.0.0.4')

	self.addLink(host2,switch1)
	self.addLink(host4,switch2)
	self.addLink(host1,switch3)
	self.addLink(host3,switch3)
def hw_test():
    topo=hw_topo()
    net=Mininet( topo=topo,host=CPULimitedHost,link=TCLink)
    net.start()
    CLI(net)
    net.stop()

if __name__=='__main__':
    setLogLevel('info')
    hw_test()



	
	
