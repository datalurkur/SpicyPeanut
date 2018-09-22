CXX = g++
CXXFLAGS = -std=c++11 -I ../wiringPi/wiringPi -I /usr/include/soci -I ../Protocol
LDLIBS = -lwiringPi -lc -lpthread -lsoci_core -lsoci_odbc

HEADERS = command.h commandlistener.h commandqueue.h config.h dbinterface.h globalstate.h log.h schedule.h socketlistener.h state.h ucinterface.h util.h
OBJECTS = command.o commandlistener.o commandqueue.o dbinterface.o globalstate.o log.o main.o schedule.o socketlistener.o state.o ucinterface.o util.o

default: SpicyPeanut

SpicyPeanut: $(OBJECTS)
	$(CXX) $(CXXFLAGS) $^ $(LDLIBS) -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OBJECTS): $(HEADERS)

clean:
	-rm -f $(OBJECTS)
	-rm -f SpicyPeanut
