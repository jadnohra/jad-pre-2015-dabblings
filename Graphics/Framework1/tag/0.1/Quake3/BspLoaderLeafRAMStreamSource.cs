using System;
using System.Diagnostics;
using System.Reflection;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Graphics;


namespace Framework1.Quake3
{

    public class BspLoaderVertexRAMStreamReader : RenderResourceManager.RAMStreamReader
    {
        // Optimization: Instead of a layout, a mapping can be initialized and checked 
        // in BspLoaderLeafRAMStreamSource and then used here.
        internal BspLoaderLeafRAMStreamSource m_Parent;
        internal RenderResourceManager.DataStructure m_Structure;
        internal Type m_Type;

        public BspLoaderVertexRAMStreamReader(BspLoaderLeafRAMStreamSource parent, RenderResourceManager.DataStructure structure, Type type)
        {
            m_Parent = parent;
            m_Structure = structure;
            m_Type = type;
        }

        public override int Count()
        {
            return m_Parent.m_IndexMapping.VertexCount;
        }

        public override void Read<T>(ref RenderResourceManager.RAMStream<T> array)
        {
            VertexElement[] layoutEls = m_Structure.Layout;
            FieldInfo[] typeFields = m_Type.GetFields();
            BspFile.Header header = m_Parent.m_BspTree.m_Level.Header;
            BspFile.CoordSysConv coordSysConv = m_Parent.m_BspTree.m_Level.CoordSysConv;

            Vector3 vector3 = new Vector3();
            Color color = new Color();

            T boxedHolder = Activator.CreateInstance<T>();
            object boxed = (object) boxedHolder;

            int i = array.Offset;
            foreach (Interval vInterval in m_Parent.m_IndexMapping.Intervals)
            {
                using (BspFile.Vertices loadedVertices = header.Loader.GetVertices(header, vInterval.Start, vInterval.Count()))
                {
                    for (int j = 0; j < loadedVertices.m_Vertices.Length; ++i, ++j)
                    {
                        BspFile.Vertices.Binary_vertex vertex = loadedVertices.m_Vertices[j];

                        for (int fieldIndex = 0; fieldIndex < layoutEls.Length; ++fieldIndex)
                        {
                            VertexElement vEl = layoutEls[fieldIndex];
                            FieldInfo fieldInfo = typeFields[fieldIndex];

                            switch (vEl.VertexElementUsage)
                            {
                                case VertexElementUsage.Position:
                                {
                                    // Trace.Assert(vEl.VertexElementFormat == VertexElementFormat.Vector3);
                                    // Trace.Assert(fieldInfo.FullName == "Microsoft.Xna.Framework.Vector3");

                                    vector3.X = vertex.position_x;
                                    vector3.Y = vertex.position_y;
                                    vector3.Z = vertex.position_z;
                                    coordSysConv.ConvertVector(ref vector3);

                                    fieldInfo.SetValue(boxed, vector3);
                                }
                                break;

                                case VertexElementUsage.Color:
                                {
                                    // Trace.Assert(vEl.VertexElementFormat == VertexElementFormat.Color);
                                    // Trace.Assert(fieldInfo.FullName == "Microsoft.Xna.Framework.Color");
                                    color.R = vertex.color_r;
                                    color.G = vertex.color_g;
                                    color.B = vertex.color_b;
                                    color.A = vertex.color_a;

                                    fieldInfo.SetValue(boxed, color);
                                }
                                break;

                                default:
                                {
                                    Trace.Assert(false, "Vertex Layout is incompatible");
                                }
                                break;
                            }
                        }

                        array.Data[i] = (T)boxed;
                    }
                }
            }

            Trace.Assert(i == array.Offset + array.Count);
        }
    }

    public class BspLoaderFaceIndexRAMStreamReader : RenderResourceManager.RAMStreamReader
    {
        // Optimization: Instead of a layout, a mapping can be initialized and checked 
        // in BspLoaderVertexRAMStreamSource and then used here.
        internal BspLoaderLeafRAMStreamSource m_Parent;
        internal Type m_Type;

        public BspLoaderFaceIndexRAMStreamReader(BspLoaderLeafRAMStreamSource parent, Type type)
        {
            m_Parent = parent;
            m_Type = type;
        }

        public override int Count()
        {
            return m_Parent.m_IndexMapping.IndexCount;
        }

