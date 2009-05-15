using System;
using System.Diagnostics;
using System.Collections.Generic;
using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Graphics;

namespace Framework1.Quake3
{
    using VertexFormat = FaceVertex;

    public struct FaceVertex : IVertex
    {
        public Vector3 Position;
        public Vector2 DiffuseTextureCoordinate;
        public Vector2 LightmapTextureCoordinate;

        public static readonly VertexElement[] VertexElements = 
        {
            new VertexElement(0, 0, VertexElementFormat.Vector3, VertexElementMethod.Default, VertexElementUsage.Position, 0),
            new VertexElement(0, (sizeof(float) * 3), VertexElementFormat.Vector2, VertexElementMethod.Default, VertexElementUsage.TextureCoordinate, 0),
            new VertexElement(0, (sizeof(float) * 3) + (sizeof(float) * 2), VertexElementFormat.Vector2, VertexElementMethod.Default, VertexElementUsage.TextureCoordinate, 1),
        };

        public static int SizeInBytes = (sizeof(float) * 3) + (sizeof(float) * 2) + (sizeof(float) * 2);

        public IVertex Mul(float f) 
        { 
            FaceVertex ret; 
            
            ret.Position = Position * f;
            ret.DiffuseTextureCoordinate = DiffuseTextureCoordinate * f;
            ret.LightmapTextureCoordinate = LightmapTextureCoordinate * f; 
            
            return ret; 
        }

        public IVertex Add(IVertex b) 
        { 
            FaceVertex ret;
            FaceVertex bVertex = (FaceVertex)b;

            ret.Position = Position + bVertex.Position;
            ret.DiffuseTextureCoordinate = DiffuseTextureCoordinate + bVertex.DiffuseTextureCoordinate;
            ret.LightmapTextureCoordinate = LightmapTextureCoordinate + bVertex.LightmapTextureCoordinate; 
            
            return ret; 
        }
    }

    public abstract class LeafRenderResourceBlock 
    : RenderResourceBlockCollector.ResourceBlock
    {
    }

