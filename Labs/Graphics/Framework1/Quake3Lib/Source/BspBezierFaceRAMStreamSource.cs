using System;
using System.Diagnostics;
using System.Reflection;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Graphics;
using BlackRice.Framework.Graphics;
using BlackRice.Framework.Quake3.Load;

namespace BlackRice.Framework.Quake3
{
    // ----------------------------------------------------------------------------
    // Can load a bezier face's vertices and triangle indices
    // ----------------------------------------------------------------------------
    public class BspBezierFaceRAMStreamSource : RenderResourceManager.RAMStreamSource
    {
        internal LoadedBspLevel m_Level;
        internal int m_Face;
        int m_TesselationLevel = 5;

        public BspBezierFaceRAMStreamSource(LoadedBspLevel bspLevel, int face)
        {
            m_Level = bspLevel;
            m_Face = face;
        }

        public override RenderResourceManager.IRAMVertexStreamReader GetVertexReader(RenderResourceManager.VertexSemantics semantics, Type type)
        {
            return new VertexReader(this, semantics, type);
        }

        public override RenderResourceManager.IRAMIndexStreamReader GetIndexReader(Type type)
        {
            return new IndexReader(this, type);
        }


        public class ReaderBase
        {
            // Optimization: Instead of a layout, a mapping can be initialized and checked 
            // in parent and then used here.
            internal BspBezierFaceRAMStreamSource m_Parent;

            public ReaderBase(BspBezierFaceRAMStreamSource parent, Type type)
            {
                m_Parent = parent;
            }
        }

        public class IndexReader
        : ReaderBase
        , RenderResourceManager.IRAMIndexStreamReader
        {
            int m_IndexCount;

            public IndexReader(BspBezierFaceRAMStreamSource parent, Type type)
                : base(parent, type)
            {
                 BspFile.Header header = m_Parent.m_Level.Header;

                 using (BspFile.Faces faces = header.Loader.GetFaces(header, m_Parent.m_Face, 1))
                 {
                     BspFile.Faces.Binary_face face = faces.m_Faces[0];

                     int patchCountX = (face.size_x - 1) / 2;
                     int patchCountY = (face.size_y - 1) / 2;
                     int sideVertexCount = m_Parent.m_TesselationLevel+1;

                     m_IndexCount = Geometry.UniformGridTesselator.GetTriangleStripSize(patchCountX, patchCountY, sideVertexCount, sideVertexCount);
                 }
            }

            public int Count()
            {
                return m_IndexCount;
            }

            public void Read<T>(ref RenderResourceManager.RAMStream<T> array)
            {
                 BspFile.Header header = m_Parent.m_Level.Header;

                 using (BspFile.Faces faces = header.Loader.GetFaces(header, m_Parent.m_Face, 1))
                 {
                     BspFile.Faces.Binary_face face = faces.m_Faces[0];

                     int patchCountX = (face.size_x - 1) / 2;
                     int patchCountY = (face.size_y - 1) / 2;
                     int sideVertexCount = m_Parent.m_TesselationLevel + 1;

                     Geometry.UniformGridTesselator.GenerateTriangleStrip(patchCountX, patchCountY, sideVertexCount, sideVertexCount, ref array.Data, array.Offset, false);
                 }
            }
        }

