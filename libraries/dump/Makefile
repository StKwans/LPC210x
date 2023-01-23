LIBMAKE+=../libraries/dump/Makefile
EXTRAINCDIRS+=../libraries/dump/

ZPAQDIR=../libraries/System

TARGETPATH:=$(CURDIR)
TARGETBASE=$(notdir $(TARGETPATH))

ifndef TARGET
TARGET=$(TARGETBASE)
endif

ALLTAR = $(call uniq,$(ALLSRC)) $(call uniq,$(INCDEF)) $(call uniq,$(LD_SCRIPT)) $(call uniq,$(LIBMAKE)) ../libraries/System/Doxyfile.template ../libraries/System/gnuarm_summon.sh $(ATTACH) $(ZPAQDIR)/max.cfg

CPIO_EXT = .cpio
ARCHIVE_EXT = tar
COMPRESS_EXT = gz

TARBALL = $(TARGET).$(ARCHIVE_EXT).$(COMPRESS_EXT)

$(TARGET).tar: $(ALLTAR)
	$(CC) --version --verbose > /tmp/gccversion.txt 2>&1
	tar chvf $(TARGET).tar -C .. $(sort $(addprefix $(TARGETBASE)/, $(ALLTAR)) /tmp/gccversion.txt)

$(TARGET).cpio: $(ALLTAR)
	$(CC) --version --verbose > /tmp/gccversion.txt 2>&1
	echo $(sort $(ALLTAR) /tmp/gccversion.txt) | cpio -o > $@

%.$(ARCHIVE_EXT).xz: %.$(ARCHIVE_EXT)
	xz -kf $<

%.$(ARCHIVE_EXT).bz2: %.$(ARCHIVE_EXT)
	bzip2 -kf $< 

%.$(ARCHIVE_EXT).Z: %.$(ARCHIVE_EXT)
	compress -c < $< > $@

%.$(ARCHIVE_EXT).gz: %.$(ARCHIVE_EXT)
	gzip < $< > $@

%.$(ARCHIVE_EXT).gz9: %.$(ARCHIVE_EXT)
	gzip -9 < $< > $@

%.$(ARCHIVE_EXT).lzo: %.$(ARCHIVE_EXT)
	lzop -f $<

%.$(ARCHIVE_EXT).lz: %.$(ARCHIVE_EXT)
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

%.$(ARCHIVE_EXT).mid.zpaq: %.$(ARCHIVE_EXT)
	zpaq tc$(ZPAQDIR)/mid.cfg $@ $<

%.$(ARCHIVE_EXT).max.zpaq: %.$(ARCHIVE_EXT)
	zpaq tc$(ZPAQDIR)/max.cfg $@ $<

TEST_ARCHIVE = $(TARGET).tar.xz $(TARGET).tar.bz2 $(TARGET).tar.Z $(TARGET).tar.gz $(TARGET).tar.gz9 $(TARGET).tar.lzo $(TARGET).tar.lz $(TARGET).zip $(TARGET).7z $(TARGET).tar.mid.zpaq $(TARGET).tar.max.zpaq

test_archive: $(TEST_ARCHIVE)

$(TARBALL).o: $(TARBALL)
	$(OBJCOPY) -I binary -O elf32-littlearm $(TARBALL) $(TARBALL).o --rename-section .data=.source --redefine-sym _binary_`echo $(TARBALL) | sed -e 's/\./_/g'`_start=source_start --redefine-sym _binary_`echo $(TARBALL) | sed -e 's/\./_/g'`_end=source_end -B arm

#EXTRAOBJ += $(TARBALL).o

