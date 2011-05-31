# Build rules for the HTTP client library

LIBNET_SOURCES =

ifeq ($(HAVE_WIN32),y)
LIBNET_SOURCES += \
	$(SRC)/Net/Session.cpp \
	$(SRC)/Net/Connection.cpp \
	$(SRC)/Net/Request.cpp
endif

LIBNET_OBJS = $(call SRC_TO_OBJ,$(LIBNET_SOURCES))
LIBNET_LIBS = $(TARGET_OUTPUT_DIR)/libnet.a

$(LIBNET_LIBS): $(LIBNET_OBJS)
	@$(NQ)echo "  AR      $@"
	$(Q)$(AR) $(ARFLAGS) $@ $^