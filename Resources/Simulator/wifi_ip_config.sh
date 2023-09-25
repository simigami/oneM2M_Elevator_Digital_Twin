#!/bin/bash
#	crontab -e
#	*/5 * * * * /home/user/Desktop/Effective-Elevator-Energy-Calculation-for-SejongAI-Center/Resources/wifi_ip_config.sh

ssid=$(/usr/sbin/iwgetid -r)
log_file="/home/user/Desktop/crontablog.txt"
timestamp=$(date +"%Y-%m-%d %H:%M:%S")

echo "Timestamp: $timestamp" >> "$log_file"
echo "SSID: $ssid" >> "$log_file"

IP_SSID1="192.168.0.205"
Gateway_SSID1="192.168.0.1"
DNS_SSID1="192.168.0.1"

IP_SSID2="192.168.100.205"
Gateway_SSID2="192.168.100.1"
DNS_SSID2="192.168.100.1"

MASK="255.255.255.0"
DNS_SUB="8.8.8.8"

# Check SSID and set IP address and DNS server accordingly
if [ "$ssid" = "TP-Link_CCDC_5G" ]; then
    ip_address=$IP_SSID1
    netmask=$MASK
    gateway=$Gateway_SSID1
    dns_server=$DNS_SSID1
    
elif [ "$ssid" = "LGU+_M200_7358BB" ]; then
    ip_address=$IP_SSID2
    netmask=$MASK
    gateway=$Gateway_SSID2
    dns_server=$DNS_SSID2
   
else
    ip_address=$IP_SSID1
    netmask=$MASK
    gateway=$Gateway_SSID1
    dns_server=$DNS_SSID1
fi

sudo sed -i '/^interface wlan0$/,$d' /etc/dhcpcd.conf

# Set the IP address and DNS server
echo -e "interface wlan0\n\
static ip_address=$ip_address/24\n\
static routers=$gateway\n\
static domain_name_servers=$dns_server $DNS_SUB" | sudo tee -a /etc/dhcpcd.conf

sudo systemctl restart dhcpcd
