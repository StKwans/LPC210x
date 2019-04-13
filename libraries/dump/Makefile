LIBMAKE+=../libraries/dump/Makefile
CPPSRC+=../libraries/dump/dump.cpp
EXTRAINCDIRS+=../libraries/dump/

ZPAQDIR=../libraries/System

TARGETPATH:=$(CURDIR)
TARGETBASE=$(notdir $(TARGETPATH))

ifndef TARGET
TARGET=$(TARGETBASE)
endif

ALLTAR = $(call uniq,$(ALLSRC)) $(call uniq,$(INCDEF)) $(call uniq,$(LD_SCRIPT)) $(call uniq,$(LIBMAKE)) ../libraries/System/Doxyfile.template ../libraries/System/gnuarm_summon.sh $(ATTACH) $(ZPAQDIR)/max.cfg

TAR_EXT = max.zpaq

TARBALL = $(TARGET).tar.$(TAR_EXT)

$(TARGET).tar: $(ALLTAR)
	$(CC) --version --verbose > /tmp/gccversion.txt 2>&1
	tar chvf $(TARGET).tar -C .. $(sort $(addprefix $(TARGETBASE)/, $(ALLTAR)) /tmp/gccversion.txt)

%.tar.xz: %.tar
	xz -kf $<

%.tar.bz2: %.tar
	bzip2 -kf $< 

%.tar.Z: %.tar
	compress -c < $< > $@

%.tar.gz: %.tar
	gzip < $< > $@

%.tar.gz9: %.tar
	gzip -9 < $< > $@

%.tar.lzo: %.tar
	lzop -f $<

%.tar.lz: %.tar
	lzip -kf $<

%.zip: %.tar
	$(REMOVE) -rf $(@).tmpdir
	mkdir $(@).tmpdir
	cd $(@).tmpdir; \
	tar xf ../$(<); \
	zip -r ../$(@) *; 
	$(REMOVE) -rf $(@).tmpdir

$(TARGET).7z: $(TARGET).tar
	$(REMOVE) -rf $(@).tmpdir
	mkdir $(@).tmpdir
	cd $(@).tmpdir; \
	tar xf ../$(<); \
	7z a ../$(TARGET).7z *; 
	$(REMOVE) -rf $(@).tmpdir

%.tar.mid.zpaq: %.tar
	zpaq tc$(ZPAQDIR)/mid.cfg $@ $<

%.tar.max.zpaq: %.tar
	zpaq tc$(ZPAQDIR)/max.cfg $@ $<

TEST_ARCHIVE = $(TARGET).tar.xz $(TARGET).tar.bz2 $(TARGET).tar.Z $(TARGET).tar.gz $(TARGET).tar.gz9 $(TARGET).tar.lzo $(TARGET).tar.lz $(TARGET).zip $(TARGET).7z $(TARGET).tar.mid.zpaq $(TARGET).tar.max.zpaq

test_archive: $(TEST_ARCHIVE)

$(TARBALL).o: $(TARBALL)
	$(OBJCOPY) -I binary -O elf32-littlearm $(TARBALL) $(TARBALL).o --rename-section .data=.source --redefine-sym _binary_`echo $(TARBALL) | sed -e 's/\./_/g'`_start=source_start --redefine-sym _binary_`echo $(TARBALL) | sed -e 's/\./_/g'`_end=source_end -B arm

EXTRAOBJ += $(TARBALL).o

