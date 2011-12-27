HARNESS_SOURCES = \
	$(SRC)/Computer/FlyingComputer.cpp \
	$(SRC)/Replay/IGCParser.cpp \
	$(SRC)/Replay/IgcReplay.cpp \
	$(SRC)/Replay/TaskAutoPilot.cpp \
	$(SRC)/Replay/AircraftSim.cpp \
	$(SRC)/UtilsText.cpp \
	$(SRC)/ComputerSettings.cpp \
	$(SRC)/Tracking/TrackingSettings.cpp \
	$(SRC)/Computer/TraceComputer.cpp \
	$(SRC)/OS/Clock.cpp \
	$(SRC)/Thread/Mutex.cpp \
	$(SRC)/Airspace/AirspaceComputerSettings.cpp \
	$(TEST_SRC_DIR)/Printing.cpp \
	$(TEST_SRC_DIR)/AirspacePrinting.cpp \
	$(TEST_SRC_DIR)/TaskPrinting.cpp \
	$(TEST_SRC_DIR)/ContestPrinting.cpp \
	$(TEST_SRC_DIR)/test_debug.cpp \
	$(TEST_SRC_DIR)/harness_aircraft.cpp \
	$(TEST_SRC_DIR)/harness_airspace.cpp \
	$(TEST_SRC_DIR)/harness_flight.cpp \
	$(TEST_SRC_DIR)/harness_waypoints.cpp \
	$(TEST_SRC_DIR)/harness_task.cpp \
	$(TEST_SRC_DIR)/harness_task2.cpp \
	$(TEST_SRC_DIR)/TaskEventsPrint.cpp \
	$(TEST_SRC_DIR)/tap.c

HARNESS_CPPFLAGS_INTERNAL = -DDO_PRINT

$(eval $(call link-library,harness,HARNESS))
