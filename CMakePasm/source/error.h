#pragma once

extern void warn(int err_num);
extern void error(int err_num);
extern void error2(int err_num, const char* parameter);

extern int error_count;
extern int warning_count;

enum
{
    error_start,

    error_out_of_memory,
    error_source_string_null,
    error_unrecognized_escape_sequence,
    error_cant_open_include_file,
    error_no_input_file_specified,
    error_cant_open_input_file,
    error_opening_list_file,
    error_invalid_opcode_or_mode,
    error_writing_output_file,
    error_value_out_of_range,
    error_missing_parameter,
    error_adding_symbol,
    error_for_reg_cant_be_nested,
    error_end_section_without_section,
    error_branch_out_of_range,
    error_org_specified_more_than_once,
    error_initializing_variable,
    error_unknown_node_type,
    error_unknown_operator_type,
    error_divide_by_zero,
    error_infinite_loop_detected,
    error_expected_next,
    error_free_without_malloc,
    error_free_unknown_pointer,
    error_macro_parameter_not_found,
    error_macro_parameter_under_flow,
    error_plus_sym_not_allowed_in_macro,
    error_plus_sym_overflow,
    error_missing_output_file,
    error_output_file_specified_more_than_once,
    error_missing_symbol_file,
    error_symbol_file_specified_more_than_once,
    error_invalid_parameters,
    error_ignore_warnings_specified_more_than_once,
    error_instruction_set_specified_more_than_once,
    error_illegal_opcodes_specified_more_than_once,
    error_missing_log_file,
    error_log_file_specified_more_than_once,
    error_c64_output_format_specified_more_than_once,
    error_verbose_specified_more_than_once,
    error_missing_list_file,
    error_list_file_specified_more_than_once,
    error_opening_log_file,
    error_maximum_number_of_passes_exceeded,
    error_opening_symbol_file,
    error_opening_output_file,
    error_creating_list_node,
    error_path_name_too_long,
    error_max
};
