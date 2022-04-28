# add std cells .db files
set DB_STDCELLS "path to your std cells .db files"

# add your memory .db files
set DB_MEM  "path to your memory .db files"

# target library
set target_library      {}
lappend target_library  $DB_STDCELLS
lappend target_library  $DB_MEM

# link library
set link_library "* $target_library"

#debug output info
puts "------------------------------------------------------------------"
puts "USED LIBRARIES"
puts $link_library
puts "------------------------------------------------------------------"

set link_library " * $link_library"

