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

#ifndef __MEMBERSHIP_TIME__
#define __MEMBERSHIP_TIME__

#include "membership/membership_additional.h"

//! checks time constraints in HMSC node
bool check_node_time(MembershipContext* c, ReferenceNodePtr node, ConfigurationPtr conf);

//! checks time intervals from top to bottom on one HMSC node
bool check_node_time_themself(MembershipContext* c, const std::vector<TimeRelationRefNodePtr>& themself_vec,
                              ReferenceNodePtr node, ConfigurationPtr conf);

//! finds the minimum event of HMSC node and returns time interval value of matching event in bMSC
Event* get_min_event(MembershipContext* c, ConfigurationPtr conf);

//! finds the maximum event of HMSC node and returns time interval value of matching event in bMSC  
Event* get_max_event(MembershipContext* c, ReferenceNodePtr node, ConfigurationPtr conf);

//! compares each event from the set with new_elemetn and updates the set of maximal events (second parameter)
void update_maximum_set(MembershipContext* c, std::set<Event*>& max, Event* new_element);

//! compares each event from the set with new_elemetn and updates the set of minimal events (second parameter)
void update_minimum_set(MembershipContext* c, std::set<Event*>& min, Event* new_element);

//! eliminates set of possible minimal events
std::set<Event*> eliminate_posible_minimal(MembershipContext* c, std::vector<Event*> pos_min);

//! eliminates set of possible maximal events 
std::set<Event*> eliminate_posible_maximal(MembershipContext* c, std::vector<Event*> pos_max);

/**                                 
 *  \brief compares absolut position of two events
 *
 *   Is the fist parameter before (more minimal) than the second? 
 *                                 
 *   return: 0 - it is not decidable
 *           1 - the fist is before the second
 *           2 - the second is before the first
 *           3 - error
 */
int compare_absolut_position(MembershipContext* c, Event* first, Event* second);

/**
 *  \brief compares events time relations
 *
 *  Checks whether the time relations in these events are same
 *
 *  paramterers: c - membership context
 *               a - HMSC node
 *               b - bMSC node
 */
bool compare_relative_time_constraints(MembershipContext* c, Event* a, Event* b);

//! returns matrix of time intervals among each nodes
BMscIntervalSetMatrix get_bmsc_matrix(BMscPtr bmsc_f);

/**
 * \brief returns map of instances and event identifications
 *
 *  Returns map where the key is instance name and the value is vector of the event identifications
 * (in case of coregion area)
 */
std::map<std::wstring, std::vector<int> > get_node_last_events(MembershipContext* c, ReferenceNodePtr node);

//! checks bottom time relations
bool check_bottom_time_relations(MembershipContext* c, std::vector<TimeRelationRefNodePtr> bottom_vec, Event* max_b);

//! checks top time relations
bool check_top_time_relations(MembershipContext* c, std::vector<TimeRelationRefNodePtr> top_vec, Event* min_b);

//! checks relative and absolut time constraints
void analyze_time_constraints(MembershipContext* c, Event* b_e);

MscTimeIntervalSetD get_continous_interval(MembershipContext* c, MscTimeIntervalSetD& a, MscTimeIntervalSetD& b);
#endif
