#ifndef _RENDERING_H
#define _RENDERING_H

#include "NeHeGL.h"

#define FREEGLUT_STATIC
#include <gl\freeglut.h>

#include "arb_multisample.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/type_ptr.hpp"
#include "glm/gtx/transform2.hpp"
#include "glm/gtx/quaternion.hpp"

namespace glm
{

 template <typename T> 
 inline glm::detail::tmat4x4<T> lookAt2(
		const glm::detail::tvec3<T>& eye, 
		const glm::detail::tvec3<T>& center, 
		const glm::detail::tvec3<T>& up)
    {
        glm::detail::tvec3<T> f = glm::normalize(center - eye);
        glm::detail::tvec3<T> u = glm::normalize(up);
        glm::detail::tvec3<T> s = glm::normalize(cross(f, u));
        u = glm::cross(s, f);

        glm::detail::tmat4x4<T> Result(1);
        Result[0][0] = s.x;
        Result[0][1] = s.y;
        Result[0][2] = s.z;
        Result[1][0] = u.x;
        Result[1][1] = u.y;
        Result[1][2] = u.z;
        Result[2][0] =-f.x;
        Result[2][1] =-f.y;
        Result[2][2] =-f.z;
		Result[3][0] =eye.x;
        Result[3][1] =eye.y;
        Result[3][2] =eye.z;

		return Result;
    }
}


#endif