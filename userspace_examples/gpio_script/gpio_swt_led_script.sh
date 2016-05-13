#!/usr/bin/env bash
################################################################################
#
# Title       :    gpio_swt_led_script.sh   
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
# Date/Beginn :    13.05.2016/13.05.2016
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
#   - switch LED on/off based on input swt :-) 
#
# Notes
#   - Use PIN11(IO-0/PI19) for LED
#   - Use PIN13(IO-2/PI18) for switch
#
################################################################################
#

# VERSION-NUMBER
VER='0.01'

# catch ctrl-c and ...
function cleanup {
    echo "0" >/sys/class/gpio/gpio275/value
    echo \"275\" >/sys/class/gpio/unexport
    echo \"274\" >/sys/class/gpio/unexport
}
trap cleanup EXIT

#
# see http://linux-sunxi.org/GPIO 
# PI19 -> (9 - 1) * 32 + 19 = 275 
# 
echo "Try to activate Pin 11 for output"
echo "275" >/sys/class/gpio/export
echo "out" >/sys/class/gpio/gpio275/direction 
echo "Activated Pin"

# 
# PI18 -> (9 - 1) * 32 + 18 = 274
# 
echo "Try to activate Pin 13 for input"
echo "274" >/sys/class/gpio/export
echo "in" >/sys/class/gpio/gpio274/direction 
echo "Activated Pin"

#
# main loop
#
while true; do
    echo "read switch value and set pin (every second)"
    cat /sys/class/gpio/gpio274/value >/sys/class/gpio/gpio275/value
    sleep 1
done
