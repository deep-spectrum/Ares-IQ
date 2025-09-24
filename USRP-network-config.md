Run the following commands:

- ip addr show # Figure out which interface is being used
- nmcli dev status # See if the interface is managed by the network manager
- nmcli dev set \<interface\> managed no  # Tell the network manager to not manage that interface
- sudo ip addr add 192.168.10.1/24 dev <interface> # Add static IP address to interact with
- 