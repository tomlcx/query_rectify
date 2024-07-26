g++ -O3 -c *.cpp ../../util/*.cpp -I ../../util/
ar -rcsv ../bin/libqueryrectify.a *.o
rm *.o
