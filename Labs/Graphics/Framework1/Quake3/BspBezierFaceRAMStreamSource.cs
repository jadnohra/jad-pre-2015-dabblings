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
            int m_IndexCount;

            public IndexReader(BspBezierFaceRAMStreamSource parent, Type type)
                : base(parent, type)
            {
                 BspFile.Header header = m_Parent.m_Level.Header;

                 using (BspFile.Faces faces = header.Loader.GetFaces(header, m_Parent.m_Face, 1))
                 {
                     BspFile.Faces.Binary_face face = faces.m_Faces[0];
                     m_IndexCount = Geometry.UniformGridTesselator.GetTriangleStripSize((Int16)face.size_x, (Int16)face.size_y);
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
                     Geometry.UniformGridTesselator.GenerateTriangleStrip(face.size_x, face.size_y, ref array.Data, array.Offset);
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

                        //continue here
                        /*
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

                            // read the vertices (grid x * y  divided into 3*3 subgrids + sharing 1 line), set control points

                            for (int patchIndexX = 0, vertexIndexX = 0; patchIndexX < patchCountX; ++patchIndexX, vertexIndexX += 2)
                            {
                                for (int patchIndexY = 0, vertexIndexY = 0; patchIndexY < patchCountY; ++patchIndexY, vertexIndexY += 2)
                                {
                                    int vertexRowIndex = face.size_x * vertexIndexY;

                                    for (int controlRowOffset = 0; controlRowOffset < 9; controlRowOffset += 3)
                                    {
                                        int vertexIndexRow0 = vertexRowIndex + vertexIndexX + controlRowOffset;

                                        patch.ControlPoints[controlRowOffset + 0] = array.Data[array.Offset + vertexIndexRow0];
                                        patch.ControlPoints[controlRowOffset + 1] = array.Data[array.Offset + vertexIndexRow0 + 1];
                                        patch.ControlPoints[controlRowOffset + 2] = array.Data[array.Offset + vertexIndexRow0 + 2];
                                    }

                                    tesselator.Tesselate(5, patch, ref tesselation);

                                    {
                                        Console.WriteLine("");
                                        Console.WriteLine(string.Format("{0:G}.{0:G}", patchIndexX, patchIndexY));

                                        Type type = typeof(T);
                                        VertexElement[] semanticEls = semantics.Layout;
                                        FieldInfo[] typeFields = type.GetFields();

                                        for (int fieldIndex = 0; fieldIndex < semanticEls.Length; ++fieldIndex)
                                        {
                                            VertexElement vEl = semanticEls[fieldIndex];
                                            FieldInfo fieldInfo = typeFields[fieldIndex];
                                        
                                            if (vEl.VertexElementUsage == VertexElementUsage.Position)
                                            {
                                                for (int vi = 0; vi < tesselation.Vertices.Length; ++vi)
                                                {
                                                    Vector3 pos = (Vector3) (fieldInfo.GetValue(((T) tesselation.Vertices[vi])));

                                                    Console.WriteLine(string.Format("{0:G} - {1:F},{2:F},{3:F}", vi, pos.X, pos.Y, pos.Z));
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                         */ 
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
                    m_IndexCount = Geometry.UniformGridTesselator.GetTriangleStripSize((Int16)face.size_x, (Int16)face.size_y);
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
                    Geometry.UniformGridTesselator.GenerateTriangleStrip(face.size_x, face.size_y, ref array.Data, array.Offset);
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