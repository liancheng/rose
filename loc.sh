#!/bin/sh

find include src -name "*.[chy]" -o -name "*.qx" | xargs wc -l
