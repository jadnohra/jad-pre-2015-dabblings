using System;
using System.Collections.Generic;
using System.Diagnostics;
using Microsoft.Xna.Framework.Graphics;
using Microsoft.Xna.Framework.Content;

namespace Framework1.Quake3
{
    public class BspContentManager
    {
        class LightMapLoaderComp : RenderResourceManager.IManagedTextureLoader
        {
            BspContentManager m_Parent;

            internal LightMapLoaderComp(BspContentManager parent)
            {
                m_Parent = parent;
            }

            public Texture2D LoadTexture(string assetName)
            {
                int lightmapIndex = Int32.Parse(assetName);

                if (lightmapIndex < 0 || lightmapIndex >= m_Parent.m_LightMaps.Length)
                {
                    return null;
                }

                if (m_Parent.m_LightMaps[lightmapIndex] != null)
                {
                    return m_Parent.m_LightMaps[lightmapIndex];
                }

                m_Parent.m_LightMaps[lightmapIndex] = LoadTexture(lightmapIndex);

                return m_Parent.m_LightMaps[lightmapIndex];
            }

            public Texture2D LoadTexture(int index)
            {
                BspFile.Header header = m_Parent.m_BspLevel.Header;
                Texture2D tex = null;

                using (BspFile.Lightmaps lightmaps = header.Loader.GetLightmaps(header, index, 1))
                {
                    tex = new Texture2D(m_Parent.m_GraphicsDevice, 128, 128, 1, TextureUsage.None, SurfaceFormat.Rgb32);
                    tex.SetData<Color>(lightmaps.m_Lightmaps[0].pixels);

                    //tex.Save(string.Format("lm{0:G}.jpg", index), ImageFileFormat.Jpg);
                }

                return tex;
            }
        }

        class TextureMapLoaderComp : RenderResourceManager.IManagedTextureLoader
        {
            BspContentManager m_Parent;
            ContentManager m_ContentManager;
            List<Texture2D> m_DebugTextures = new List<Texture2D>();

            internal TextureMapLoaderComp(BspContentManager parent, ContentManager baseContentManager)
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
                    pixels[0].R = (byte) m_DebugTextures.Count;
                    pixels[0].G = (byte) m_DebugTextures.Count;
                    pixels[0].B = (byte) m_DebugTextures.Count;
                    pixels[0].A = 255;
                    
                    debugTex.SetData<Color>(pixels);

                    m_DebugTextures.Add(debugTex);

                    return debugTex;
                }
            }
        }

        public BspContentManager(LoadedBspLevel bspLevel, GraphicsDevice graphicsDevice, ContentManager baseContentManager)
        {
            m_GraphicsDevice = graphicsDevice;
            m_BspLevel = bspLevel;
            m_LightMapLoader = new LightMapLoaderComp(this);
            m_LightMaps = new Texture2D[bspLevel.Header.Loader.GetLightmapCount(bspLevel.Header)];

            m_TexLoader = new TextureMapLoaderComp(this, baseContentManager);
        }

        public RenderResourceManager.IManagedTextureLoader GetLightMapLoader()
        {
            return m_LightMapLoader;
        }

        public RenderResourceManager.IManagedTextureLoader GetTextureLoader()
        {
            return m_TexLoader;
        }

        LoadedBspLevel m_BspLevel;
        GraphicsDevice m_GraphicsDevice;
        LightMapLoaderComp m_LightMapLoader;
        TextureMapLoaderComp m_TexLoader;
        Texture2D[] m_LightMaps;
    }
}