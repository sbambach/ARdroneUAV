#Standard boilerplate
prefix = /usr/local
exec_prefix = ${prefix}
mandir = ${datarootdir}/man
includedir = ${prefix}/include
datarootdir = ${prefix}/share
pkgconfig = 

hdr = $(DESTDIR)/$(includedir)/TooN


.PHONY: all

all:
	@echo There is nothing to be compiled in TooN.
	@echo It is now ready to be installed.

install:
	mkdir -p $(hdr)
	cp *.h $(hdr)
	cp -r optimization $(hdr)/
	cp -r internal $(hdr)/
	cp -r functions $(hdr)/
	cp TooN.pc $(pkgconfig)/

internal/data_functions.hh:  make_data_functions.awk
	awk -f make_data_functions.awk > $@
internal/make_vector.hh:  make_make_vector.awk
	awk -f make_make_vector.awk > $@

internal/builtin_typeof.h:make_typeof.awk
	awk -f make_typeof.awk > $@

clean:
	rm -rf html

docs:
	doxygen 


TESTS=lu slice vector_resize gauss_jordan eigen-sqrt determinant chol_toon chol_lapack simplex sym_eigen fill so3 complex qr


TEST_RESULT=$(TESTS:%=regressions/%.result)
TEST_FILES=$(TESTS:%=regressions/%.out) $(TESTS:%=regressions/%.test) $(TEST_RESULT)

testclean:
	rm -f $(TEST_FILES)

foo:
	echo $(MAKEFLAGS)

.PHONY: test

test:regressions/results
	@echo -------------- Test Results ---------------
	@cat regressions/results

regressions/results:$(TEST_RESULT)
	cat $(TEST_RESULT) > regressions/results

.PRECIOUS: regressions/%.out regressions/%.test

#Build a test executable from a test program. On compile error,
#create an execuaable which decalres the error.
regressions/%.test: regressions/%.cc
	$(CXX) -g -ggdb $< -o $@ -llapack -DTOON_CHECK_BOUNDS -DTOON_INITIALIZE_SNAN -I ..  -I . -g -O2 -Wall -Wextra -W -O3 -Werror||\
	{ \
	  echo "echo 'Compile error!'" > $@ ; \
	  chmod +x $@; \
	}
	
regressions/%.out: regressions/%.test
	$< > $@ || ( echo Crash!!! > $@ )

regressions/%.result: regressions/%.out regressions/%.txt
	awk -vname=$* -f numdiff.awk -vf1=$< -vf2=regressions/$*.txt > $@	
	
