using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Graphics;

namespace Framework1
{
    class Quake3FacesAssetRenderJob : RenderJob
    {
        override public UInt32 GetSortKey()
        {
            return 0;
        }

        override public void Execute(Renderer renderer_)
        {
            //BasicRenderer renderer = (BasicRenderer)renderer_;
            //GraphicsDevice device = renderer.Device;

            short[] indices;
            Quake3.AssetConvert.Convert(m_Faces.Header, m_Faces.m_Faces, out indices);
            //m_Faces.Header.Loader.GetMeshVertices(m_Faces.Header, m_Faces.m_Faces[0].meshvert

            //we need a nice way to load al the data needed to render a face directly from loader for 'streaming'
            //add this fctionality to Loader i.e: streaming loader!

            {
                m_Effect.Begin();
                m_Effect.CurrentTechnique.Passes[0].Begin();

                {
                }

                m_Effect.CurrentTechnique.Passes[0].End();
                m_Effect.End();
            }
        }

        Effect m_Effect;
        Quake3.BspFile.Faces m_Faces;
    }

    class Scene
    {
        virtual public void Render(ManualCamera camera, Matrix projection)
        {
        }
    }
}
