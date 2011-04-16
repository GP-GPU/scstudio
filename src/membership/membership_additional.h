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
 */

#ifndef __MEMBERSHIP_ADDITIONAL__
#define __MEMBERSHIP_ADDITIONAL__

#include "membership/membership_base.h"
#include "membership/membership_time.h"
#include "check/time/time_pseudocode.h"
#include "check/time/tightening.h"

//! compares two events 
bool compare_events(MembershipContext* c, Event* a, Event* b);

/**
 * \brief compares events attributes
 *
 * parameters: c - membership context
 *             a - event from the specification
 *             b - event from the flow
 */
bool compare_events_attribute(MembershipContext* c, Event* a, Event* b);

void compare_events_attribute_diff(MembershipContext* c, Event* spec_e, Event* flow_e);

//! tries to find configuration into map of seared configuration
bool look_at_checked_conf(MembershipContext* c, ReferenceNodePtr node, ConfigurationPtr b);

/**
 *  \brief in case events are send events, adds attribute to both events and to both receive event in case messages are complete
 *
 * parameters: node_e - event from HMSC node
 *             b_e - event from bMSC
 */
void set_identification(MembershipContext* c, Event* node_e, Event* b_e);

//! checks whether node has null pointer to reference bMSC
bool is_node_null(MembershipContext* c, ReferenceNodePtr node);

//! checks whether instance contains any event 
bool is_instance_null(InstancePtr instance);

//! checks whether coregion area contains any event 
bool is_cor_area_null(CoregionAreaPtr cor);

//!checks whether strict order area contains any event
bool is_strict_area_null(StrictOrderAreaPtr strict);

//! checks if instance contains any event
bool is_empty_instance(MembershipContext* c, InstancePtrList node_instances, InstancePtrList b_instances);

//! returns the last event on instance
Event* get_last_instance_event(MembershipContext* c, Event* start);

/** 
 * \brief finds event in bMSC on instance by id
 *
 * parameters: c - membership context
 *             label - label of instance where the event should be
 *             id - id of event which is looked for
 */
Event* find_event_on_instance_by_id(MembershipContext* c, std::wstring label, int id, StrictEventPtr start_event = NULL);
#endif
