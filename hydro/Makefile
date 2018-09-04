CXX = g++
CXXFLAGS = -std=c++11 -I ../wiringPi/wiringPi
LDLIBS = -lwiringPi -lc -lpthread

HEADERS = command.h commandqueue.h config.h log.h schedule.h state.h ucinterface.h util.h
OBJECTS = command.o commandqueue.o log.o main.o schedule.o state.o ucinterface.o util.o

default: SpicyPeanut

SpicyPeanut: $(OBJECTS)
	$(CXX) $(CXXFLAGS) $^ $(LDLIBS) -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OBJECTS): $(HEADERS)

clean:
	-rm -f $(OBJECTS)
	-rm -f SpicyPeanut