        public override void Read<T>(ref RenderResourceManager.RAMStream<T> array)
        {
            BspFile.Header header = m_Parent.m_BspTree.m_Level.Header;
            BspFile.CoordSysConv coordSysConv = m_Parent.m_BspTree.m_Level.CoordSysConv;

            T boxedHolder = Activator.CreateInstance<T>();
            object boxed = (object)boxedHolder;

            int i = array.Offset;
            int globalMapOffset = m_Parent.m_IndexMapping.GetGlobalOffset();

            Interval[] leafFaceIntervals = m_Parent.m_BspTree.GetLeafFaceIntervals(m_Parent.m_Leaf);

            foreach (Interval leafFaceInterval in leafFaceIntervals)
            {
                using (BspFile.Faces faces = header.Loader.GetFaces(header, leafFaceInterval.Start, leafFaceInterval.Count()))
                {
                    foreach (BspFile.Faces.Binary_face face in faces.m_Faces)
                    {
                        if (face.type == (int)BspFile.FaceType.Mesh || face.type == (int)BspFile.FaceType.Polygon)
                        {
                            Trace.Assert(face.n_meshverts % 3 == 0);

                            using (BspFile.MeshVertices triIndices = header.Loader.GetMeshVertices(header, face.meshvert, face.n_meshverts))
                            {
                                int faceIndex = 0;
                                foreach (BspFile.MeshVertices.Binary_meshvert triIndex in triIndices.m_Vertices)
                                {
                                    int mappedIndex = globalMapOffset + m_Parent.m_IndexMapping.Map(triIndex.offset + face.vertex);
                                    Trace.Assert(mappedIndex >= 0 && mappedIndex < m_Parent.m_IndexMapping.VertexCount);

                                    array.Data[i++] = (T)Convert.ChangeType(mappedIndex, m_Type);

                                    if (faceIndex == 2)
                                    {
                                        coordSysConv.ConvertFace<T>(ref array.Data[i - 3], ref array.Data[i - 2], ref array.Data[i - 1]);
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
                }
            }

            Trace.Assert(i == array.Offset + array.Count);
        }

    }

    // ----------------------------------------------------------------------------
    // Can load a leaf's vertices and triangle indices
    // ----------------------------------------------------------------------------
    public class BspLoaderLeafRAMStreamSource : RenderResourceManager.RAMStreamSource
    {
        public class IndexMapping : IndexCompressor
        {
            public int VertexCount;
            public int IndexCount;

            internal IndexMapping(LoadedBspLevel bspLevel)
            {
                VertexCount = bspLevel.Header.Loader.GetVertexCount(bspLevel.Header);
                IndexCount = bspLevel.Header.Loader.GetMeshVertexCount(bspLevel.Header);

                Intervals = new Interval[1];
                Intervals[0] = new Interval(0, VertexCount - 1);
            }

            internal IndexMapping(BspTree bspTree, BspTree.Leaf leaf)
            {
                BspFile.Header header = bspTree.m_Level.Header;
                IntervalAnalyzer vertexIntervals = new IntervalAnalyzer();

                Interval[] leafFaceIntervals = bspTree.GetLeafFaceIntervals(leaf);

                foreach (Interval leafFaceInterval in leafFaceIntervals)
                {
                    using (BspFile.Faces faces = header.Loader.GetFaces(header, leafFaceInterval.Start, leafFaceInterval.Count()))
                    {
                        foreach (BspFile.Faces.Binary_face face in faces.m_Faces)
                        {
                            if (face.type == (int)BspFile.FaceType.Mesh || face.type == (int)BspFile.FaceType.Polygon)
                            {
                                IndexCount += face.n_meshverts;
                                using (BspFile.MeshVertices triIndices = header.Loader.GetMeshVertices(header, face.meshvert, face.n_meshverts))
                                {
                                    foreach (BspFile.MeshVertices.Binary_meshvert triIndex in triIndices.m_Vertices)
                                    {
                                        vertexIntervals.AddIndex(triIndex.offset + face.vertex);
                                    }
                                }
                            }
                        }
                    }
                }

                vertexIntervals.ProcessIntervals();
                VertexCount = vertexIntervals.GetIndexCount();
                Intervals = vertexIntervals.Intervals.ToArray();
            }

            internal int GetGlobalOffset()
            {
                return 0;
            }
        }

        internal BspTree m_BspTree;
        internal BspTree.Leaf m_Leaf;
        internal IndexMapping m_IndexMapping;

        public BspLoaderLeafRAMStreamSource(BspTree bspTree)
        {
            m_BspTree = bspTree;
            m_Leaf = null;
            m_IndexMapping = new IndexMapping(bspTree.m_Level);
        }

        public BspLoaderLeafRAMStreamSource(BspTree bspTree, BspTree.Leaf leaf)
        {
            m_BspTree = bspTree;
            m_Leaf = leaf;
            m_IndexMapping = new IndexMapping(bspTree, leaf);
                       
            // Note: We are obviously breaking the sharing of vertices between leafs using indices,
            // this has to do with the granularity of resource blocks, the level could be pre-processed 
            // for a specified optimal granularity
            // Performance: We are precomputing the VertexMapping here and storing it, this takes up memory, at the expense of the readers doing it,
            // it we are short on RAM and have processing power to burn, this might not be the best compromise, but it is easy to change!
        }

        

        public override RenderResourceManager.RAMStreamReader GetVertexReader(RenderResourceManager.DataStructure structure, Type type)
        {
            if (m_IndexMapping.VertexCount == 0)
                return null;

            return new BspLoaderVertexRAMStreamReader(this, structure, type);
        }

        public override RenderResourceManager.RAMStreamReader GetIndexReader(Type type)
        {
            if (m_IndexMapping.IndexCount == 0)
                return null;

            return new BspLoaderFaceIndexRAMStreamReader(this, type);
        }
    }
}