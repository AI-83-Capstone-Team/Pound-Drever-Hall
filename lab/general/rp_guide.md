# How To Red Pitaya

# 0.0 Ecosystem
Ensure you have an SD card with a valid Red Pitaya ecosystem installed. This SD card houses the boot binary as well as the filesystem. Do not remove it while the system is running. Do not destroy or lose the SD card. SD cards are not edible, do not attempt to eat it. In the event of accidental SD card consumption, call ECE lab safety team immediately and kindly request that they revoke your lab access. 


## 1.0 Connection
Red Pitaya nominally uses ethernet to establish a host connecttion to the client. If a valid connection is present, the Red Pitaya's MAC address will be assigned an IP in your client's ARP table i.e. on Linux enter `arp -a` to view the table entries. `ip neigh` and related commands may also work here. 

## 2.0 SSH
You can SSH into the RP provided the standard ecosystem is up and running. The easiest way to do so is as the root user. On Linux, from your terminal simply type
```bash
ssh root@ip_adress_that_you_must_find
``` 

and enter `root` again when prompted for the password (root user password is root).

## SCP
Much like SSH: 
```bash
scp path_to_file root@ip_adress_that_you_must_find:/root/target_file_path
```