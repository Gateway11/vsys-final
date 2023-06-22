#CXX = icpc
CXX = g++
#MKLROOT=/opt/intel/composer_xe_2013.5.192/mkl
#CXX = icpc
#MYFLAGS = -DTIME_INFO  -Wno-deprecated -DINT_WID
#MYFLAGS = -DTIME_INFO  -Wno-deprecated -DNBEST
#MYFLAGS = -DTIME_INFO  -Wno-deprecated -fno-omit-frame-pointer 
#CXXFLAGS = -O2 -msse2 -I $(HOME)/include -I/home/harry/tools/openfst/openfst-1.3.2/include/ $(MYFLAGS) 
CXXFLAGS = -O2 -msse2 -I ./ -I $(HOME)/include -I/usr/local/include/ -fPIC $(MYFLAGS)  
#LDFLAGS = -L$(HOME)/lib -liniparser -lam-gmm-feature -static
#LDFLAGS = -L$(HOME)/lib -liniparser -lam-gmm-varmix -static
#LDFLAGS = -L../libs -Wl,-Bstatic -lmkl_intel_lp64  -lmkl_core -lfftw3f -liniparser -Wl,-Bdynamic -fPIC -shared
#LDFLAGS = -Wl,--start-group  $(MKLROOT)/lib/intel64/libmkl_intel_lp64.a $(MKLROOT)/lib/intel64/libmkl_intel_thread.a $(MKLROOT)/lib/intel64/libmkl_core.a -Wl,--end-group -lpthread -lm -L../libs -Wl,-Bstatic -lfftw3f -liniparser -Wl,-Bdynamic -fPIC -shared
#LDFLAGS = -Wl,--start-group  $(MKLROOT)/lib/intel64/libmkl_intel_lp64.a $(MKLROOT)/lib/intel64/libmkl_intel_thread.a $(MKLROOT)/lib/intel64/libmkl_core.a -Wl,--end-group -liomp5 -lpthread -lm -L../libs -Wl,-Bstatic -lfftw3f -liniparser -Wl,-Bdynamic -fPIC -shared
#LDFLAGS = -D_GNU_SOURCE -L/opt/intel/composer_xe_2013.5.192/mkl/lib/intel64 -Wl,--start-group  $(MKLROOT)/lib/intel64/libmkl_intel_lp64.a $(MKLROOT)/lib/intel64/libmkl_intel_thread.a $(MKLROOT)/lib/intel64/libmkl_core.a /opt/intel/composer_xe_2013.5.192/compiler/lib/intel64/libiomp5.a -Wl,--end-group -lpthread -lm -L../libs -Wl,-Bstatic -lfftw3f -liniparser -Wl,-Bdynamic -fPIC -shared
#LDFLAGS = -D_GNU_SOURCE -L../libs -Wl,-Bstatic -lmkl_intel_lp64 -lmkl_sequential -lmkl_core -lfftw3f -liniparser -Wl,-Bdynamic -fPIC -shared


#LDFLAGS = -L$(HOME)/lib/mkl -Wl,-Bstatic -lmkl_intel_lp64 -lmkl_sequential -lmkl_core -lfftw3f -Wl,-Bdynamic -fPIC -shared
#LDFLAGS = -L$(HOME)/lib -liniparser -lfftw3f -L/usr/local/lib -lfst -L$(HOME)/lib/mkl -lmkl_intel_lp64 -lmkl_sequential -lmkl_core -fPIC -shared
#LDFLAGS = -L$(HOME)/lib -liniparser -lam-gmm-dnn-gpu -L/opt/intel/Compiler/11.1/072/mkl/lib/em64t/ -lmkl_intel_lp64 -lmkl_sequential -lmkl_core 

#CUDATKDIR = /usr/local/cuda/
#CXXFLAGS += -DHAVE_CUDA -I$(CUDATKDIR)/include
#LDFLAGS += -L$(CUDATKDIR)/lib64 -Wl,-rpath,$(CUDATKDIR)/lib64
#LDFLAGS += -lcublas -lcudart -lcuda

SOURCES = \
	AllocSpace.cpp \
	feintf.cpp \
	FloatFFT.cpp \
	Pitch.cpp \
	PLP.cpp

OBJ = \
	AllocSpace.o \
	feintf.o \
	FloatFFT.o \
	Pitch.o \
	PLP.o

all: lib libso

lib: $(OBJ)
	ar -cvq -o ../bin/libfeplp.a $(OBJ)

libso: $(OBJ)
	$(CXX) $(CXXFLAGS) -shared -o ../lib/libfeplp.so $(OBJ) ../backup_fixhistory_mod2_compat_update.5.time.part.endsil.bigvoc.nosilback/iniparser.o ../backup_fixhistory_mod2_compat_update.5.time.part.endsil.bigvoc.nosilback/dictionary.o $(LDFLAGS)  -ldl -lpthread

feintf.o: feintf.h PLP.h PLP.o

PLP.o: PLP.h AllocSpace.h AllocSpace.o

#Line.o: Line.h pevents.h Recognize.h

clean:
	rm -f libfeplp.a libfeplp.so *.o
