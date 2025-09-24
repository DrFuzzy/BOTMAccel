#*****************************************************************************************
# Vivado (TM) v2025.1 (64-bit)
#
# project.tcl: Tcl script for re-creating project 'project_1'
#
#*****************************************************************************************

# Set the reference directory for source file relative paths (by default the value is script directory path)
set origin_dir "."

# Set the project name
set proj_name "project_1"

variable script_file
set script_file "project.tcl"

# Create project
create_project $proj_name $origin_dir/./vivado/$proj_name

# Set the directory path for the new project
set proj_dir [get_property directory [current_project]]

# Set project properties
set obj [current_project]
set_property -name "default_lib" -value "xil_defaultlib" -objects $obj
# set_property -name "enable_vhdl_2008" -value "1" -objects $obj
set_property -name "ip_cache_permissions" -value "read write" -objects $obj
set_property -name "ip_output_repo" -value "$proj_dir/${proj_name}.cache/ip" -objects $obj
set_property -name "mem.enable_memory_map_generation" -value "1" -objects $obj
set_property -name "part" -value "xc7z020clg400-1" -objects $obj
set_property -name "platform.board_id" -value "pynq-z1" -objects $obj
set_property -name "revised_directory_structure" -value "1" -objects $obj
set_property -name "sim.central_dir" -value "$proj_dir/${proj_name}.ip_user_files" -objects $obj
set_property -name "sim.ip.auto_export_scripts" -value "1" -objects $obj
set_property -name "simulator_language" -value "Mixed" -objects $obj
# set_property -name "sim_compile_state" -value "1" -objects $obj
# set_property -name "use_inline_hdl_ip" -value "1" -objects $obj
# set_property -name "webtalk.modelsim_export_sim" -value "11" -objects $obj
# set_property -name "webtalk.questa_export_sim" -value "11" -objects $obj
# set_property -name "webtalk.riviera_export_sim" -value "11" -objects $obj
# set_property -name "webtalk.vcs_export_sim" -value "11" -objects $obj
# set_property -name "webtalk.xsim_export_sim" -value "11" -objects $obj
# set_property -name "xpm_libraries" -value "XPM_CDC XPM_FIFO XPM_MEMORY" -objects $obj

# Create 'sources_1' fileset (if not found)
set obj [get_filesets sources_1]
if {[string equal [get_filesets -quiet sources_1] ""]} {
  create_fileset -srcset sources_1
}

# Set 'sources_1' fileset properties
set_property -name "dataflow_viewer_settings" -value "min_width=16" -objects $obj
set_property -name "top" -value "design_3_wrapper" -objects $obj
set_property -name "top_auto_set" -value "0" -objects $obj

# Set IP repository paths
if { $obj != {} } {
   set_property "ip_repo_paths" "[file normalize "$origin_dir/../src/hls/hls_component/aco"]" $obj

   # Rebuild user ip_repo's index before adding any source files
   update_ip_catalog -rebuild
}

# Create 'constrs_1' fileset (if not found)
if {[string equal [get_filesets -quiet constrs_1] ""]} {
  create_fileset -constrset constrs_1
}

# Set 'constrs_1' fileset object
set obj [get_filesets constrs_1]

# Set 'constrs_1' fileset properties
set_property -name "target_part" -value "xc7z020clg400-1" -objects $obj

# Create 'sim_1' fileset (if not found)
if {[string equal [get_filesets -quiet sim_1] ""]} {
  create_fileset -simset sim_1
}

# Set 'sim_1' fileset object
set obj [get_filesets sim_1]

# Set 'sim_1' fileset properties
set obj [get_filesets sim_1]
set_property -name "sim_wrapper_top" -value "1" -objects $obj
set_property -name "top" -value "design_3_wrapper" -objects $obj
set_property -name "top_lib" -value "xil_defaultlib" -objects $obj

# Create 'synth_1' run (if not found)
if {[string equal [get_runs -quiet synth_1] ""]} {
    create_run -name synth_1 -part xc7z020clg400-1 -flow {Vivado Synthesis 2025} -strategy "Vivado Synthesis Defaults" -report_strategy {No Reports} -constrset constrs_1
} else {
  set_property strategy "Vivado Synthesis Defaults" [get_runs synth_1]
  set_property flow "Vivado Synthesis 2025" [get_runs synth_1]
}
set obj [get_runs synth_1]

# Set the current synth run
current_run -synthesis [get_runs synth_1]

# Create 'impl_1' run (if not found)
if {[string equal [get_runs -quiet impl_1] ""]} {
    create_run -name impl_1 -part xc7z020clg400-1 -flow {Vivado Implementation 2025} -strategy "Vivado Implementation Defaults" -report_strategy {No Reports} -constrset constrs_1 -parent_run synth_1
} else {
  set_property strategy "Vivado Implementation Defaults" [get_runs impl_1]
  set_property flow "Vivado Implementation 2025" [get_runs impl_1]
}
set obj [get_runs impl_1]
set_property -name "steps.write_bitstream.args.readback_file" -value "0" -objects $obj
set_property -name "steps.write_bitstream.args.verbose" -value "0" -objects $obj

# Set the current impl run
current_run -implementation [get_runs impl_1]

puts "INFO: Project created:${proj_name}"

# Create block design
# source $origin_dir/./bd/design_1.tcl
# source $origin_dir/./bd/design_2.tcl
# source $origin_dir/./bd/design_3.tcl

read_bd $origin_dir/./bd/design_1.bd
read_bd $origin_dir/./bd/design_2.bd
read_bd $origin_dir/./bd/design_3.bd

# Generate the wrapper
set design_name "design_3"
make_wrapper -files [get_files $design_name.bd] -top -import
