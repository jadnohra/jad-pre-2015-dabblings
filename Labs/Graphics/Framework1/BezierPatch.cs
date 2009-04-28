using System;
using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Graphics;

namespace Framework1
{
    public class BiQuadBezierPatch
    {
        public Vector3[] ControlPoints;
        public int Width;
        public int Height;
    }

    public class Tesselation
    {
        public Vector3[] Vertices;
        public Int16[] Indices;
        public PrimitiveType PrimType;
    }

    public class BezierPatchTesselator
    {
        Vector3[] tempVertices = new Vector3[3];

        void Tesselate(int subdivisionLevel, BiQuadBezierPatch patch, ref Tesselation tesselation)
        {
            int level = subdivisionLevel;

            // The number of vertices along a side is 1 + num edges
            int sideVertexCount = level + 1;

            tesselation.Vertices = new Vector3[sideVertexCount * sideVertexCount];

            // Compute the vertices
            {
                for (int i = 0; i <= level; ++i) {

                    float a = (float)i / (float)level;
                    float b = 1.0f - a;

                    tesselation.Vertices[i] = 
                        patch.ControlPoints[0] * (b * b) + 
                        patch.ControlPoints[3] * (2.0f * b * a) +
                        patch.ControlPoints[6] * (a * a);
                }

                for (int i = 1; i <= level; ++i) {

                    float a = (float)i / (float)level;
                    float b = 1.0f - a;

                    int j;
                    for (j = 0; j < 3; ++j) {

                        int k = 3 * j;
                        tempVertices[j] =
                            patch.ControlPoints[k + 0] * (b * b) +
                            patch.ControlPoints[k + 1] * (2.0f * b * a) +
                            patch.ControlPoints[k + 2] * (a * a);
                    }

                    for(j = 0; j <= level; ++j) {

                        float a2 = (float)j / level;
                        float b2 = 1.0f - a;

                        tesselation.Vertices[i * sideVertexCount + j] =
                            tempVertices[0] * (b2 * b2) +
                            tempVertices[1] * (2.0f * b2 * a2) +
                            tempVertices[2] * (a2 * a2);
                    }
                }
            }


            // Compute the indices
            {
                /*
                int row;
                tesselation.Indices = new Int16[level * (sideVertexCount) * 2];
                tesselation.PrimType = PrimitiveType.TriangleStrip;

                for (row = 0; row < level; ++row) {

                    for(int col = 0; col <= level; ++col)	{

                        tesselation.Indices[(row * (level + 1) + col) * 2 + 1] = (Int16)(row * sideVertexCount + col);
                        tesselation.Indices[(row * (level + 1) + col) * 2] = (Int16)((row + 1) * sideVertexCount + col);
                    }
                }
                */

                /*
                trianglesPerRow.resize(level);
                rowIndexes.resize(level);
                for (row = 0; row < level; ++row) {
                    trianglesPerRow[row] = 2 * sideVertexCount;
                    rowIndexes[row]      = &indexes[row * 2 * sideVertexCount];
                }
                */ 
            }
        }
    }
}