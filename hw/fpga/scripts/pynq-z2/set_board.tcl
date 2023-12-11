# Select board
set_property -name "board_part_repo_paths" -value "[file normalize "../../../hw/fpga/board_files/"]" -objects [current_project]
set_property -name "board_part" -value "tul.com.tw:pynq-z2:part0:1.0" -objects [current_project]
