/**
 * vanilla: Datum.h
 * Copyright (c) Torr Vision Group, University of Oxford, 2016. All rights reserved.
 */

#ifndef H_VANILLA_DATUM
#define H_VANILLA_DATUM

#include <iostream>
#include <vector>

//#################### TYPEDEFS ####################

typedef std::pair<std::vector<float>, std::vector<float> > Datum;

//#################### OUTPUT ####################

std::ostream& operator<<(std::ostream& os, const Datum& d);

#endif
