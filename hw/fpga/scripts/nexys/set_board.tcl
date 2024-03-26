# Select board
set_property -name "board_part_repo_paths" -value "[file normalize "../../../hw/fpga/board_files/"]" -objects [current_project]
set_property -name "board_part" -value "digilentinc.com:nexys-a7-100t:part0:1.3" -objects [current_project]
