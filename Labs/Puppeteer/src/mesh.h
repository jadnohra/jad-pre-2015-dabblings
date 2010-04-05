#ifndef _MESH_H
#define _MESH_H

#include "glm/glm.hpp"
#include <vector>

#pragma warning( push )
#pragma warning( disable : 4091 )
#include "wavefrontObjLoader/objLoader.h"
#pragma warning( pop ) 


class Mesh
{
public:

	typedef std::vector<glm::vec3> Positions;
	typedef std::vector<glm::vec3> Normals;
	typedef std::vector<unsigned __int16> Indices;

	Positions mPositions;
	Normals mNormals;
	Indices mIndices;

	void Draw()
	{
		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(3, GL_FLOAT, 0, &(*mPositions.begin()));
		glDrawElements(GL_TRIANGLES,mIndices.size()/3,GL_UNSIGNED_SHORT,&(*mIndices.begin()));
		glDisableClientState(GL_VERTEX_ARRAY);
	}

	void LoadWavefrontObj(const char* filePath, /*Camera* pCamera, */bool autoPlaceCamIfNeeded, bool autoPlaceLightIfNeeded, bool autoGenerateAmbientIfNeeded) {

		mPositions.clear();
		mNormals.clear();
		mIndices.clear();

		struct wfo_load {
		
			static inline glm::vec3 getPos(objLoader& objData, int index) {

				obj_vector& vect = *(objData.vertexList[index]);
				return glm::vec3(vect.e[0], vect.e[1], vect.e[2]);
			}

			static inline glm::vec3 getNormal(objLoader& objData, int index) {

				obj_vector& vect = *(objData.normalList[index]);
				return glm::vec3(vect.e[0], vect.e[1], vect.e[2]);
			}

			/*
			static void loadMaterial(objLoader& objData, int matIndex, Material& material, bool* pHasAmbient = NULL, Vector3* pAmbient = NULL) {

				if (matIndex >= 0) {

					obj_material& mat = dref(objData.materialList[matIndex]);

					material.reflection() = (float) mat.reflect;
					material.refraction() = (float) mat.refract;
					material.refractionIndex() = (float) mat.refract_index;
					material.diffuse() = 1.0f;
					material.specularPower() = (float) mat.glossy;
					material.specular() = (mat.spec[0] != 0.0f || mat.spec[1] != 0.0f || mat.spec[2] != 0.0f) ? 1.0f : 0.0f;
					material.color().set((float) mat.diff[0], (float) mat.diff[1], (float) mat.diff[2]);

					material.color().capMax(1.0f);

					if (pHasAmbient) {

						*pHasAmbient = true;
						pAmbient->el[0] += (float) mat.amb[0];
						pAmbient->el[1] += (float) mat.amb[1];
						pAmbient->el[2] += (float) mat.amb[2];
					}

				} else {

					material.reflection() = 0.0f;
					material.diffuse() = 1.0f;
					material.specular() = 0.7f;
					material.color().set(1.0f, 1.0f, 1.0f);

					if (pHasAmbient) {

						*pHasAmbient = true;
						pAmbient->el[0] += 0.2f;
						pAmbient->el[1] += 0.2f;
						pAmbient->el[2] += 0.2f;
					}
				}
			}
			*/

			/*
			static void onLightMaterial(objLoader& objData, int matIndex, Material& material, bool* pHasAmbient = NULL, Vector3* pAmbient = NULL) {

				if (matIndex >= 0) {

					obj_material& mat = dref(objData.materialList[matIndex]);

					if (pHasAmbient) {

						*pHasAmbient = true;
						pAmbient->el[0] += (float) mat.amb[0];
						pAmbient->el[1] += (float) mat.amb[1];
						pAmbient->el[2] += (float) mat.amb[2];
					}

				} else {

					if (pHasAmbient) {

						*pHasAmbient = true;
						pAmbient->el[0] += 0.2f;
						pAmbient->el[1] += 0.2f;
						pAmbient->el[2] += 0.2f;
					}
				}
			}
			*/
		};

		objLoader objData;
		if (!objData.load(const_cast<char*>(filePath)))
			return;

		/*
		Primitives& primitives = mPrimitives;
		Primitives& lights = mLights;
		Materials& materials = mMaterials;
		FastUnitCoordSys& coordSys = mCoordSys;
		
		static FileCoordSys fileCoordSys;
		CoordSysConv_Swizzle sysConv;
		sysConv.init(fileCoordSys, mCoordSysBase);


		{
			if (pCamera && objData.camera) {

				SceneTransform& transf = pCamera->worldTransform();

				transf.identity();
				transf.setPosition(getPos(sysConv, objData, objData.camera->camera_pos_index));
				transf.setOrientation(transf.getPosition(), 
										getPos(sysConv, objData, objData.camera->camera_look_point_index),
										getNormal(sysConv, objData, objData.camera->camera_up_norm_index));

				if (objData.camera->fov > 0.0f)
					pCamera->initProjection(dref(this), pCamera->getNearZ(), pCamera->getFarZ(), objData.camera->fov);

				pCamera->markDirty();
			}
		}

		{
			for (int i = 0; i < objData.materialCount; ++i) {

				SoftPtr<Material> material = (materials.addOne() = new Material());

				loadMaterial(objData, i, material);
			}
		}
		*/

		/*
		{
			for (int i = 0; i < objData.sphereCount; ++i) {

				obj_sphere& sphere = dref(objData.sphereList[i]);

				SoftPtr<Primitive_Sphere> prim = new Primitive_Sphere(getPos(sysConv, objData, sphere.pos_index), getNormal(sysConv, objData, sphere.up_normal_index).mag());
				primitives.addOne() = prim;
			
				prim->setMaterial(sphere.material_index >= 0 ? mMaterials[sphere.material_index] : &mDefaultMaterial);

				{
					Sphere sphere(prim->position(), prim->getRadius());
					
					mSceneAAB.add(sphere);
				}
			}
		}
		*/

		{
			mPositions.resize(objData.vertexCount);
			
			for (int i = 0; i < objData.vertexCount; ++i) {

				mPositions[i] = wfo_load::getPos(objData, i);
			}

			unsigned int faceCount = 0;

			for (int i = 0; i < objData.faceCount; ++i) {

				obj_face& face = *(objData.faceList[i]);

				if (face.vertex_count == 3) {

					++faceCount;
				}
			}

			mIndices.resize(faceCount*3);
			//mNormals.resize(faceCount);

			for (int i = 0, index_index=0,normal_index=0; i < objData.faceCount; ++i) {

				obj_face& face = *(objData.faceList[i]);

				if (face.vertex_count == 3) {
					
					mIndices[index_index++] = 0 + face.vertex_index[0];
					mIndices[index_index++] = 0 + face.vertex_index[1];
					mIndices[index_index++] = 0 + face.vertex_index[2];

					//if (face.normal_index[0] >= 0)
					//	mNormals[normal_index++] = getNormal(objData, face.normal_index[0]);
					//else
					//	tri.autoGenNormal(mVertices.el, fileCoordSys.rotationLeftHanded == 1);
				}
			}
		}

		/*
		{
			for (int i = 0; i < objData.faceCount; ++i) {

				obj_face& face = dref(objData.faceList[i]);

				if (face.vertex_count == 3) {

					
					SoftPtr<Primitive_Triangle> prim = new Primitive_Triangle(getPos(sysConv, objData, face.vertex_index[0]), 
																							getPos(sysConv, objData, face.vertex_index[1]), 
																							getPos(sysConv, objData, face.vertex_index[2]), 
																							face.normal_index[0] >= 0 ? &getNormal(sysConv, objData, face.normal_index[0]) : NULL, fileCoordSys.rotationLeftHanded == 1);
					primitives.addOne() = prim;

					prim->setMaterial(face.material_index >= 0 ? mMaterials[face.material_index] : &mDefaultMaterial);

					{
						mSceneAAB.add(prim->V(0));
						mSceneAAB.add(prim->V(1));
						mSceneAAB.add(prim->V(2));
					}
				}
			}
		}
		*/

		{
			/*
			for (int i = 0; i < objData.lightPointCount; ++i) {

				obj_light_point& light = dref(objData.lightPointList[i]);

				SoftPtr<Primitive> prim = new Primitive_Sphere(getPos(sysConv, objData, light.pos_index), 0.0f);
				lights.addOne() = prim;
				
				prim->setMaterial(light.material_index >= 0 ? mMaterials[light.material_index] : &mDefaultMaterial);
				onLightMaterial(objData, light.material_index, prim->material(), &mHasAmbient, &mAmbient);
			}
			*/

			/*
			for (int i = 0; i < objData.lightDiscCount; ++i) {

				obj_light_disc& light = dref(objData.lightDiscList[i]);

				SoftPtr<Primitive> prim = new Primitive_Sphere(getPos(sysConv, objData, light.pos_index), 0.1f);
				lights.addOne() = prim;
				
				prim->isLight() = true;
				prim->material().diffuse() = 1.0f;
				prim->material().specular() = 1.0f;
				prim->material().color().set(0.0f, 0.8f, 0.0f, &mHasAmbient, &mAmbient);
			}
			*/

			/*
			for (int i = 0; i < objData.lightQuadCount; ++i) {

				obj_light_quad& light = dref(objData.lightQuadList[i]);

				SoftPtr<Primitive> prim = new Primitive_Sphere(getPos(sysConv, objData, light.vertex_index[0]), 0.1f);
				lights.addOne() = prim;
				
				prim->isLight() = true;
				prim->material().diffuse() = 1.0f;
				prim->material().specular() = 1.0f;
				prim->material().color().set(0.0f, 0.0f, 0.9f, &mHasAmbient, &mAmbient);
			}
			*/
		}

		/*
		{
			if (pCamera && !objData.camera && autoPlaceCamIfNeeded && !mSceneAAB.isEmpty()) {

				SceneTransform& transf = pCamera->worldTransform();

				Vector3 camPos;
				Vector3 lookAtPos;
				Vector3 up;

				up.zero();
				up.el[Scene_Up] = 1.0f;

				sysConv.toTargetVector(up.el);

				mSceneAAB.center(lookAtPos);
				camPos = lookAtPos;
				
				camPos.el[Scene_Forward] = mSceneAAB.min.el[Scene_Forward];
				camPos.el[Scene_Forward] -= 1.4f * mSceneAAB.size(Scene_Right);
				//camPos.el[Scene_Forward] = -camPos.el[Scene_Forward];
				
				transf.identity();
				transf.setPosition(camPos);
				transf.setOrientation(transf.getPosition(), lookAtPos, up);

				pCamera->markDirty();
			}

			if (lights.count == 0 && autoGenerateAmbientIfNeeded) {

				if (autoPlaceLightIfNeeded && pCamera) {

					Vector3 lightPos = pCamera->worldTransform().getPosition();
					lightPos.el[Scene_Right] += 0.2f * mSceneAAB.size(Scene_Right);
					lightPos.el[Scene_Up] += 0.5f * mSceneAAB.size(Scene_Up);

					SoftPtr<Primitive> prim = (new Primitive_Sphere(lightPos, 0.0f));
					lights.addOne() = prim;
					
					prim->setMaterial(&mDefaultMaterial);
					onLightMaterial(objData, -1, prim->material(), &mHasAmbient, &mAmbient);
			
				} else if (autoGenerateAmbientIfNeeded) {

					mHasAmbient = true; 
					mAmbient.set(0.7f);
				}
			}
		}
		*/
	}

};

#endif