package qspis_pkg;
	// qspis structure
	typedef struct packed {
		logic sd0_o;
		logic sd0_oe;
		logic sd1_o;
		logic sd1_oe;
		logic sd2_o;
		logic sd2_oe;
		logic sd3_o;
		logic sd3_oe;
	} qspis_to_pad_t;
	typedef struct packed {
		logic sd0_i;
		logic sd1_i;
		logic sd2_i;
		logic sd3_i;
		logic csn_i;
		logic sck_i;
	} pad_to_qspis_t;
endpackage