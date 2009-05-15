using System;
using System.Collections.Generic;
using System.Diagnostics;
using Microsoft.Xna.Framework.Graphics;
using Microsoft.Xna.Framework.Content;
using BlackRice.Framework.Graphics;

namespace BlackRice.Framework.Quake3.Load
{
    class TextureLoader : RenderResourceManager.IManagedTextureLoader
    {
        LevelContentManager m_Parent;
        ContentManager m_ContentManager;
        List<Texture2D> m_DebugTextures = new List<Texture2D>();

        internal TextureLoader(LevelContentManager parent, ContentManager baseContentManager)
        {
            m_Parent = parent;
            m_ContentManager = new ContentManager(baseContentManager.ServiceProvider, m_Parent.m_BspLevel.RootPath);
        }

        public Texture2D LoadTexture(string assetName)
        {
            try
            {
                return m_ContentManager.Load<Texture2D>(assetName);
            }
            catch (Exception e)
            {
                Trace.TraceWarning("Tex Not found: '" + assetName + "'");

                Texture2D debugTex = new Texture2D(m_Parent.m_GraphicsDevice, 1, 1, 1, TextureUsage.None, SurfaceFormat.Rgb32);

                Color[] pixels = new Color[1];
                pixels[0].R = (byte)m_DebugTextures.Count;
                pixels[0].G = (byte)m_DebugTextures.Count;
                pixels[0].B = (byte)m_DebugTextures.Count;
                pixels[0].A = 255;

                debugTex.SetData<Color>(pixels);

                m_DebugTextures.Add(debugTex);

                return debugTex;
            }
        }
    }
}