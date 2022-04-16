PROJECTS=./consola ./cpu ./kernel ./memoria
LIBS=./utils
TESTS=./consola_tests ./cpu_tests ./kernel_tests ./memoria_tests ./utils_tests

all: $(PROJECTS)

$(PROJECTS): $(LIBS)
	$(MAKE) -C $@

$(LIBS):
	$(MAKE) -C $@

clean: clean-vgcores clean-logs
	$(foreach PROJ, $(LIBS) $(PROJECTS) $(TESTS), $(MAKE) -C $(PROJ) clean;)

release: test
	$(foreach PROJ, $(LIBS) $(PROJECTS), $(MAKE) -C $(PROJ) release;)

test:
	$(foreach PROJ, $(TESTS), $(MAKE) -C $(PROJ) start;)

clean-vgcores:
	$(RM) **/vgcore.*

clean-logs:
	$(RM) **/**/*.log

.PHONY: all $(PROJECTS) $(LIBS) $(TESTS) clean release test clean-vgcores clean-logs
