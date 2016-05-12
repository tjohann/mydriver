#!/usr/bin/env bash
################################################################################
#
# Title       :    gpio_script.sh    
#
# License:
#
# GPL                                                                        
# (c) 2016, thorsten.johannvorderbrueggen@t-online.de
#                                                                            
# This program is free software; you can redistribute it and/or modify       
# it under the terms of the GNU General Public License as published by       
# the Free Software Foundation; either version 2 of the License, or          
# (at your option) any later version.                                        
#                                                                            
# This program is distributed in the hope that it will be useful,            
# but WITHOUT ANY WARRANTY; without even the implied warranty of             
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the               
# GNU General Public License for more details.                                
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
#
################################################################################
#
# Date/Beginn :    12.05.2016/12.05.2016
#
# Version     :    V0.01
#
# Milestones  :    V0.01 (may 2016) -> initial version
#
# Requires    :    ...
#                 
#
################################################################################
# Description
#   
#   A simple tool to demonstrate sysfs gpio interface
#
# Some features
#   - let LED blink :-) 
#
# Notes
#   - Use PIN11(IO-0/PI19) 
#
################################################################################
#

# VERSION-NUMBER
VER='0.01'

# catch ctrl-c
trap "echo \"275\" > /sys/class/gpio/unexport" EXIT 

#
# see http://linux-sunxi.org/GPIO 
# PI19 -> (9 - 1) * 32 + 19 = 275 
# 
echo "Try to activate Pin 11 for output"
echo "275" >/sys/class/gpio/export
echo "out" >/sys/class/gpio/gpio275/direction 
echo "Activated Pin"

while true; do
    echo "set 3.3 Volt"
    echo "1" >/sys/class/gpio/gpio275/value
    sleep 1
    echo "set 0 Volt"
    echo "0" >/sys/class/gpio/gpio275/value
    sleep 1
done
