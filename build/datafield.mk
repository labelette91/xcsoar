# Build rules for the DataField library

DATA_FIELD_SRC_DIR = $(SRC)/DataField

DATA_FIELD_SOURCES = \
	$(DATA_FIELD_SRC_DIR)/Base.cpp \
	$(DATA_FIELD_SRC_DIR)/Boolean.cpp \
	$(DATA_FIELD_SRC_DIR)/ComboList.cpp \
	$(DATA_FIELD_SRC_DIR)/Enum.cpp \
	$(DATA_FIELD_SRC_DIR)/FileReader.cpp \
	$(DATA_FIELD_SRC_DIR)/Number.cpp \
	$(DATA_FIELD_SRC_DIR)/Float.cpp \
	$(DATA_FIELD_SRC_DIR)/Integer.cpp \
	$(DATA_FIELD_SRC_DIR)/String.cpp \

$(eval $(call link-library,datafield,DATA_FIELD))
