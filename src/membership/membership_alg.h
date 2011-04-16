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

#ifndef __MEMBERSHIP_ALG__
#define __MEMBERSHIP_ALG__

#include "membership/membership_time.h"

//! find instance in the list of instances
InstancePtr find_instance(InstancePtrList instances, std::wstring name);

//! checks bmsc as the specification
MscPtr search_bmsc(MembershipContext* c, BMscPtr bmsc, BMscPtr bmsc_f);

//! checks hmsc as the specification
MscPtr search_hmsc(MembershipContext* c, HMscPtr hmsc, BMscPtr bmsc_f);

//! checks whole branch from defined node in HMSC
bool check_branch(MembershipContext* c, HMscNodePtr node, ConfigurationPtr b);

/**
 * \brief adds searched configuration to map of checked configurations
 * 
 * configuration - state of node in searched HMSC and state of bMSC which is looked for
 */
void add_checked_branch(MembershipContext* c, ReferenceNodePtr ref_node, ConfigurationPtr searched_conf);

/**
 * \brief compares strict order area from HMSC node with matching strict order area from bMSC which is looked for
 *
 * parameters: node_events - events of HMSC node cut
 *             b_events - events of bMSC cut 
 *             type: membership - checks matching events and adds receive ordering
 *                   receive_ordering - checks receive ordering
 */
bool strict_strict(MembershipContext* c, std::vector<Event*>& node_events, std::vector<Event*>& b_events, enum check_type type);

/**
 * \brief compares coregion area from HMSC node with matching strict order area from bMSC which is looked for
 *
 * parameters: node_events - events of HMSC node cut
 *             b_events - events of bMSC cut
 *             type: membership - checks matching events and adds receive ordering
 *                   receive_ordering - checks receive ordering
 */
bool coregion_strict(MembershipContext* c, std::vector<Event*>& node_events, std::vector<Event*>& b_events, enum check_type type);

/**
 *  \brief compares strict order area from HMSC node and coregion area from bMSC which is looked for
 *  DEVELOPMENT WAS STOPPED
 */
bool strict_coregion(MembershipContext* c, StrictOrderAreaPtr node_strict, std::vector<Event*>& node_events,
                     CoregionAreaPtr b_coregion, std::vector<Event*>& b_events);

/**
 * \brief compares coretion area from HMSC node and coregion area from bMSC which is looked for
 *  DEVELOPMENT WAS STOPPED
 */
bool coregion_coregion(MembershipContext* c, CoregionAreaPtr node_coregion, std::vector<Event*>& node_events,
                       CoregionAreaPtr b_coregion, std::vector<Event*>& b_events);

/**
 * \brief compares one instance from HMSC node with matching instance from bMSC which is looked for
 * 
 * parameters: node_instance - instance from HMSC node
 *             type: membership - checks matching events and adds receive ordering
 *                   receive_ordering - checks receive ordering
 *             old_position - events where the checking starts
 */
bool check_instance(MembershipContext* c, InstancePtr node_instance, enum check_type type, PositionPtr old_position);

/**
 * \brief compares bMsc from HMSC node with matching scenario in bMSC which is looked for
 *
 * parameters: node - HMSC node
 *             type: membership - checks matching events and adds receive ordering
 *                   receive_ordering - checks receive ordering
 *             old_conf - HMSC node configuration of start searching
 */
bool check_node(MembershipContext* c, ReferenceNodePtr node, enum check_type type, ConfigurationPtr old_conf);

//! check each successor of hmsc_node
bool check_next(MembershipContext* c, HMscNodePtr hmsc_node, ConfigurationPtr b);

MscPtr make_result(MembershipContext* c, HMscPtr msc);

//! makes path colored
HMscPtr color_path(MembershipContext* c, HMscPtr msc);

//! makes not full covered intervals colored
void color_intervals(MembershipContext* c);

#endif
