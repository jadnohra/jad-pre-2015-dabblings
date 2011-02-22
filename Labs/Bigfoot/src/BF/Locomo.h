#ifndef _INCLUDED_BIGFOOT_LOCOMO_H
#define _INCLUDED_BIGFOOT_LOCOMO_H

#include "BFMath.h"

namespace BF 
{

enum EFoot
{
	ELFoot = 0, ERFoot = 1
};

enum EFootFlag
{
	ELFlag = 1 << 0, 
	ERFlag = 1 << 1, 
};

struct FootState
{
	glm::vec2 pos;
	float dir;
};

struct LocomoWorld
{
	glm::vec2 fwd;
	glm::vec2 right;
};

struct LocomoBody
{
	float footDist;
};

struct LocomoState
{
	FootState l;
	FootState r;
	EFootFlag support;
};

struct WalkParams
{
	float stride;
	float speed;
	float turn;
	float side;
};


bool execute(const LocomoWorld& inWorld, const LocomoBody& inBody, const LocomoState& inState, 
			 const WalkParams& inParams, LocomoState& outState, float& outDuration);


}

#endif