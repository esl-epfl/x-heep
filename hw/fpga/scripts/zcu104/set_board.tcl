# Select board
set_property -name "board_part_repo_paths" -value "[file normalize "../../../hw/fpga/board_files/"]" -objects [current_project]
set_property -name "board_part" -value "xilinx.com:zcu104:part0:1.0" -objects [current_project]
