# Project settings for Z-Craft engine

## Compiler

CPPC :=		g++
ifdef RELEASE
CPPFLAGS :=		-O2 -Wall -Werror -Wno-unused-function --std=c++17 \
				-MMD -MP \
				-Iinclude
else
CPPFLAGS :=		-g -Wall -Werror -Wno-unused-function --std=c++17 \
				-MMD -MP \
				-Iinclude
endif
LD :=			g++
LDFLAGS :=

## Project

OBJNAME :=		aim
SRC :=			$(wildcard src/*.cpp)
OBJS =			$(SRC:src/%.cpp=obj/%.o)

## Packaging

ifdef RELEASE
AITOOL_SRC :=	https://github.com/AppImage/AppImageKit/releases/download/continuous/appimagetool-x86_64.AppImage
AITOOL :=		appimagetool-x86_64.AppImage
APPDIR :=		$(OBJNAME).AppDir
endif

# Targets

## Helpers

ifdef RELEASE
.PHONY: all
all: clean /realworld/$(OBJNAME).AppImage
else
.PHONY: all
all: $(OBJNAME)
endif

.PHONY: clean
clean:
	rm -rf obj/
	rm -rf *.d
	rm -rf $(OBJNAME)
	-rm -rf $(APPDIR)

obj/%.o: src/%.cpp
	mkdir -p obj/
	$(CPPC) -o $@ $(CPPFLAGS) -c $<

-include $(OBJS:.o=.d)

ifdef RELEASE
$(AITOOL):
	wget $(AITOOL_SRC)
	chmod +x $@

$(APPDIR)/usr/bin/$(OBJNAME): $(OBJNAME)
	mkdir -p $(APPDIR)/usr/bin
	cp $< $@

$(APPDIR)/$(OBJNAME).desktop: $(OBJNAME).desktop
	mkdir -p $(APPDIR)
	cp $< $@

$(APPDIR)/$(OBJNAME).svg: $(OBJNAME).svg
	mkdir -p $(APPDIR)
	cp $< $@

$(APPDIR)/AppRun: $(OBJNAME)
	mkdir -p $(APPDIR)
	ln -s usr/bin/$(OBJNAME) $@
endif

## Main

$(foreach file,$(SRC),$(eval $(call compile_obj,$(basename $(notdir $(file))))))

$(OBJNAME): $(OBJS)
	$(LD) -o $@ $(OBJS) $(LDFLAGS)

ifdef RELEASE
$(OBJNAME)-x86_64.AppImage: $(AITOOL) $(APPDIR)/usr/bin/$(OBJNAME) $(APPDIR)/$(OBJNAME).desktop $(APPDIR)/$(OBJNAME).svg $(APPDIR)/AppRun
	./$(AITOOL) --appimage-extract-and-run $(APPDIR)

/realworld/$(OBJNAME).AppImage: $(OBJNAME)-x86_64.AppImage
	cp $< $@
endif