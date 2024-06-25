function(make_folder folder)
  if (CMAKE_GENERATOR MATCHES "Visual Studio")
    foreach(project ${ARGN})
      set_property(TARGET ${project} PROPERTY FOLDER ${folder})
    endforeach(project)
  endif()
endfunction(make_folder)