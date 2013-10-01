CXXFLAGS=-DDEBUG
#CXXFLAGS=-DWRITE_TO_FILE
#CXXFLAGS=-DSEND_DATA
LDFLAGS=-lpthread -lcurl

EXE=weatherd
BIN=/usr/bin/weatherd

all: $(EXE)

$(EXE): Tx7Receiver.o WeatherMonitor.cpp HttpNotifier.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $+ -o $@ -lwiringPi
	

clean:
	$(RM) *.o $(EXE)

install: all init.weatherd
	sudo cp $(EXE) $(BIN)
	sudo cp init.weatherd /etc/init.d/weatherd
	sudo cp default.weatherd /etc/default/weatherd

