
BENDER ?= bender
REG_PATH ?= $(shell $(BENDER) path register_interface)
REG_TOOL ?= $(REG_PATH)/vendor/lowrisc_opentitan/util/regtool.py

REG32_FE_DIR = ./frontend
REG32_HJSON = $(REG32_FE_DIR)/idma_reg32_frontend.hjson

REG_HTML_STRING = "<!DOCTYPE html>\n<html>\n<head>\n<link rel="stylesheet" href="reg_html.css">\n</head>\n"

reg32_regs:
	$(PYTHON) $(REG_TOOL) $(REG32_HJSON) -t $(REG32_FE_DIR) -r
	$(PYTHON) $(REG_TOOL) $(REG32_HJSON) -D > $(REG32_FE_DIR)/idma_reg32_frontend.h
	printf $(REG_HTML_STRING) > $(REG32_FE_DIR)/idma_reg32_frontend.html
	$(PYTHON) $(REG_TOOL) $(REG32_HJSON) -d >> $(REG32_FE_DIR)/idma_reg32_frontend.html
	printf "</html>\n" >> $(REG32_FE_DIR)/idma_reg32_frontend.html
	cp $(REG_PATH)/vendor/lowrisc_opentitan/util/reggen/reg_html.css $(REG32_FE_DIR)	

clean:
	rm -f $(REG32_FE_DIR)/idma_reg32_frontend.h
	rm -f $(REG32_FE_DIR)/idma_reg32_frontend_reg_pkg.sv
	rm -f $(REG32_FE_DIR)/idma_reg32_frontend_reg_top.sv
	rm -f $(REG32_FE_DIR)/reg_html.css