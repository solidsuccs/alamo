
-include Makefile.pre.conf

AMREX_TARGET ?= 
CC ?= mpicxx -cxx=g++
MPI_LIB ?= -lgfortran -lmpich

RESET              = \033[0m
B_ON               = \033[1m
FG_RED             = \033[31m
FG_DIM             = \033[2m
FG_LIGHTRED        = \033[91m
FG_LIGHTGRAY       = \033[37m
FG_GRAY            = \033[90m
FG_GREEN           = \033[32m
FG_LIGHTGREEN      = \033[92m
FG_YELLOW          = \033[33m
FG_LIGHTYELLOW     = \033[93m
FG_BLUE            = \033[34m
FG_LIGHTBLUE       = \033[94m
FG_CYAN            = \033[36m
FG_MAGENTA         = \033[35m




METADATA_GITHASH  = $(shell git describe --always --dirty)
METADATA_USER     = $(shell whoami)
METADATA_PLATFORM = $(shell hostname)
METADATA_COMPILER = $(COMP)
METADATA_DATE     = $(shell date +%x)
METADATA_TIME     = $(shell date +%H:%M:%S)
BUILD_DIR         = ${shell pwd}

METADATA_FLAGS = -DMETADATA_GITHASH=\"$(METADATA_GITHASH)\" -DMETADATA_USER=\"$(METADATA_USER)\" -DMETADATA_PLATFORM=\"$(METADATA_PLATFORM)\" -DMETADATA_COMPILER=\"$(METADATA_COMPILER)\" -DMETADATA_DATE=\"$(METADATA_DATE)\" -DMETADATA_TIME=\"$(METADATA_TIME)\" -DBUILD_DIR=\"${BUILD_DIR}\" $(if ${MEME}, -DMEME)


CXX_COMPILE_FLAGS += -Winline -Wextra -Wall  -std=c++11 $(METADATA_FLAGS)

LINKER_FLAGS += -Bsymbolic-functions

INCLUDE = $(if ${EIGEN}, -isystem ${EIGEN})  $(if ${AMREX}, -isystem ${AMREX}/include/) -I./src/ $(for pth in ${CPLUS_INCLUDE_PATH}; do echo -I"$pth"; done)
LIB     = -L${AMREX}/lib/ -lamrex -lpthread

HDR_ALL = $(shell find src/ -name *.H)
HDR_TEST = $(shell find src/ -name *Test.H)
HDR = $(filter-out $(HDR_TEST),$(HDR_ALL))
SRC = $(shell find src/ -mindepth 2  -name "*.cpp" )
SRC_F = $(shell find src/ -mindepth 2  -name "*.F90" )
SRC_MAIN = $(shell find src/ -maxdepth 1  -name "*.cc" )
EXE = $(subst src/,bin/, $(SRC_MAIN:.cc=-$(POSTFIX))) 
OBJ = $(subst src/,obj/obj-$(POSTFIX)/, $(SRC:.cpp=.cpp.o)) 
DEP = $(subst src/,obj/obj-$(POSTFIX)/, $(SRC:.cpp=.cpp.d)) $(subst src/,obj/obj-$(POSTFIX)/, $(SRC_MAIN:.cc=.cc.d))
OBJ_MAIN = $(subst src/,obj/obj-$(POSTFIX)/, $(SRC_MAIN:.cpp=.cc.o))
OBJ_F = $(subst src/,obj/obj-$(POSTFIX)/, $(SRC_F:.F90=.F90.o))

NUM = $(words $(SRC) $(SRC_F) $(SRC_MAIN))
CTR = 0
NUM_DEP = $(words $(DEP))
CTR_DEP = 0
NUM_EXE = $(words $(EXE))
CTR_EXE = 0

.SECONDARY: 


default: $(DEP) $(EXE) $(DEP_EXTRA)
	@printf "$(B_ON)$(FG_GREEN)DONE $(RESET)\n" 


python: $(OBJ)
	@printf "$(B_ON)$(FG_MAGENTA)PYTHON  $(RESET)    Compiling library\n" 
	@$(CC) -x c++ -c py/alamo.cpy -fPIC -o py/alamo.cpy.o ${INCLUDE} ${PYTHON_INCLUDE} ${CXX_COMPILE_FLAGS} 
	@$(CC) -shared -Wl,-soname,alamo.so -o alamo.so py/alamo.cpy.o ${OBJ} ${LIB} ${MPI_LIB} $(PYTHON_LIB) 

tidy:
	@printf "$(B_ON)$(FG_RED)TIDYING  $(RESET)\n" 
	rm -f Backtrace*
	rm -f amrex.build.log

clean: tidy
	@printf "$(B_ON)$(FG_RED)CLEANING  $(RESET)\n" 
	find src/ -name "*.o" -exec rm {} \;
	rm -f .diff.html
	rm -f bin/*
	rm -rf obj
	rm -f Backtrace*
	rm -rf docs/build docs/doxygen docs/html docs/latex
	rm -f amrex.build.log

realclean: clean
	@printf "$(B_ON)$(FG_RED)CLEANING AMREX $(RESET)\n" 
	-make -C amrex realclean
	rm -rf amrex/1d* amrex/2d* amrex/3d*
	@printf "$(B_ON)$(FG_RED)CLEANING OLD CONFIGURATIONS $(RESET)\n" 
	rm -f Makefile.conf Makefile.amrex.conf


info:
	@printf "$(B_ON)$(FG_BLUE)Compiler version information$(RESET)\n"
	@$(CC) --version

-include Makefile.post.conf

bin/%: bin/%-$(POSTFIX) ;

bin/%-$(POSTFIX): ${OBJ_F} ${OBJ} obj/obj-$(POSTFIX)/%.cc.o 
	$(eval CTR_EXE=$(shell echo $$(($(CTR_EXE)+1))))
	@printf "$(B_ON)$(FG_BLUE)LINKING$(RESET)$(FG_LIGHTBLUE)     " 
	@printf '%9s' "($(CTR_EXE)/$(NUM_EXE)) " 
	@printf "$(RESET)$@\n"
	@mkdir -p bin/
	#./colornvcc --link -o $@ $^ ${LIB} ${MPI_LIB} amrex/3d-cuda-g++/lib/libamrex.a 
	nvcc -Wno-deprecated-gpu-targets -m64 -arch=compute_60 -code=sm_60 -maxrregcount=255 -g -G --ptxas-options=-v --use_fast_math  -DAMREX_DEBUG -DBL_USE_MPI -DAMREX_USE_MPI -DAMREX_GIT_VERSION=\"19.11-dirty\" -DBL_GCC_VERSION=5.4.0 -DBL_GCC_MAJOR_VERSION=5 -DBL_GCC_MINOR_VERSION=4 -DAMREX_NVCC_VERSION=10.1 -DAMREX_NVCC_MAJOR_VERSION=10 -DAMREX_NVCC_MINOR_VERSION=1 -DAMREX_USE_CUDA -DAMREX_LAUNCH="" -DAMREX_DEVICE="" -DAMREX_CUDA_FORT_GLOBAL="" -DAMREX_CUDA_FORT_DEVICE="" -DAMREX_CUDA_FORT_HOST="" -DAMREX_CUDA_FORT_HOST_DEVICE="" -DAMREX_USE_GPU -DBL_COALESCE_FABS -DBL_SPACEDIM=3 -DAMREX_SPACEDIM=3 -DBL_FORT_USE_UNDERSCORE -DAMREX_FORT_USE_UNDERSCORE -DBL_Linux -DAMREX_Linux -DBL_USE_ASSERTION -DAMREX_USE_ASSERTION -DAMREX_GPU_MAX_THREADS=256 -I. -I/usr/local/cuda/include -I../../..//Src/Base -L. -L/usr/lib/gcc/x86_64-linux-gnu/5/ -L/usr/local/cuda/lib64 -o $@ $^ amrex/3d-cuda-g++/lib/libamrex.a -Xlinker=-Bsymbolic-functions -Xlinker=-z,relro -I/usr/include/mpich -I/usr/include/mpich -L/usr/lib/x86_64-linux-gnu -lmpichfort -lmpich -lcuda -lgfortran -lquadmath
	#@$(CC) -o $@ $^ ${LIB}  ${MPI_LIB}  ${LINKER_FLAGS}

obj/obj-$(POSTFIX)/test.cc.o: src/test.cc ${AMREX_TARGET}
	$(eval CTR=$(shell echo $$(($(CTR)+1))))
	@printf "$(B_ON)$(FG_YELLOW)COMPILING$(RESET)$(FG_LIGHTYELLOW)   "
	@printf '%9s' "($(CTR)/$(NUM)) " 
	@printf "$(RESET)$<\n"
	@mkdir -p $(dir $@)
	./colornvcc -ccbin=mpicxx -Xcompiler=' -Werror=return-type -g -O3  -pthread  --std=c++14' --std=c++14 -Wno-deprecated-gpu-targets -m64 -arch=compute_60 -code=sm_60 -maxrregcount=255 -lineinfo --ptxas-options=-O3 --ptxas-options=-v --use_fast_math  -dc -x cu --expt-relaxed-constexpr --expt-extended-lambda -DNDEBUG -DBL_USE_MPI -DAMREX_USE_MPI -DAMREX_GIT_VERSION=\"19.11\" -DBL_GCC_VERSION=7.4.0 -DBL_GCC_MAJOR_VERSION=7 -DBL_GCC_MINOR_VERSION=4 -DAMREX_NVCC_VERSION=10.1 -DAMREX_NVCC_MAJOR_VERSION=10 -DAMREX_NVCC_MINOR_VERSION=1 -DAMREX_USE_CUDA -DAMREX_LAUNCH="" -DAMREX_DEVICE="" -DAMREX_CUDA_FORT_GLOBAL="" -DAMREX_CUDA_FORT_DEVICE="" -DAMREX_CUDA_FORT_HOST="" -DAMREX_CUDA_FORT_HOST_DEVICE="" -DAMREX_USE_GPU -DBL_COALESCE_FABS -DBL_SPACEDIM=2 -DAMREX_SPACEDIM=2 -DBL_FORT_USE_UNDERSCORE -DAMREX_FORT_USE_UNDERSCORE -DBL_Linux -DAMREX_Linux -DAMREX_PARTICLES -DAMREX_GPU_MAX_THREADS=256 -DBL_USE_F_INTERFACES -I. -I/usr/local/cuda/include -I/home/brunnels/Research/alamo/amrex/Src/Base -I/home/brunnels/Research/alamo/amrex/Src/AmrCore -I/home/brunnels/Research/alamo/amrex/Src/Amr -I/home/brunnels/Research/alamo/amrex/Src/Boundary -I/home/brunnels/Research/alamo/amrex/Src/Particle -I/home/brunnels/Research/alamo/amrex/Src/F_Interfaces/Particle -I/home/brunnels/Research/alamo/amrex/Src/F_Interfaces/Base -I/home/brunnels/Research/alamo/amrex/Src/F_Interfaces/Octree -I/home/brunnels/Research/alamo/amrex/Src/F_Interfaces/AmrCore -I/home/brunnels/Research/alamo/amrex/Src/LinearSolvers/MLMG -I/home/brunnels/Research/alamo/amrex/Src/LinearSolvers/Projections -I/home/brunnels/Research/alamo/amrex/Src/F_Interfaces/LinearSolvers $(INCLUDE) -c $< -o $@
	#@$(CC) -c $< -o $@ ${INCLUDE} ${CXX_COMPILE_FLAGS} 

obj/obj-$(POSTFIX)/%.cc.o: src/%.cc ${AMREX_TARGET} 
	$(eval CTR=$(shell echo $$(($(CTR)+1))))
	@printf "$(B_ON)$(FG_YELLOW)COMPILING$(RESET)$(FG_LIGHTYELLOW)   "
	@printf '%9s' "($(CTR)/$(NUM)) " 
	@printf "$(RESET)$<\n"
	@mkdir -p $(dir $@)
	./colornvcc -ccbin=mpicxx -Xcompiler=' -Werror=return-type -g -O3  -pthread  --std=c++14' --std=c++14 -Wno-deprecated-gpu-targets -m64 -arch=compute_60 -code=sm_60 -maxrregcount=255 -lineinfo --ptxas-options=-O3 --ptxas-options=-v --use_fast_math  -dc -x cu --expt-relaxed-constexpr --expt-extended-lambda -DNDEBUG -DBL_USE_MPI -DAMREX_USE_MPI -DAMREX_GIT_VERSION=\"19.11\" -DBL_GCC_VERSION=7.4.0 -DBL_GCC_MAJOR_VERSION=7 -DBL_GCC_MINOR_VERSION=4 -DAMREX_NVCC_VERSION=10.1 -DAMREX_NVCC_MAJOR_VERSION=10 -DAMREX_NVCC_MINOR_VERSION=1 -DAMREX_USE_CUDA -DAMREX_LAUNCH="" -DAMREX_DEVICE="" -DAMREX_CUDA_FORT_GLOBAL="" -DAMREX_CUDA_FORT_DEVICE="" -DAMREX_CUDA_FORT_HOST="" -DAMREX_CUDA_FORT_HOST_DEVICE="" -DAMREX_USE_GPU -DBL_COALESCE_FABS -DBL_SPACEDIM=2 -DAMREX_SPACEDIM=2 -DBL_FORT_USE_UNDERSCORE -DAMREX_FORT_USE_UNDERSCORE -DBL_Linux -DAMREX_Linux -DAMREX_PARTICLES -DAMREX_GPU_MAX_THREADS=256 -DBL_USE_F_INTERFACES -I. -I/usr/local/cuda/include -I/home/brunnels/Research/alamo/amrex/Src/Base -I/home/brunnels/Research/alamo/amrex/Src/AmrCore -I/home/brunnels/Research/alamo/amrex/Src/Amr -I/home/brunnels/Research/alamo/amrex/Src/Boundary -I/home/brunnels/Research/alamo/amrex/Src/Particle -I/home/brunnels/Research/alamo/amrex/Src/F_Interfaces/Particle -I/home/brunnels/Research/alamo/amrex/Src/F_Interfaces/Base -I/home/brunnels/Research/alamo/amrex/Src/F_Interfaces/Octree -I/home/brunnels/Research/alamo/amrex/Src/F_Interfaces/AmrCore -I/home/brunnels/Research/alamo/amrex/Src/LinearSolvers/MLMG -I/home/brunnels/Research/alamo/amrex/Src/LinearSolvers/Projections -I/home/brunnels/Research/alamo/amrex/Src/F_Interfaces/LinearSolvers $(INCLUDE) -c $< -o $@
	#@$(CC) -c $< -o $@ ${INCLUDE} ${CXX_COMPILE_FLAGS} 

obj/obj-$(POSTFIX)/%.cpp.o: 
	$(eval CTR=$(shell echo $$(($(CTR)+1))))
	@printf "$(B_ON)$(FG_YELLOW)COMPILING$(RESET)$(FG_LIGHTYELLOW)   "
	@printf '%9s' "($(CTR)/$(NUM)) " 
	@printf "$(RESET)$<\n"
	@mkdir -p $(dir $@)
	./colornvcc -ccbin=mpicxx -Xcompiler=' -Werror=return-type -g -O3  -pthread  --std=c++14' --std=c++14 -Wno-deprecated-gpu-targets -m64 -arch=compute_60 -code=sm_60 -maxrregcount=255 -lineinfo --ptxas-options=-O3 --ptxas-options=-v --use_fast_math  -dc -x cu --expt-relaxed-constexpr --expt-extended-lambda -DNDEBUG -DBL_USE_MPI -DAMREX_USE_MPI -DAMREX_GIT_VERSION=\"19.11\" -DBL_GCC_VERSION=7.4.0 -DBL_GCC_MAJOR_VERSION=7 -DBL_GCC_MINOR_VERSION=4 -DAMREX_NVCC_VERSION=10.1 -DAMREX_NVCC_MAJOR_VERSION=10 -DAMREX_NVCC_MINOR_VERSION=1 -DAMREX_USE_CUDA -DAMREX_LAUNCH="" -DAMREX_DEVICE="" -DAMREX_CUDA_FORT_GLOBAL="" -DAMREX_CUDA_FORT_DEVICE="" -DAMREX_CUDA_FORT_HOST="" -DAMREX_CUDA_FORT_HOST_DEVICE="" -DAMREX_USE_GPU -DBL_COALESCE_FABS -DBL_SPACEDIM=2 -DAMREX_SPACEDIM=2 -DBL_FORT_USE_UNDERSCORE -DAMREX_FORT_USE_UNDERSCORE -DBL_Linux -DAMREX_Linux -DAMREX_PARTICLES -DAMREX_GPU_MAX_THREADS=256 -DBL_USE_F_INTERFACES -I. -I/usr/local/cuda/include -I/home/brunnels/Research/alamo/amrex/Src/Base -I/home/brunnels/Research/alamo/amrex/Src/AmrCore -I/home/brunnels/Research/alamo/amrex/Src/Amr -I/home/brunnels/Research/alamo/amrex/Src/Boundary -I/home/brunnels/Research/alamo/amrex/Src/Particle -I/home/brunnels/Research/alamo/amrex/Src/F_Interfaces/Particle -I/home/brunnels/Research/alamo/amrex/Src/F_Interfaces/Base -I/home/brunnels/Research/alamo/amrex/Src/F_Interfaces/Octree -I/home/brunnels/Research/alamo/amrex/Src/F_Interfaces/AmrCore -I/home/brunnels/Research/alamo/amrex/Src/LinearSolvers/MLMG -I/home/brunnels/Research/alamo/amrex/Src/LinearSolvers/Projections -I/home/brunnels/Research/alamo/amrex/Src/F_Interfaces/LinearSolvers $(INCLUDE) -c $< -o $@
	#$(CC) -c $< -o $@ ${INCLUDE} ${CXX_COMPILE_FLAGS} 

obj/obj-$(POSTFIX)/%.cpp.d: src/%.cpp  ${AMREX_TARGET}
	$(eval CTR_DEP=$(shell echo $$(($(CTR_DEP)+1))))
	@printf "$(B_ON)$(FG_GRAY)DEPENDENCY$(RESET)$(FG_LIGHTGRAY)  " 
	@printf '%9s' "($(CTR_DEP)/$(NUM)) " 
	@printf "$(RESET)$<\n"
	@mkdir -p $(dir $@)
	@$(CC) -I./src/ $< ${INCLUDE} ${CXX_COMPILE_FLAGS} -MM -MT $(@:.cpp.d=.cpp.o) -MF $@

obj/obj-$(POSTFIX)/%.cc.d: src/%.cc ${AMREX_TARGET}
	$(eval CTR_DEP=$(shell echo $$(($(CTR_DEP)+1))))
	@printf "$(B_ON)$(FG_GRAY)DEPENDENCY$(RESET)$(FG_LIGHTGRAY)  " 
	@printf '%9s' "($(CTR_DEP)/$(NUM)) " 
	@printf "$(RESET)$<\n"
	@$(CC) -I./src/ $< ${INCLUDE} ${CXX_COMPILE_FLAGS} -MM -MT $(@:.cc.d=.cc.o) -MF $@

obj/obj-$(POSTFIX)/IO/WriteMetaData.cpp.o: .FORCE ${AMREX_TARGET}
	$(eval CTR=$(shell echo $$(($(CTR)+1))))
	@printf "$(B_ON)$(FG_LIGHTYELLOW)COMPILING$(RESET)$(FG_LIGHTYELLOW)   "
	@printf '%9s' "($(CTR)/$(NUM)) " 
	@printf "$(RESET)${subst obj/obj-$(POSTFIX)/,src/,${@:.cpp.o=.cpp}} \n"
	@mkdir -p $(dir $@)
	./colornvcc -c ${subst obj/obj-$(POSTFIX)/,src/,${@:.cpp.o=.cpp}} -o $@ -ccbin=mpicxx -Xcompiler=' -Werror=return-type -g -O3  -pthread  --std=c++14' --std=c++14 -Wno-deprecated-gpu-targets -m64 -arch=compute_60 -code=sm_60 -maxrregcount=255 -lineinfo --ptxas-options=-O3 --ptxas-options=-v --use_fast_math  -dc -x cu --expt-relaxed-constexpr --expt-extended-lambda -DNDEBUG -DBL_USE_MPI -DAMREX_USE_MPI -DAMREX_GIT_VERSION=\"19.11\" -DBL_GCC_VERSION=7.4.0 -DBL_GCC_MAJOR_VERSION=7 -DBL_GCC_MINOR_VERSION=4 -DAMREX_NVCC_VERSION=10.1 -DAMREX_NVCC_MAJOR_VERSION=10 -DAMREX_NVCC_MINOR_VERSION=1 -DAMREX_USE_CUDA -DAMREX_LAUNCH="" -DAMREX_DEVICE="" -DAMREX_CUDA_FORT_GLOBAL="" -DAMREX_CUDA_FORT_DEVICE="" -DAMREX_CUDA_FORT_HOST="" -DAMREX_CUDA_FORT_HOST_DEVICE="" -DAMREX_USE_GPU -DBL_COALESCE_FABS -DBL_SPACEDIM=2 -DAMREX_SPACEDIM=2 -DBL_FORT_USE_UNDERSCORE -DAMREX_FORT_USE_UNDERSCORE -DBL_Linux -DAMREX_Linux -DAMREX_PARTICLES -DAMREX_GPU_MAX_THREADS=256 -DBL_USE_F_INTERFACES -I. -I/usr/local/cuda/include -I/home/brunnels/Research/alamo/amrex/Src/Base -I/home/brunnels/Research/alamo/amrex/Src/AmrCore -I/home/brunnels/Research/alamo/amrex/Src/Amr -I/home/brunnels/Research/alamo/amrex/Src/Boundary -I/home/brunnels/Research/alamo/amrex/Src/Particle -I/home/brunnels/Research/alamo/amrex/Src/F_Interfaces/Particle -I/home/brunnels/Research/alamo/amrex/Src/F_Interfaces/Base -I/home/brunnels/Research/alamo/amrex/Src/F_Interfaces/Octree -I/home/brunnels/Research/alamo/amrex/Src/F_Interfaces/AmrCore -I/home/brunnels/Research/alamo/amrex/Src/LinearSolvers/MLMG -I/home/brunnels/Research/alamo/amrex/Src/LinearSolvers/Projections -I/home/brunnels/Research/alamo/amrex/Src/F_Interfaces/LinearSolvers $(INCLUDE)  ${METADATA_FLAGS} 
	#@$(CC) -c ${subst obj/obj-$(POSTFIX)/,src/,${@:.cpp.o=.cpp}} -o $@ ${INCLUDE} ${CXX_COMPILE_FLAGS} 

.PHONY: .FORCE

FORT_INCL = $(shell for i in ${CPLUS_INCLUDE_PATH//:/ }; do echo -I'$i'; done)

obj/obj-$(POSTFIX)/%.F90.o: src/%.F90 
	@printf "$(B_ON)$(FG_YELLOW)COMPILING  $(RESET)$<\n" 
	@mkdir -p $(dir $@)
	mpif90 -c $< -o $@  -I${subst :, -I,$(CPLUS_INCLUDE_PATH)}
	rm *.mod -rf

docs: docs/doxygen/index.html docs/build/html/index.html .FORCE 
	@printf "$(B_ON)$(FG_MAGENTA)DOCS$(RESET) Done\n" 

docs/doxygen/index.html: $(SRC) $(SRC_F) $(SRC_MAIN) $(HDR_ALL)
	@printf "$(B_ON)$(FG_MAGENTA)DOCS$(RESET) Generating doxygen files\n" 	
	@cd docs && doxygen 
docs/build/html/index.html: $(shell find docs/source/ -type f) Readme.rst
	@printf "$(B_ON)$(FG_MAGENTA)DOCS$(RESET) Generating sphinx\n" 	
	@make -C docs html > /dev/null


ifneq ($(MAKECMDGOALS),tidy)
ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),realclean)
ifneq ($(MAKECMDGOALS),info)
ifneq ($(MAKECMDGOALS),help)
ifneq ($(MAKECMDGOALS),docs)
-include $(DEP)
endif
endif
endif
endif
endif
endif

