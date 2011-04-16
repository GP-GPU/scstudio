/*
 * scstudio - Sequence Chart Studio
 * http://scstudio.sourceforge.net
 *
 *      This library is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU Lesser General Public
 *      License version 2.1, as published by the Free Software Foundation.
 *
 *      This library is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *      Lesser General Public License for more details.
 *
 * Copyright (c) 2008 Matus Madzin <gotti@mail.muni.cz>
 *
 * $Id: Context.h 1006 2010-12-08 20:34:49Z madzin $
 */

#ifndef _Z120_CONTEXT_H
#define _Z120_CONTEXT_H

/*
 * we use ANTRL v3 (ANTLR 3.1.1)
 * see http://www.antlr.org
 */
#ifdef __cplusplus
extern "C"
{
#endif

struct Context;
struct s_Msc;
struct s_Z120;

enum msg_kind {output, input};
enum relation_kind {before, after};
enum time_relation_kind {top, bottom, unknown, time_def};

void my_print(char* a);

void set_msc_name_fun(struct Context* context, char* name);

void init(struct Context* context);

struct s_Msc** get_total_msc_fun(struct Context* context);

struct Context* new_context();

void add_z_fun(struct Context* context, struct s_Z120* z);

void delete_context(struct Context* context);

void check_collections_fun(struct Context* context);

void check_references_fun(struct Context* context);

void msc_was_read_fun(struct Context* context);

void bug_report_fun(struct Context* context, char* report);

void clear_time(struct Context* context);

void add_time_fun(struct Context* context, char* time_part);

void duplicate_time_point(struct Context* context);

void set_time_fun(struct Context* context, char* time);

void set_absolut_time_true_fun(struct Context* context);

void set_absolut_first_or_second_true_fun(struct Context* context);

void set_origin_fun(struct Context* context);

void set_time_dest_fun(struct Context* context);

void set_end_msc_fun(struct Context* context);

//BMsc
void new_bmsc_fun(struct Context* context);

void incomplete_message_fun(struct Context* context, char* msg_identifications, enum msg_kind kind);

void new_instance_fun(struct Context* context);

void start_coregion_fun(struct Context* context);

void end_coregion_fun(struct Context* context);

void message_fun(struct Context* context, char* msg_identifications, enum msg_kind kind);

void add_order_event_fun(struct Context* context, char* name);

void add_relation_fun(struct Context* context, enum relation_kind kind);

void set_instance_name_fun(struct Context* context, char* name);

void set_event_name_fun(struct Context* context, char* name);

void set_not_create_event_fun(struct Context* context);

void missing_message_label_fun(struct Context* context);

void end_instance_fun(struct Context* context);

void set_time_event_fun(struct Context* context);

void set_time_event_false_fun(struct Context* context);

void add_time_relation_event_fun(struct Context* context);

void add_event_absolut_time_fun(struct Context* context);

void set_time_reference_event_fun(struct Context* context, char* name);

void create_future_time_relations(struct Context* context);

void set_data_parameter_decl(struct Context* context);

void set_instance_parameter_decl(struct Context* context);

void set_message_parameter_decl(struct Context* context);

void set_timer_parameter_decl(struct Context* context);


//HMsc
void new_hmsc_fun(struct Context* context);

void new_start_node_fun(struct Context* context);

void new_end_node_fun(struct Context* context);

void set_reference_name_fun(struct Context* context, char* name);

void set_condition_name_fun(struct Context* context, char* name);

void clear_condition_name_fun(struct Context* context);

void set_node_name_fun(struct Context* context, char* name);

void add_connect_name_fun(struct Context* context, char* name);

void new_reference_node_fun(struct Context* context);

void new_connection_node_fun(struct Context* context);

void new_condition_node_fun(struct Context* context);

void create_connections_fun(struct Context* context);

void future_connection_fill_in_fun(struct Context* context);

void set_time_reference_node_fun(struct Context* context, char* name);

void add_time_relation_ref_time_fun(struct Context* context);

void add_time_relation_ref_fun(struct Context* context);

void set_first_time_rel_kind_fun(struct Context* context, enum time_relation_kind kind);

void set_second_time_rel_kind_fun(struct Context* context, enum time_relation_kind kind);


//Comment
void add_global_comment(struct Context* context, char* text);

void add_element_comment(struct Context* context, char* text);

#ifdef __cplusplus
}
#endif

#endif /* _Z120_CONTEXT_H */

// $Id: Context.h 1006 2010-12-08 20:34:49Z madzin $
