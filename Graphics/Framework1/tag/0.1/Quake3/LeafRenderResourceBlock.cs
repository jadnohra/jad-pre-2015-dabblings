//using System.Diagnostics;
//using System.Collections.Generic;
using System;
using Microsoft.Xna.Framework.Graphics;

namespace Framework1.Quake3
{
    public abstract class LeafRenderResourceBlock : RenderResourceBlockCollector.ResourceBlock
    {
    }

    // One variation of the block of resources for rendering a whole leaf
    // this variation streams vertices from RAM
    // the destructor MUST release the resources in the RenderResourceManager!
    // or even better automatically release on ProxyDestructor
    public class RAMLeafRenderResourceBlock : LeafRenderResourceBlock, IBasicRenderable, IBasicRenderJob
    {
        RenderResourceManager.RAMIndexStreamProxy<Int16> TriangleList;
        RenderResourceManager.RAMVertexStreamProxy<VertexPositionColor> Vertices;
        RenderResourceManager.ManagedTexture2DProxy DiffuseTexture;

        public override void Evict(RenderResourceManager resMan)
        {
            if (TriangleList != null)
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
        }

        public override void Prepare(RenderResourceManager resMan)
        {
            if (TriangleList != null)
            {
                Vertices.Prepare(resMan);
                TriangleList.Prepare(resMan);
            }
        }

        public RAMLeafRenderResourceBlock(RenderResourceManager renderResMan, BspTree bspTree, BspTree.Leaf leaf)
        {
            BspLoaderLeafRAMStreamSource source = new BspLoaderLeafRAMStreamSource(bspTree, leaf);
            Vertices = renderResMan.NewRAMVertexStreamProxy<VertexPositionColor>(new RenderResourceManager.DataStructure(VertexPositionColor.VertexElements), source, true);
            TriangleList = renderResMan.NewRAMIndexStreamProxy<Int16>(source, true);
            //DiffuseTexture = renderResMan.NewManagedTexture2D("e7/e7bricks01", true);
        }

        // ----------------------------------------------------------------------------
        // IBasicRenderable implementation
        // ----------------------------------------------------------------------------

        public void PushJobs(BasicRenderQueue queue, BasicRenderer renderer)
        {
            queue.Push((IBasicRenderJob)this);
        }

        // ----------------------------------------------------------------------------
        // Embedded IBasicRenderJob implementation until we need to do otherwize
        // ----------------------------------------------------------------------------

        public void Execute(BasicRenderer renderer)
        {
            // Performance: This can be optimized and the resource block not even created when the leaf has nothing to render
            if (this.TriangleList != null)
            {
                RenderResourceManager.RAMStream<Int16> indexData;
                this.TriangleList.Get(out indexData);

                RenderResourceManager.RAMStream<VertexPositionColor> vertexData;
                this.Vertices.Get(out vertexData);

                // TODO!!!! this is also a resource!!! make it that way!
                renderer.Device.VertexDeclaration = new VertexDeclaration(renderer.Device, VertexPositionColor.VertexElements);
                //renderer.Device.DrawUserPrimitives<VertexPositionColor>(PrimitiveType.PointList, vertexData.Data, vertexData.Offset, vertexData.Count);
                renderer.Device.DrawUserIndexedPrimitives<VertexPositionColor>(PrimitiveType.TriangleList, vertexData.Data, vertexData.Offset, vertexData.Count, indexData.Data, indexData.Offset, indexData.Count / 3);
            }
        }

    }

}