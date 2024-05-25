#!/bin/bash

while true
do
	echo 1 > /dev/hlekout0
	sleep 0.5
	echo 1 > /dev/hlekout1
	sleep 0.5
	echo 1 > /dev/hlekout2
	sleep 0.5

	echo 0 > /dev/hlekout0
	sleep 0.5
	echo 0 > /dev/hlekout1
	sleep 0.5
	echo 0 > /dev/hlekout2
	sleep 0.5
done


