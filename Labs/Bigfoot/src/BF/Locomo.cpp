#include "Locomo.h"

#include "glm/gtx/rotate_vector.hpp"

namespace BF
{

bool execute(const LocomoWorld& inWorld, const LocomoBody& inBody, const LocomoState& inState, const WalkParams& inParams, LocomoState& outState, float& outDuration)
{
	const FootState& support = (inState.support & ELFlag) ? inState.l : inState.r;
	const FootState& other = (inState.support & ELFlag) ? inState.r : inState.l;

	glm::vec2 world_fwd = glm::rotate(inWorld.fwd, support.dir);
	glm::vec2 world_right = glm::rotate(inWorld.right, support.dir);
	glm::vec2 world_side = (inState.support & ELFlag) ? world_right : -world_right;

	glm::vec2 pos_mid_flight = support.pos + (world_side * inBody.footDist);
	
	float dist_fwd_sq = inParams.stride * inParams.stride - inParams.side * inParams.side;

	if (dist_fwd_sq == 0.0f)
		return false;


	float dist_fwd = sqrtf(dist_fwd_sq);
	float dist_right = inParams.side;;

	glm::vec2 pos_end = pos_mid_flight + (world_side * dist_right) + (world_fwd * dist_fwd);
	
	FootState& next_support = (inState.support & ELFlag) ? outState.r : outState.l;
	FootState& next_other = (inState.support & ELFlag) ? outState.l : outState.r;

	next_other = support;
	next_support.pos = pos_end;
	next_support.dir = support.dir + inParams.turn;

	outState.support = (EFootFlag) ((inState.support & ELFlag) ? ERFlag : ELFlag);

	outDuration = glm::distance(other.pos, pos_end) / inParams.speed;

	return true;
}

}