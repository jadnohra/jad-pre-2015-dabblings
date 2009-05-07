using System;
using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Graphics;

namespace Framework1
{
    public class BiQuadBezierPatch<T>
    {
        public T[] ControlPoints = new T[9];
    }

    public class Tesselation<T>
    {
        public T[] Vertices;
    }

   
    public class BezierPatchTesselator<T> where T : IVertex
    {
        T[] tempVertices = new T[3];

        public BezierPatchTesselator()
        {
            //for (int i = 0; i < 3; ++i)
              //  tempVertices[i] = new T();
        }

        T Combine(T p1, float w1, T p2, float w2, T p3, float w3)
        {
            return (T)(p1.Mul(w1).Add((T)p2.Mul(w2)).Add((T)p3.Mul(w3)));
        }

        static public int GetPathVertexCount(int subdivisionLevel)
        {
            // The number of vertices along a side is 1 + num edges
            int sideVertexCount = subdivisionLevel + 1;

            return sideVertexCount * sideVertexCount;
        }

        public void Tesselate(int subdivisionLevel, BiQuadBezierPatch<T> patch, ref Tesselation<T> tesselation)
        {
            int level = subdivisionLevel;

            // The number of vertices along a side is 1 + num edges
            int sideVertexCount = level + 1;
            int totalVertexCount = GetPathVertexCount(subdivisionLevel);

            if (tesselation.Vertices == null || tesselation.Vertices.Length != totalVertexCount)
            {
                tesselation.Vertices = new T[totalVertexCount];
            }

            // Compute the vertices
            {
                for (int i = 0; i <= level; ++i) {

                    float a = (float)i / (float)level;
                    float b = 1.0f - a;

                    //tesselation.Vertices[i] = 
                      //  patch.ControlPoints[0] * (b * b) + 
                        //patch.ControlPoints[3] * (2.0f * b * a) +
                        //patch.ControlPoints[6] * (a * a);
                    tesselation.Vertices[i] = Combine(patch.ControlPoints[0], b * b,
                                                      patch.ControlPoints[3], 2.0f * b * a,
                                                      patch.ControlPoints[6], a * a); 
                }

                for (int i = 1; i <= level; ++i) {

                    float a = (float)i / (float)level;
                    float b = 1.0f - a;

                    int j;
                    for (j = 0; j < 3; ++j) {

                        int k = 3 * j;
                        
                        //tempVertices[j] =
                        //  patch.ControlPoints[k + 0] * (b * b) +
                        //  patch.ControlPoints[k + 1] * (2.0f * b * a) +
                        //  patch.ControlPoints[k + 2] * (a * a);

                        tempVertices[j] = Combine(patch.ControlPoints[k + 0], b * b,
                                                  patch.ControlPoints[k + 1], 2.0f * b * a,
                                                  patch.ControlPoints[k + 2], a * a); 
                    }

                    for(j = 0; j <= level; ++j) {

                        float a2 = (float)j / level;
                        float b2 = 1.0f - a2;

                        //tesselation.Vertices[i * sideVertexCount + j] =
                        //    tempVertices[0] * (b2 * b2) +
                        //    tempVertices[1] * (2.0f * b2 * a2) +
                        //    tempVertices[2] * (a2 * a2);

                        tesselation.Vertices[i * sideVertexCount + j] =
                                            Combine(tempVertices[0], b2 * b2,
                                                    tempVertices[1], 2.0f * b2 * a2,
                                                    tempVertices[2], a2 * a2); 
                    }
                }
            }
        }
    }
}