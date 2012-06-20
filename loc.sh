#!/bin/sh

find include src tools -name "*.[chy]" -o -name "*.qx" | xargs wc -l
