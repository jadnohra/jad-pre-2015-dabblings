using System;
using System.Diagnostics;
using System.Collections.Generic;
using Microsoft.Xna.Framework.Graphics;

namespace Framework1.Quake3
{
    using VertexFormat = VertexPositionColorTexture;

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
            internal RenderResourceManager.RAMVertexStreamProxy<VertexFormat> Vertices;
            internal RenderResourceManager.ManagedTexture2DProxy DiffuseTexture;

            public FaceRenderJob(RenderResourceManager renderResMan, BspTree bspTree, BspTree.Leaf leaf, int face)
            {
                VertexSemantics = renderResMan.NewGPUVertexSemanticsProxy(typeof(VertexFormat), VertexFormat.VertexElements);

                BspFaceRAMStreamSource source = new BspFaceRAMStreamSource(bspTree.m_Level, face);
                Vertices = renderResMan.NewRAMVertexStreamProxy<VertexFormat>(new RenderResourceManager.VertexSemantics(VertexFormat.VertexElements), source, true);
                TriangleList = renderResMan.NewRAMIndexStreamProxy<Int16>(source, true);

                LoadDiffuseTexture(renderResMan, bspTree, face);
            }

            void LoadDiffuseTexture(RenderResourceManager renderResMan, BspTree bspTree, int faceIndex)
            {
                 BspFile.Header header = bspTree.m_Level.Header;

                 using (BspFile.Faces faces = header.Loader.GetFaces(header, faceIndex, 1))
                 {
                     BspFile.Faces.Binary_face face = faces.m_Faces[0];

                     if (face.texture >= 0)
                     {
                         using (BspFile.Textures textures = header.Loader.GetTextures(header, face.texture, 1))
                         {
                             DiffuseTexture = renderResMan.NewManagedTexture2D(textures.m_Textures[0].GetTextureNameString(), true);
                         }
                     }
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
                RenderResourceManager.RAMStream<Int16> indexData;
                TriangleList.Get(out indexData);

                RenderResourceManager.RAMStream<VertexFormat> vertexData;
                Vertices.Get(out vertexData);

                Texture2D diffuseTexture = null;
                if (DiffuseTexture != null)
                {
                    DiffuseTexture.Get(out diffuseTexture);
                }

                // TODO!!!! this is also a resource!!! make it that way!
                renderer.Device.VertexDeclaration = VertexSemantics.m_VertexDeclaration;
                renderer.Device.Textures[0] = diffuseTexture;
                renderer.Device.DrawUserIndexedPrimitives<VertexFormat>(PrimitiveType.TriangleList, vertexData.Data, vertexData.Offset, vertexData.Count, indexData.Data, indexData.Offset, indexData.Count / 3);
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

        public RAMLeafRenderResourceBlock(RenderResourceManager renderResMan, BspTree bspTree, BspTree.Leaf leaf)
        {
            BspFile.Header header = bspTree.m_Level.Header;

            Interval[] leafFaceIntervals = GetLeafFaceIntervals(bspTree, leaf);

            // We could coalesce by material (texture)
            FaceRenderJobs = new FaceRenderJob[leafFaceIntervals.Length];

            int i = 0;
            foreach (Interval leafFaceInterval in leafFaceIntervals)
            {
                Trace.Assert(leafFaceInterval.Count() == 1);
                FaceRenderJobs[i++] = new FaceRenderJob(renderResMan, bspTree, leaf, leafFaceInterval.Start);
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

                    if ((face.type == (int)BspFile.FaceType.Mesh || face.type == (int)BspFile.FaceType.Polygon)
                        && face.n_meshverts > 0)
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