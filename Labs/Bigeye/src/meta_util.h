/******************************************************************************
 * This file is part of The AI Sandbox.
 * 
 * Copyright (c) 2008, AiGameDev.com
 * 
 * Credits:         See the PEOPLE file in the base directory.
 * License:         This software may be used for your own personal research
 *                  and education only.  For details, see the LICENSING file.
 *****************************************************************************/

#ifndef CORE_META_UTIL_H
#define CORE_META_UTIL_H

/// @file           core/meta_util.h
/// @brief          Meta Programming Helpers and utilities.

struct restricted_bool
{
    unsigned i;
};

typedef unsigned restricted_bool::*unspecified_bool_type;
const unspecified_bool_type unspecified_bool_false = 0;
const unspecified_bool_type unspecified_bool_true = &restricted_bool::i;


class noncopyable
{ 
private:  
    noncopyable(const noncopyable& x);  
    noncopyable& operator=(const noncopyable& x);  

public:  
    noncopyable() {}; 
};  


#endif // CORE_META_UTIL_H
