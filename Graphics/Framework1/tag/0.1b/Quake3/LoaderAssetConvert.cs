using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Diagnostics;
using Microsoft.Xna.Framework.Graphics;
using Microsoft.Xna.Framework;

namespace Framework1.Quake3
{
    using BspFile;

    class AssetConvert
    {
        public static void ConvertFace(ref short i0, ref short i1, ref short i2)
        {
            //short i = i1;
            //i1 = i2;
            //i2 = i;
        }

        public static void ConvertVector(ref Vector3 vector)
        {
            float temp = vector.Z;

            vector.Z = vector.Y;
            vector.Y = -temp;
        }

        public static void ConvertTexCoord(ref Vector2 vector)
        {
        }

        public static void Convert(Vertices.Binary_vertex[] source, VertexPositionColor[] dest)
        {
            for (int i = 0; i < source.Length; ++i)
            {
                dest[i].Position.X = source[i].position_x;
                dest[i].Position.Y = source[i].position_y;
                dest[i].Position.Z = source[i].position_z;

                ConvertVector(ref dest[i].Position);

                dest[i].Color.R = source[i].color_r;
                dest[i].Color.G = source[i].color_g;
                dest[i].Color.B = source[i].color_b;
                dest[i].Color.A = source[i].color_a;
            }
        }

        public static void Convert(Vertices.Binary_vertex[] source, VertexPositionColorTexture[] dest)
        {
            for (int i = 0; i < source.Length; ++i)
            {
                dest[i].Position.X = source[i].position_x;
                dest[i].Position.Y = source[i].position_y;
                dest[i].Position.Z = source[i].position_z;

                ConvertVector(ref dest[i].Position);

                dest[i].Color.R = source[i].color_r;
                dest[i].Color.G = source[i].color_g;
                dest[i].Color.B = source[i].color_b;
                dest[i].Color.A = source[i].color_a;

                dest[i].TextureCoordinate.X = source[i].texcoord_s;
                dest[i].TextureCoordinate.Y = source[i].texcoord_t;

                ConvertTexCoord(ref dest[i].TextureCoordinate);
            }
        }


        public static void Convert(Header header, Faces.Binary_face[] source, out short[] dest)
        {
            uint indexCount = 0;
            int minMeshVert = System.Int32.MaxValue;
            int maxMeshVert = System.Int32.MinValue;

            for (int i = 0; i < source.Length; ++i)
            {
                if (source[i].type == (int)FaceType.Mesh || source[i].type == (int)FaceType.Polygon)
                {

                    indexCount += (uint)source[i].n_meshverts;

                    if (source[i].meshvert < minMeshVert)
                        minMeshVert = source[i].meshvert;

                    if (source[i].meshvert + source[i].n_meshverts > maxMeshVert)
                        maxMeshVert = source[i].meshvert + source[i].n_meshverts;
                }
            }

            dest = null;

            if (maxMeshVert > minMeshVert)
            {
                using (MeshVertices meshVerts =
                        header.Loader.GetMeshVertices(header, minMeshVert, (int)(maxMeshVert - minMeshVert)))
                {
                    dest = new short[indexCount];

                    uint destIndex = 0;

                    for (int i = 0; i < source.Length; ++i)
                    {
                        if (source[i].type == (int)FaceType.Mesh || source[i].type == (int)FaceType.Polygon)
                        {
                            int meshVertIndex = source[i].meshvert - minMeshVert;

                            Trace.Assert(source[i].n_meshverts % 3 == 0);

                            int faceVertexIndex = 0;
                            for (int j = 0; j < source[i].n_meshverts; ++j, ++meshVertIndex)
                            {
                                int index = (source[i].vertex + (meshVerts.m_Vertices[meshVertIndex].offset));
                                Trace.Assert(index == (int)((short)index));
                                dest[destIndex++] = (short)index;

                                if (faceVertexIndex == 2)
                                {
                                    faceVertexIndex = 0;
                                    ConvertFace(ref dest[destIndex - 3], ref dest[destIndex - 2], ref dest[destIndex - 1]);
                                }
                                else
                                {
                                    ++faceVertexIndex;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

/*
class AssetInterpreter_LoaderVertices_CPUPositionColorStream : AssetInterpreter
{
    public Type GetAssetType()
    {
        return typeof(LoaderVerticesAsset);
    }

    public Type GetTargetType()
    {
        return typeof(CPUPositionColorStream);
    }

    static public override Object Interpret(Asset asset, AssetPart part)
    {
        Header header = ((LoaderVerticesAsset)asset).header;
        Loader loader = header.Loader; 
        CPUPositionColorStream result = null;

        int vertexCount = loader.GetVertexCount(header);

        if (vertexCount > 0)
        {
            if (part == null)
            {
                // We can use a buffer here to limit memory usage

                using (Vertices vertices = loader.GetVertices(header, 0, vertexCount))
                {

                    result = new CPUPositionColorStream();
                    result.Vertices = new VertexPositionColor[vertexCount];

                    Quake3LoaderAssetConvert.Convert(vertices.m_Vertices, result.Vertices);
                }
            }
            else
            {
            }
        }

        return result;
    }
}

class AssetInterpreter_LoaderVertices_CPUPositionColorTextureStream : AssetInterpreter
{
    public Type GetAssetType()
    {
        return typeof(LoaderVerticesAsset);
    }

    public Type GetTargetType()
    {
        return typeof(CPUPositionColorStream);
    }

    public override Object Interpret(Asset asset, AssetPart part)
    {
        Header header = ((LoaderVerticesAsset)asset).header;
        Loader loader = header.Loader;
        CPUPositionColorTextureStream result = null;

        int vertexCount = loader.GetVertexCount(header);

        if (vertexCount > 0)
        {
            if (part == null)
            {
                // We can use a buffer here to limit memory usage

                using (Vertices vertices = loader.GetVertices(header, 0, vertexCount))
                {

                    result = new CPUPositionColorTextureStream();
                    result.Vertices = new VertexPositionColorTexture[vertexCount];

                    Quake3LoaderAssetConvert.Convert(vertices.m_Vertices, result.Vertices);
                }
            }
            else
            {
            }
        }

        return result;
    }
}

class AssetInterpreter_LoaderFaces_CPUIndex16Stream : AssetInterpreter
{
    public Type GetAssetType()
    {
        return typeof(LoaderFacesAsset);
    }

    public Type GetTargetType()
    {
        return typeof(CPUIndex16Stream);
    }

    static public override Object Interpret(Asset asset, AssetPart part)
    {
        Header header = ((LoaderFacesAsset)asset).header;
        Loader loader = header.Loader;
        CPUIndex16Stream result = null;

        int faceCount = loader.GetFaceCount(header);

        if (faceCount > 0)
        {
            if (part == null)
            {
                // We can use a buffer here to limit memory usage

                using (Faces faces = loader.GetFaces(header, 0, faceCount))
                {
                    result = new CPUIndex16Stream();

                    Quake3LoaderAssetConvert.Convert(header, faces.m_Faces, out result.Indices);
                }
            }
            else
            {
            }
        }

        return result;
    }
}
 */
