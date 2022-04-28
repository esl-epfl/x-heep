# add std cells .db files
set DB_STDCELLS {}

#lappend DB_STDCELLS "path to your std cells .db files"
#lappend DB_STDCELLS "path to your std cells .db files"
#lappend DB_STDCELLS "path to your std cells .db files"
#lappend DB_STDCELLS ...

# add your memory .db files
set DB_MEM {}

#lappend DB_MEM "path to your memory .db files"
#lappend DB_MEM "path to your memory .db files"
#lappend DB_MEM "path to your memory .db files"
#lappend DB_MEM ...


# target library
set target_library      {}
set target_library  "$DB_STDCELLS $DB_MEM"

# link library
set link_library "* $target_library"

#debug output info
puts "------------------------------------------------------------------"
puts "USED LIBRARIES"
puts $link_library
puts "------------------------------------------------------------------"

set link_library " * $link_library"
