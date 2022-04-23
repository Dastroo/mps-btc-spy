#!/bin/bash
if [ "$(whoami)" != root ]; then
    echo Please run this script as root or using sudo
    exit
fi

service_name="mps-btc-spy.service"
cd build || (echo "build does not exist" && exit)
make uninstall
systemctl disable --now $service_name
rm -rv /etc/systemd/system/$service_name || echo no service to uninstall
