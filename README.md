# PCIe Latency Testing Driver

This driver is a complete rewrite and enhancement of pcie-lat by Andre Richter (https://github.com/andre-richter/pcie-lat)

The purpose of this rewritten driver is to test latency from individual CPU cores to a particular PCIe device. The driver will register IRQ handlers and character devices for the specified device. These devices can be interacted with from user-land in order to trigger tests and view results.

**THIS DRIVER IS A WORK IN PROGESS AND THE CURRENT COMMIT IS INCOMPLETE AND NON-FUNCTIONAL**

This is a complete ground-up rewrite of a substantially more enhanced version I wrote during previous employment.

This driver is meant to accompany a plugin being written for HPCBench, though also aims to be capable of being used as a stand-alone. 

The code is unfinished and currently only registers the IRQ handlers, creates the character devices and identifies appropriate memory BAR's. There is currently no way to trigger the interrupt / IRQ handler. On Intel this is simply a matter of calling int with the appropriate IRQ handler offset. On ARM it's a bit more complex. 

## Usage
### Loading the driver
The driver must be loaded before it can be used. A loader helper script is included with the driver under tests/ directory. Please note that this will unload whatever driver is currently using the targeted device and replace it with our driver.

    # ./load_driver.sh 02:00.0
    02:00.0
    Vendor ID: 8086
    Device ID: 10d3
    Driver:    e1000e
    
    Unregistering device from current driver...
    Loading pcilatdriver with claim on 8086:10d3
    Complete!

The driver loading will be logged and we can see details on identified BARs with IORESOURCE_MEM on them:

    [58156.327936] e1000e 0000:02:00.0 ens160: removed PHC
    [58156.390529] e1000e 0000:02:00.0 ens160: NIC Link is Down
    [58156.435487] pcilatdriver: loading out-of-tree module taints kernel.
    [58156.435537] pcilatdriver: module verification failed: signature and/or required key missing - tainting kernel
    [58156.435924] pcilatdriver: Driver loaded, initializing... CPU: 0
    [58156.435950] pcilatdriver: Driver initialized with device ID(s) specified: 8086:10d3
    [58156.435951] pcilatdriver: Adding 8086:10D3 sub=FFFFFFFF:FFFFFFFF cls=00000000/00000000
    [58156.436012] pcilatdriver: Probing device 02:00.0
    [58156.436021] pcilatdriver 0000:02:00.0: Scanning for BARs with IORESOURCE_MEM only...
    [58156.436025] pcilatdriver 0000:02:00.0: Found usable BAR: 0
    [58156.436029] pcilatdriver 0000:02:00.0: Found usable BAR: 3
    [58156.436086] pcilatdriver 0000:02:00.0: Requesting MSI[x] IRQ Vectors, min: 4 max: 4
    [58156.436163] pcilatdriver 0000:02:00.0: Received 4 vectors
    [58156.436164] pcilatdriver 0000:02:00.0: Registering vector 0: IRQ 46
    [58156.436196] pcilatdriver 0000:02:00.0: Registering vector 1: IRQ 47
    [58156.436215] pcilatdriver 0000:02:00.0: Registering vector 2: IRQ 48
    [58156.436233] pcilatdriver 0000:02:00.0: Registering vector 3: IRQ 70
    [58156.436256] pcilatdriver 0000:02:00.0: Status is 0
    [58156.436319] pcilatdriver 0000:02:00.0: Status 0
    [58156.436319] pcilatdriver 0000:02:00.0: Device now claimed by pcilatdriver

Continuing on the example above, we now have new devices registered under the driver which we can see under /sys/bus/pci/devices/0000:02:00.0/pcilatdriver/02:00.0

    # ls
    dev	pcilat_config	  pcilat_iomem	subsystem
    device	pcilat_interrupt  power		uevent
    # 

We can see that the MSI-X IRQ handlers are effectively registered:

    # ls ./msi_irqs
    46  47	48  70
    # 

The configuration settings can be changed on the driver, including setting which BAR, loop count and delay between loops:

    #cat pcilat_config 
    bar 0
    loops 1000
    ndelay 800
    #

To change options, simply write the option and the value out to the device:

    #echo "bar 3" >./pcilat_config
    #cat  ./pcilat_config
    bar 3
    loops 1000
    ndelay 800
    #


More documentation will be avialable on this when it's complete.
## iomem Testing

The driver aims to identify BARs with IORESOURCE_MEM and make them available for testing read/write latencies. 

The test is triggered by writing to the pcilat_iomem device under the driver.


The BAR tested will by default be the BAR specified in the config from above. However, the BAR may be chosen at the time of triggering the test by passing on the BAR ID to the write.

## Interrupt Timing Testing

In addition to IO performance testing on the device, the aim is to test the actual time and latencies in servicing individual portions of the interrupt. From user-land with the accompanying HPCBench plugin, we'll look to time the duration from raising the interrupt flag by writing to the character device to the time the interrupt handler is entered, and the time it's entered to the time it's serviced and completed. 

Controlling the CPU servicing the interrupt will  be done by setting the smp_affinity on the particular IRQ handler(s) prior to triggering. However, this gives a good opportunity to also test the time it takes between CPUX requesting a resource that would trigger a context switch and interrupt to the time that the interrupt serviced, handled by CPUY and returned.

## Reading the results
**NOTE: Not yet implemented.**

The driver will have devices pcilat_iomem_results and pcilat_interrupt_results. Reading these devices will print the results of the test. The results can be cleared by writing "clear" to the device.