    // One variation of the block of resources for rendering a whole leaf
    // this variation streams vertices from RAM
    // the destructor MUST release the resources in the RenderResourceManager!
    // or even better automatically release on ProxyDestructor
    public class RAMLeafRenderResourceBlock 
    : LeafRenderResourceBlock
    , IBasicRenderable
    {
        class FaceRenderJob
        : IBasicRenderJob
        {
            internal RenderResourceManager.GPUVertexSemanticsProxy VertexSemantics;
            internal RenderResourceManager.RAMIndexStreamProxy<Int16> TriangleList;
            PrimitiveType TriangleListType;
            internal RenderResourceManager.RAMVertexStreamProxy<VertexFormat> Vertices;
            internal RenderResourceManager.ManagedTexture2DProxy DiffuseTexture;
            internal RenderResourceManager.ManagedTexture2DProxy LightmapTexture;

            public FaceRenderJob(RenderResourceManager renderResMan, LevelContentManager bspConentManager, BspTree bspTree, BspTree.Leaf leaf, int faceIndex)
            {
                BspFile.Header header = bspTree.m_Level.Header;

                using (BspFile.Faces faces = header.Loader.GetFaces(header, faceIndex, 1))
                {
                    BspFile.Faces.Binary_face face = faces.m_Faces[0];

                    VertexSemantics = renderResMan.NewGPUVertexSemanticsProxy(typeof(VertexFormat), VertexFormat.VertexElements);
                    RenderResourceManager.RAMStreamSource source = null;

                    if ((face.type == (int)BspFile.FaceType.Mesh) || (face.type == (int)BspFile.FaceType.Polygon))
                    {
                        source = new BspMeshFaceRAMStreamSource(bspTree.m_Level, faceIndex);
                        TriangleListType = PrimitiveType.TriangleList;
                    }
                    else if (face.type == (int)BspFile.FaceType.Patch)
                    {
                        source = new SimpleBspBezierFaceRAMStreamSource(bspTree.m_Level, faceIndex);
                        TriangleListType = PrimitiveType.TriangleStrip;
                    }
                    
                    Vertices = renderResMan.NewRAMVertexStreamProxy<VertexFormat>(new RenderResourceManager.VertexSemantics(VertexFormat.VertexElements), source, true);
                    TriangleList = renderResMan.NewRAMIndexStreamProxy<Int16>(source, true);

                    LoadTextures(renderResMan, bspConentManager, bspTree, face);
                }
            }

            void LoadTextures(RenderResourceManager renderResMan, LevelContentManager bspConentManager, BspTree bspTree, BspFile.Faces.Binary_face face)
            {
                 if (face.texture >= 0)
                 {
                     BspFile.Header header = bspTree.m_Level.Header;

                     using (BspFile.Textures textures = header.Loader.GetTextures(header, face.texture, 1))
                     {
                         DiffuseTexture = renderResMan.LoadManagedTexture2D(bspConentManager.GetTextureLoader(), textures.m_Textures[0].GetTextureNameString(), true);
                     }
                 }

                 if (face.lm_index >= 0)
                 {
                     LightmapTexture = renderResMan.LoadManagedTexture2D(bspConentManager.GetLightmapLoader(), string.Format("{0:G}", face.lm_index), true);
                 }
            }


            internal void Evict(RenderResourceManager resMan)
            {
                resMan.Delete(Vertices);
                Vertices = null;

                resMan.Delete(TriangleList);
                TriangleList = null;

                if (DiffuseTexture != null)
                {
                    resMan.Delete(DiffuseTexture);
                    DiffuseTexture = null;
                }
            }

            internal void Prepare(RenderResourceManager resMan)
            {
                Vertices.Prepare(resMan);
                TriangleList.Prepare(resMan);
            }

            public void Execute(BasicRenderer renderer)
            {
                //if (TriangleListType != PrimitiveType.TriangleStrip)
                //    return;

                RenderResourceManager.RAMStream<Int16> indexData;
                TriangleList.Get(out indexData);

                RenderResourceManager.RAMStream<VertexFormat> vertexData;
                Vertices.Get(out vertexData);

                Texture2D diffuseTexture = null;
                if (DiffuseTexture != null)
                {
                    DiffuseTexture.Get(out diffuseTexture);
                }

                Texture2D lightmapTexture = null;
                if (LightmapTexture != null)
                {
                    LightmapTexture.Get(out lightmapTexture);
                }

                renderer.Device.Textures[0] = diffuseTexture;
                renderer.Device.Textures[1] = lightmapTexture;
                renderer.Device.VertexDeclaration = VertexSemantics.m_VertexDeclaration;
                renderer.Device.DrawUserIndexedPrimitives<VertexFormat>(TriangleListType, vertexData.Data, vertexData.Offset, vertexData.Count, indexData.Data, indexData.Offset, TriangleListTypes.ToPrimitiveCount(TriangleListType, indexData.Count));
            }
        }

        FaceRenderJob[] FaceRenderJobs;
        
        public override void Evict(RenderResourceManager resMan)
        {
            foreach(FaceRenderJob job in FaceRenderJobs)
            {
                job.Evict(resMan);
            }
        }

        public override void Prepare(RenderResourceManager resMan)
        {
            foreach (FaceRenderJob job in FaceRenderJobs)
            {
                job.Prepare(resMan);
            }
        }

        public RAMLeafRenderResourceBlock(RenderResourceManager renderResMan, LevelContentManager bspConentManager, BspTree bspTree, BspTree.Leaf leaf)
        {
            BspFile.Header header = bspTree.m_Level.Header;

            Interval[] leafFaceIntervals = GetLeafFaceIntervals(bspTree, leaf);

            // We could coalesce by material (texture)
            FaceRenderJobs = new FaceRenderJob[leafFaceIntervals.Length];

            int i = 0;
            foreach (Interval leafFaceInterval in leafFaceIntervals)
            {
                Trace.Assert(leafFaceInterval.Count() == 1);
                FaceRenderJobs[i++] = new FaceRenderJob(renderResMan, bspConentManager, bspTree, leaf, leafFaceInterval.Start);
            }
        }

        Interval[] GetLeafFaceIntervals(BspTree bspTree, BspTree.Leaf leaf)
        {
            BspFile.Header header = bspTree.m_Level.Header;

            List<Interval> intervals = new List<Interval>();

            for (int lf = leaf.firstLeafFace, lfi = 0; lfi < leaf.leafFaceCount; ++lfi, ++lf)
            {
                int faceIndex = bspTree.m_LeafFaces[lf].faceIndex;

                using (BspFile.Faces faces = header.Loader.GetFaces(header, faceIndex, 1))
                {
                    BspFile.Faces.Binary_face face = faces.m_Faces[0];

                    if ((face.type == (int)BspFile.FaceType.Mesh && face.n_meshverts > 0)
                        || (face.type == (int)BspFile.FaceType.Polygon && face.n_meshverts > 0)
                        || (face.type == (int)BspFile.FaceType.Patch && face.n_vertexes > 0)
                        )
                    {
                        intervals.Add(new Interval(faceIndex, faceIndex));
                    }
                }
            }

            return intervals.ToArray();
        }

        public void PushJobs(BasicRenderQueue queue, BasicRenderer renderer)
        {
            foreach (FaceRenderJob job in FaceRenderJobs)
            {
                queue.Push((IBasicRenderJob)job);
            }
        }
    }

}