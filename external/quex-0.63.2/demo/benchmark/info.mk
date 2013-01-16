#   _______ _______ _______ _     _  _____ 
#   |______ |______    |    |     | |_____]
#   ______| |______    |    |_____| |      
#                                            
# General information about test setup and author
#______________________________________________________________________________

AUTHOR_EMAIL = '"fschaef@users.sourceforge.net"'
AUTHOR_NAME  = "Frank-Rene Schaefer"
COMPILER     = g++    # icpc # g++ 
COMPILER_V   = "4.3.1"  # 10.1 # 4.3.1
COMPILER_OPT = -Os -march=native# -O3 # -O3 -ggdb -Os
BUFFER_SIZE  = 65536
CPU_NAME     = 'Intel Core Duo'
CPU_CODE     = T2400
# Following two lines work only under linux and similar systems.
# Fill in the data by hand, if they do not work on yours.
CPU_MHZ      = $(word 4, $(shell cat /proc/cpuinfo  | grep -e 'cpu MHz'))
OS_NAME      = $(shell uname -s -r)
NOTE         =# 

