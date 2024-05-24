#!/bin/bash

trap reset_resolution INT
function reset_resolution() {
	echo "recv ctrl-c, exit"
	adb shell wm size reset
}

sleep 60
