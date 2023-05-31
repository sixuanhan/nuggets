# Testing for client
# Author: James Quirk
# Date: May 31, 2023

make all

#testing: zero arguments
./client

#testing: two arguments
./client plank

#testing: five arguments
./client plank 34523 james quirk

#testing: port is negative
./client plank -34532

make clean