# testing for server
# Author: Sixuan Han
# Date: May 24 2023

make all

# edge cases for incorrect args
#testing: zero argument
./server

#testing: three argument
./server maps/main.txt 1 2

#testing: map file does not exist
./server maps/no-map.txt 1

#testing: unreadable map file
touch unreadable.txt
chmod -r unreadable.txt
./server unreadable.txt 1
rm -f unreadable.txt

#testing: seed argument is not int
./server maps/main.txt abc

make clean
