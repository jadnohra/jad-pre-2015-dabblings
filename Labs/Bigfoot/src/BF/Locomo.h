#ifndef _INCLUDED_BIGFOOT_LOCOMO_H
#define _INCLUDED_BIGFOOT_LOCOMO_H

#include "BFMath.h"
#include <vector>

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

	FootState& supportFoot() { return (support & ELFlag) ? l : r; }
	FootState& otherFoot() { return (support & ELFlag) ? r : l; }

	const FootState& supportFoot() const { return (support & ELFlag) ? l : r; }
	const FootState& otherFoot() const { return (support & ELFlag) ? r : l; }
};

struct WalkParams
{
	float stride;
	float speed;
	float turn;
	float side;
};

struct PathPlanState
{
	std::vector<glm::vec2> path;
	int cursor;

	WalkParams defaultWalk;
};

bool step(const LocomoWorld& inWorld, const LocomoBody& inBody, const LocomoState& inState, 
			 PathPlanState& inPlanState, const WalkParams& inDefaultParams, LocomoState& outState, float& outDuration);

bool execute(const LocomoWorld& inWorld, const LocomoBody& inBody, const LocomoState& inState, 
			 const WalkParams& inParams, LocomoState& outState, float& outDuration);


}

#endif