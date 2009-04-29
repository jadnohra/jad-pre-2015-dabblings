using System;
using System.Diagnostics;
using System.Reflection;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Graphics;


namespace Framework1.Quake3
{
    // ----------------------------------------------------------------------------
    // Can load a bezier face's vertices and triangle indices
    // ----------------------------------------------------------------------------
    public class BspBezierFaceRAMStreamSource : RenderResourceManager.RAMStreamSource
    {
        internal LoadedBspLevel m_Level;
        internal int m_Face;

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
            public IndexReader(BspBezierFaceRAMStreamSource parent, Type type)
                : base(parent, type)
            {
            }

            public int Count()
            {
                return 0;
            }

            public void Read<T>(ref RenderResourceManager.RAMStream<T> array)
            {
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
                        m_VertexCount = face.n_vertexes;
                        /*
                        Trace.Assert(face.n_vertexes != 0);
                        
                        int patchCountX = (face.size_x - 1) / 2;
                        int patchCountY = (face.size_y - 1) / 2;
                        int patchCount = patchCountX * patchCountY;

                        Trace.Assert((patchCountX * 2) + 1 == face.size_x);
                        Trace.Assert((patchCountY * 2) + 1 == face.size_y);

                        BiQuadBezierPatch patch = new BiQuadBezierPatch();

                        // read the vertices (grid x * y  divided into 3*3 subgrids + sharing 1 line), set control points

                        for (int x = 0, x2 = 0; x < patchCountX; ++x, x2 += 2)
                        {
                            for (int y = 0, y2 = 0; y < patchCountY; ++y, y2 += 2)
                            {
                                for (int ctli = 0; ctli < 3; ++ctli)
                                {
                                    int ctlOffset = ctli * face.size_x;

                                    int vertexIndex0 = face.vertex + x2 + face.size_x * y2 + ctlOffset;
                                    int vertexIndex1 = vertexIndex0 + 1;
                                    int vertexIndex2 = vertexIndex0 + 2;
                                }
                            }
                        }
                        */ 
                    }
                }
            }

            public int Count()
            {
                return m_VertexCount;
            }

            public void Read<T>(ref RenderResourceManager.RAMStream<T> array, RenderResourceManager.VertexSemantics semantics)
            {
                BspFile.Header header = m_Parent.m_Level.Header;

                using (BspFile.Faces faces = header.Loader.GetFaces(header, m_Parent.m_Face, 1))
                {
                    BspFile.Faces.Binary_face face = faces.m_Faces[0];

                    using (BspFile.Vertices loadedVertices = header.Loader.GetVertices(header, face.n_vertexes, face.vertex))
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