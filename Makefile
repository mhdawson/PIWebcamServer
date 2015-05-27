all: PIWCmain

PIWCmain: PIWC.o PIWCmain.o 
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $+ -o $@ -lpaho-mqtt3cs
	
clean:
	$(RM) *.o PIWCmain