        public class VertexReader 
        : ReaderBase
        , RenderResourceManager.IRAMVertexStreamReader
        {
            int m_VertexCount;

            public VertexReader(BspBezierFaceRAMStreamSource parent, RenderResourceManager.VertexSemantics semantics, Type type)
            : base(parent, type)
            {
                BspFile.Header header = m_Parent.m_Level.Header;

                using (BspFile.Faces faces = header.Loader.GetFaces(header, m_Parent.m_Face, 1))
                {
                    BspFile.Faces.Binary_face face = faces.m_Faces[0];

                    Trace.Assert(face.type == (int)BspFile.FaceType.Patch);
                    {
                        int patchCountX = (face.size_x - 1) / 2;
                        int patchCountY = (face.size_y - 1) / 2;
                        int patchCount = patchCountX * patchCountY;
                        m_VertexCount = patchCount * BezierPatchTesselator<IVertex>.GetPathVertexCount(m_Parent.m_TesselationLevel);
                    }
                }
            }

            public int Count()
            {
                return m_VertexCount;
            }

            public void Read<T>(ref RenderResourceManager.RAMStream<T> array, RenderResourceManager.VertexSemantics semantics)
            where T : IVertex
            {
                BspFile.Header header = m_Parent.m_Level.Header;

                using (BspFile.Faces faces = header.Loader.GetFaces(header, m_Parent.m_Face, 1))
                {
                    BspFile.Faces.Binary_face face = faces.m_Faces[0];

                    using (BspFile.Vertices loadedVertices = header.Loader.GetVertices(header, face.vertex, face.n_vertexes))
                    {
                        BspVertexLoader vertexLoader = new BspVertexLoader(typeof(T).GetFields(), semantics.Layout, m_Parent.m_Level.CoordSysConv);

                        T boxedHolder = Activator.CreateInstance<T>();
                        object boxed = (object)boxedHolder;

                        // We do not have to stitch the patches together
                        {
                            Trace.Assert(face.n_vertexes != 0);

                            int patchCountX = (face.size_x - 1) / 2;
                            int patchCountY = (face.size_y - 1) / 2;
                            int patchCount = patchCountX * patchCountY;

                            Trace.Assert((patchCountX * 2) + 1 == face.size_x);
                            Trace.Assert((patchCountY * 2) + 1 == face.size_y);

                            BiQuadBezierPatch<T> patch = new BiQuadBezierPatch<T>();
                            BezierPatchTesselator<T> tesselator = new BezierPatchTesselator<T>();
                            Tesselation<T> tesselation = new Tesselation<T>();

                            int patchIndexStride = face.size_x;
                            int sideVertexCount = m_Parent.m_TesselationLevel + 1;
                            int tesselpathIndexStride = patchCountX * sideVertexCount;

                            for (int patchIndexX = 0; patchIndexX < patchCountX; patchIndexX += 1)
                            {
                                for (int patchIndexY = 0; patchIndexY < patchCountY; patchIndexY += 1)
                                {
                                    int patchIndexOffset = (2 * patchIndexX) + (patchIndexY * patchIndexStride);

                                    int controlPointIndex = 0;
                                    for (int controlY = 0; controlY < 3; ++controlY)
                                    {
                                        int controlYOffset = controlY * 3;
                                        int controlIndexOffset = patchIndexOffset + (controlY * patchIndexStride);

                                        for (int controlX = 0; controlX < 3; ++controlX)
                                        {
                                            vertexLoader.Read(loadedVertices.m_Vertices[controlIndexOffset + controlX], ref boxed);
                                            patch.ControlPoints[controlX + controlYOffset] = (T)boxed;
                                            ++controlPointIndex;
                                        }
                                    }
                                    Trace.Assert(controlPointIndex == 9);

                                    tesselator.Tesselate(m_Parent.m_TesselationLevel, patch, ref tesselation);
                                    int tesselPatchIndexOffset = (sideVertexCount * patchIndexX) + (patchIndexY * tesselpathIndexStride);
                                    int tesselationVertexIndex = 0;

                                    for (int tesselY = 0; tesselY < sideVertexCount; ++tesselY)
                                    {
                                        int tessetlYOffset = tesselY * sideVertexCount;
                                        int tesselIndexOffset = array.Offset + (tesselPatchIndexOffset + (tesselY * tesselpathIndexStride));

                                        for (int tesselX = 0; tesselX < sideVertexCount; ++tesselX)
                                        {
                                            array.Data[tesselX + tesselIndexOffset] = tesselation.Vertices[tesselationVertexIndex++];
                                        }
                                    }

                                    Trace.Assert(tesselationVertexIndex == tesselation.Vertices.Length);
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    public class SimpleBspBezierFaceRAMStreamSource : RenderResourceManager.RAMStreamSource
    {
        internal LoadedBspLevel m_Level;
        internal int m_Face;

        public SimpleBspBezierFaceRAMStreamSource(LoadedBspLevel bspLevel, int face)
        {
            m_Level = bspLevel;
            m_Face = face;
        }

        public override RenderResourceManager.IRAMVertexStreamReader GetVertexReader(RenderResourceManager.VertexSemantics semantics, Type type)
        {
            return new VertexReader(this, semantics, type);
        }

        public override RenderResourceManager.IRAMIndexStreamReader GetIndexReader(Type type)
        {
            return new IndexReader(this, type);
        }


        public class ReaderBase
        {
            // Optimization: Instead of a layout, a mapping can be initialized and checked 
            // in parent and then used here.
            internal SimpleBspBezierFaceRAMStreamSource m_Parent;

            public ReaderBase(SimpleBspBezierFaceRAMStreamSource parent, Type type)
            {
                m_Parent = parent;
            }
        }

        public class IndexReader
        : ReaderBase
        , RenderResourceManager.IRAMIndexStreamReader
        {
            int m_IndexCount;

            public IndexReader(SimpleBspBezierFaceRAMStreamSource parent, Type type)
                : base(parent, type)
            {
                BspFile.Header header = m_Parent.m_Level.Header;

                using (BspFile.Faces faces = header.Loader.GetFaces(header, m_Parent.m_Face, 1))
                {
                    BspFile.Faces.Binary_face face = faces.m_Faces[0];
                    m_IndexCount = Geometry.UniformGridTesselator.GetTriangleStripSize(face.size_x, face.size_y);
                }
            }

            public int Count()
            {
                return m_IndexCount;
            }

            public void Read<T>(ref RenderResourceManager.RAMStream<T> array)
            {
                BspFile.Header header = m_Parent.m_Level.Header;

                using (BspFile.Faces faces = header.Loader.GetFaces(header, m_Parent.m_Face, 1))
                {
                    BspFile.Faces.Binary_face face = faces.m_Faces[0];
                    Geometry.UniformGridTesselator.GenerateTriangleStrip(face.size_x, face.size_y, ref array.Data, array.Offset, true);
                }
            }
        }

        public class VertexReader
        : ReaderBase
        , RenderResourceManager.IRAMVertexStreamReader
        {
            int m_VertexCount;

            public VertexReader(SimpleBspBezierFaceRAMStreamSource parent, RenderResourceManager.VertexSemantics semantics, Type type)
                : base(parent, type)
            {
                BspFile.Header header = m_Parent.m_Level.Header;

                using (BspFile.Faces faces = header.Loader.GetFaces(header, m_Parent.m_Face, 1))
                {
                    BspFile.Faces.Binary_face face = faces.m_Faces[0];

                    Trace.Assert(face.type == (int)BspFile.FaceType.Patch);
                    {
                        m_VertexCount = face.n_vertexes;
                    }
                }
            }

            public int Count()
            {
                return m_VertexCount;
            }

            public void Read<T>(ref RenderResourceManager.RAMStream<T> array, RenderResourceManager.VertexSemantics semantics)
            where T : IVertex
            {
                BspFile.Header header = m_Parent.m_Level.Header;

                using (BspFile.Faces faces = header.Loader.GetFaces(header, m_Parent.m_Face, 1))
                {
                    BspFile.Faces.Binary_face face = faces.m_Faces[0];

                    using (BspFile.Vertices loadedVertices = header.Loader.GetVertices(header, face.vertex, face.n_vertexes))
                    {
                        BspVertexLoader vertexLoader = new BspVertexLoader(typeof(T).GetFields(), semantics.Layout, m_Parent.m_Level.CoordSysConv);

                        T boxedHolder = Activator.CreateInstance<T>();
                        object boxed = (object)boxedHolder;

                        int i = array.Offset;
                        for (int j = 0; j < loadedVertices.m_Vertices.Length; ++i, ++j)
                        {
                            BspFile.Vertices.Binary_vertex vertex = loadedVertices.m_Vertices[j];

                            vertexLoader.Read(loadedVertices.m_Vertices[j], ref boxed);
                            array.Data[i] = (T)boxed;
                        }
                    }
                }
            }
        }
    }
}