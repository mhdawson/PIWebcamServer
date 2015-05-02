all: PIWCmain

PIWCmain: PIWC.o PIWCmain.o 
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $+ -o $@ -lpaho-mqtt3c
	
clean:
	$(RM) *.o PIWCmain
