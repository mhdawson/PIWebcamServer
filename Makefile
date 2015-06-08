all: PIWCmain

PIWCmain: PIWC.o PIWCmain.o 
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $+ -o $@ -lpaho-mqtt3cs -lpthread
	
clean:
	$(RM) *.o PIWCmain
