#include "Locomo.h"

#include "BE/BEMath.h"
#include "glm/gtx/rotate_vector.hpp"
#include "glm/gtx/closest_point.inl"

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

bool step(const LocomoWorld& inWorld, const LocomoBody& inBody, const LocomoState& inState, 
			 PathPlanState& inPlanState, const WalkParams& inWalk, LocomoState& outState, float& outDuration)
{
	if (inPlanState.cursor == 0)
	{
		if (inPlanState.cursor + 1 >= inPlanState.path.size())
			return false;

		glm::vec2 path_dir = glm::normalize(inPlanState.path[inPlanState.cursor+1] - inPlanState.path[inPlanState.cursor]);

		outState = inState;
		outState.l.pos = inPlanState.path[inPlanState.cursor];
		outState.l.dir = gDegConstrain360(gRadToDeg(atan2f(-path_dir.x, path_dir.y)));

		outState.r.pos = inPlanState.path[inPlanState.cursor];
		outState.r.dir = gDegConstrain360(gRadToDeg(atan2f(-path_dir.x, path_dir.y)));
	
		outState.support = (EFootFlag) (ELFlag | ERFlag);
		inPlanState.cursor = 1;

		return true;
	}

	using namespace BE;
	LocomoState next_state;

	if (!execute(inWorld, inBody, inState, inWalk, next_state, outDuration))
		return false;

	const FootState& next_foot = (next_state.support & ELFlag) ? next_state.l : next_state.r;

	float dist_cursor = 0.0f;
	float prev_dist = -1.0f;
	int new_cursor = -1;
	glm::vec2 closest_proj;
	for (int i = inPlanState.cursor; i + 1 < (int) inPlanState.path.size() && (dist_cursor < 2.0f * inWalk.stride); ++i)
	{
		glm::vec3 closest = glm::gtx::closest_point::closestPointOnLine(to3d_point(next_foot.pos), to3d_point(inPlanState.path[i]), to3d_point(inPlanState.path[i+1]));
		float dist = glm::distance(closest, to3d_point(next_foot.pos));

		if (dist >= 10.0f)
			return false;

		if (prev_dist == -1.0f || dist < prev_dist)
		{
			new_cursor = i;
			closest_proj = to2d_point(closest);
		}
		else
			break;

		prev_dist = dist;
		dist_cursor += glm::distance(inPlanState.path[i], inPlanState.path[i+1]);
	}

	if (new_cursor == -1 || new_cursor == inPlanState.cursor)
		return false;

	const FootState& support = (inState.support & ELFlag) ? inState.l : inState.r;

	glm::vec2 path_dir = glm::normalize(inPlanState.path[new_cursor+1] - inPlanState.path[new_cursor]);
	glm::vec2 path_side = glm::rotate(path_dir, 90.0f) * ((inState.support & ELFlag) ? 1.0f : -1.0f);

	glm::vec2 world_fwd = glm::rotate(inWorld.fwd, support.dir);
	glm::vec2 world_right = glm::rotate(inWorld.right, support.dir);
	glm::vec2 world_side = (inState.support & ELFlag) ? world_right : -world_right;

	glm::vec2 tune_side_min = next_foot.pos + (world_side * 0.3f * inBody.footDist);
	glm::vec2 tune_side_max = next_foot.pos - (world_side * 0.3f * inBody.footDist);

	glm::vec2 path_best = closest_proj + (path_side * inBody.footDist);
	glm::vec3 closest_tune = glm::gtx::closest_point::closestPointOnLine(to3d_point(path_best), to3d_point(tune_side_min), to3d_point(tune_side_max));
		
	outState = next_state;
	FootState& tune_next_foot = (outState.support & ELFlag) ? outState.l : outState.r;
	tune_next_foot.pos = to2d_point(closest_tune);
	tune_next_foot.dir =  gDegConstrain360(gRadToDeg(atan2f(-path_dir.x, path_dir.y)));

	inPlanState.cursor = new_cursor;

	return fabs(tune_next_foot.dir - support.dir) < 80.0f;
}

}