include MakeDefns
# module ("library") directories
LIBDIRS = cgi index parsing utility

# application directories
APPDIRS = app

DIRS = $(LIBDIRS) $(APPDIRS)

# derived targets for sub directories
OBJDIRS = $(DIRS:%=%/obj)
DEPENDDIRS = $(DIRS:%=%/depend)

INSTALLDIRS = $(bindir) $(includedir) $(includedir)/indri $(libdir)

MF = ../src/Makefile

.PHONY: all lib clean $(DIRS) 

all: $(DIRS) 

lib: $(LIBDIRS)

$(OBJDIRS) $(DEPENDDIRS) $(INSTALLDIRS):
	$(INSTALL_DIR) $@

$(DIRS): $(OBJDIRS) $(DEPENDDIRS)
	$(MAKE) -C $@/obj -f $(MF) all

clean:
	for dir in $(DIRS) ; do $(MAKE) -C $$dir/obj -f $(MF) clean ; done

