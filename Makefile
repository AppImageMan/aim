# Project settings for AIM

## Compiler

CPPC :=		g++
ifdef RELEASE
CPPFLAGS :=	-O2 -Wall -Werror -Wno-unused-function --std=c++17 \
			-MMD -MP \
			-Iinclude
else
CPPFLAGS :=	-g -Wall -Werror -Wno-unused-function --std=c++17 \
			-MMD -MP \
			-Iinclude
endif
LD :=		g++
LDFLAGS := 	

## Project

OBJNAME :=	aim
SRC :=		$(wildcard src/*.cpp)
OBJS =		$(SRC:src/%.cpp=obj/%.o)

# Targets

## Helpers

ifdef RELEASE
.PHONY: all
all: clean $(OBJNAME)
else
.PHONY: all
all: $(OBJNAME)
endif

.PHONY: clean
clean:
	rm -rf obj/
	rm -rf *.d
	rm -rf $(OBJNAME)

obj/%.o: src/%.cpp
	mkdir -p obj/
	$(CPPC) -o $@ $(CPPFLAGS) -c $<

-include $(OBJS:.o=.d)

## Main

$(foreach file,$(SRC),$(eval $(call compile_obj,$(basename $(notdir $(file))))))

$(OBJNAME): $(OBJS)
	$(LD) -o $@ $(OBJS) $(LDFLAGS)
