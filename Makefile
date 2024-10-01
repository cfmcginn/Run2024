CXX = g++
#CXXFLAGS += -O2 -Wall -Werror -Wextra

#Test that RUN2024DIR is setup correctly
define RUN2024DIRERR

RUN2024DIR is not set at all. Please set this environment variable to point to your build. Run
source setRUN2024Env.sh
from top level of Run2024
For more, see README for full setup recommendations

endef

ifndef RUN2024DIR
$(error "$(RUN2024DIRERR)") 
endif

ROOTFLAGS := `root-config --cflags --glibs --ldflags` -lRooFit -lRooFitCore -lMinuit #-lEG
INC=-I ${RUN2024DIR}

BUILDDIR = ./build

SRCS  = $(wildcard src/*.C)
EXES  = $(patsubst %.C,%,$(SRCS))
DEPS  = $(patsubst %.C,$(BUILDDIR)/%.d,$(SRCS))

.PHONY: all clean

all: $(EXES)

%: %.C
	@mkdir -p $(BUILDDIR)/$(@D)
	mkdir -p bin
	$(CXX) $(CXXFLAGS) -MMD -MF $(BUILDDIR)/$(@D)/$(*F).d $< -o bin/${*F}.exe $(INC) $(ROOTFLAGS) 

clean:
	@$(RM) $(EXES) $(DEPS)
	@rm -rf $(BUILDDIR)/*
	@rm -rf bin
	@rm -f *~
	@rm -f src/*~

-include $(DEPS)
