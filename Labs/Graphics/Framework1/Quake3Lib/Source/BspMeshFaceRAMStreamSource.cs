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
    // Can load a mesh face's vertices and triangle indices
    // ----------------------------------------------------------------------------
    public class BspMeshFaceRAMStreamSource : RenderResourceManager.RAMStreamSource
    {
        internal LoadedBspLevel m_Level;
        internal int m_Face;

        public BspMeshFaceRAMStreamSource(LoadedBspLevel bspLevel, int face)
        {
            m_Level = bspLevel;
            m_Face = face;
            // Note: We are obviously breaking the sharing of vertices between leafs using indices,
            // this has to do with the granularity of resource blocks, the level could be pre-processed 
            // for a specified optimal granularity
            // Performance: We are precomputing the VertexMapping here and storing it, this takes up memory, at the expense of the readers doing it,
            // it we are short on RAM and have processing power to burn, this might not be the best compromise, but it is easy to change!
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
            internal BspMeshFaceRAMStreamSource m_Parent;
            internal CoalescedIntervals m_CoalescedVertexIntervals;

            public ReaderBase(BspMeshFaceRAMStreamSource parent, Type type)
            {
                m_Parent = parent;
            }

            public void Init(ref int indexCount, ref int vertexCount)
            {
                m_CoalescedVertexIntervals = new CoalescedIntervals();

                using (IntervalCoalescor coalescor = new IntervalCoalescor())
                {
                    Build(coalescor, ref indexCount, ref vertexCount);
                    coalescor.BuildCoalescedIntervals(ref m_CoalescedVertexIntervals);
                }
            }

            void Build(IntervalCoalescor coalescor, ref int indexCount, ref int vertexCount)
            {
                indexCount = 0;
                BspFile.Header header = m_Parent.m_Level.Header;

                using (BspFile.Faces faces = header.Loader.GetFaces(header, m_Parent.m_Face, 1))
                {
                    BspFile.Faces.Binary_face face = faces.m_Faces[0];

                    if (face.type == (int)BspFile.FaceType.Mesh || face.type == (int)BspFile.FaceType.Polygon)
                    {
                        Trace.Assert(face.n_meshverts % 3 == 0);
                        indexCount = face.n_meshverts;

                        using (BspFile.MeshVertices triIndices = header.Loader.GetMeshVertices(header, face.meshvert, face.n_meshverts))
                        {
                            foreach (BspFile.MeshVertices.Binary_meshvert triIndex in triIndices.m_Vertices)
                            {
                                coalescor.AddElement(triIndex.offset + face.vertex);
                            }
                        }
                    }
                }

                vertexCount = coalescor.GetElementCount();
            }
        }

        public class IndexReader
        : ReaderBase
        , RenderResourceManager.IRAMIndexStreamReader
        {
            int m_VertexCount;
            int m_IndexCount;

            public IndexReader(BspMeshFaceRAMStreamSource parent, Type type)
                : base(parent, type)
            {
                base.Init(ref m_IndexCount, ref m_VertexCount);
            }

            public int Count()
            {
                return m_IndexCount;
            }

            public void Read<T>(ref RenderResourceManager.RAMStream<T> array)
            {
                Type type = typeof(T);
                BspFile.Header header = m_Parent.m_Level.Header;
                BspFile.CoordSysConv conv = m_Parent.m_Level.CoordSysConv;

                int i = array.Offset;
                using (BspFile.Faces faces = header.Loader.GetFaces(header, m_Parent.m_Face, 1))
                {
                    BspFile.Faces.Binary_face face = faces.m_Faces[0];

                    if (face.type == (int)BspFile.FaceType.Mesh || face.type == (int)BspFile.FaceType.Polygon)
                    {
                        using (BspFile.MeshVertices triIndices = header.Loader.GetMeshVertices(header, face.meshvert, face.n_meshverts))
                        {
                            int faceIndex = 0;
                            foreach (BspFile.MeshVertices.Binary_meshvert triIndex in triIndices.m_Vertices)
                            {
                                int mappedIndex = m_CoalescedVertexIntervals.Map(triIndex.offset + face.vertex);
                                Trace.Assert(mappedIndex >= 0 && mappedIndex < m_VertexCount);

                                array.Data[i++] = (T)Convert.ChangeType(mappedIndex, type);

                                if (faceIndex == 2)
                                {
                                    conv.ConvertFace<T>(ref array.Data[i - 3], ref array.Data[i - 2], ref array.Data[i - 1]);
                                    faceIndex = 0;
                                }
                                else
                                {
                                    ++faceIndex;
                                }
                            }
                        }
                    }
                }

                Trace.Assert(i == array.Offset + array.Count);
            }
        }

        public class VertexReader 
        : ReaderBase
        , RenderResourceManager.IRAMVertexStreamReader
        {
            int m_VertexCount;

            public VertexReader(BspMeshFaceRAMStreamSource parent, RenderResourceManager.VertexSemantics semantics, Type type)
            : base(parent, type)
            {
                int indexCount = 0;
                base.Init(ref indexCount, ref m_VertexCount);
            }

            public int Count()
            {
                return m_VertexCount;
            }

            public void Read<T>(ref RenderResourceManager.RAMStream<T> array, RenderResourceManager.VertexSemantics semantics)
            where T : IVertex
            {
                BspVertexLoader vertexLoader = new BspVertexLoader(typeof(T).GetFields(), semantics.Layout, m_Parent.m_Level.CoordSysConv);
                BspFile.Header header = m_Parent.m_Level.Header;

                T boxedHolder = Activator.CreateInstance<T>();
                object boxed = (object)boxedHolder;

                int i = array.Offset;
                foreach (Interval vInterval in m_CoalescedVertexIntervals.Intervals)
                {
                    using (BspFile.Vertices loadedVertices = header.Loader.GetVertices(header, vInterval.Start, vInterval.Count()))
                    {
                        for (int j = 0; j < loadedVertices.m_Vertices.Length; ++i, ++j)
                        {
                            BspFile.Vertices.Binary_vertex vertex = loadedVertices.m_Vertices[j];

                            vertexLoader.Read(loadedVertices.m_Vertices[j], ref boxed);
                            array.Data[i] = (T)boxed;
                        }
                    }
                }

                Trace.Assert(i == array.Offset + array.Count);
            }
        }
    }
}