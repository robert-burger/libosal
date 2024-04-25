#!/bin/sh
#
mkdir -p coverage
cissy run -p gcovr gcovr -v --decisions --html-details coverage/details.html -r . --filter '(.+/)?(.+)\.cc$' . 
