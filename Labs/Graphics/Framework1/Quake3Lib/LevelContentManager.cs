using System;
using System.IO;
using Microsoft.Xna.Framework.Graphics;
using Microsoft.Xna.Framework.Content;
using Framework1.Quake3.Load;

namespace Framework1.Quake3
{
    public class LevelContentManager
    {
        public LevelContentManager(LoadedBspLevel bspLevel, GraphicsDevice graphicsDevice, ContentManager baseContentManager)
        {
            m_GraphicsDevice = graphicsDevice;
            m_BspLevel = bspLevel;
            
            m_LightmapLoader = new LightmapLoader(this);
            m_Lightmaps = new Texture2D[bspLevel.Header.Loader.GetLightmapCount(bspLevel.Header)];
            
            m_TexLoader = new TextureLoader(this, baseContentManager);

            m_ShaderLoader = new ScriptLoader(this, "*.shader");
        }

        public RenderResourceManager.IManagedTextureLoader GetLightmapLoader()
        {
            return m_LightmapLoader;
        }

        public RenderResourceManager.IManagedTextureLoader GetTextureLoader()
        {
            return m_TexLoader;
        }

        internal LoadedBspLevel m_BspLevel;
        internal GraphicsDevice m_GraphicsDevice;
        LightmapLoader m_LightmapLoader;
        TextureLoader m_TexLoader;
        ScriptLoader m_ShaderLoader;
        internal Texture2D[] m_Lightmaps;
    }
}