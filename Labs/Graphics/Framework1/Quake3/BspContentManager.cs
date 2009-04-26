using System;
using Microsoft.Xna.Framework.Graphics;


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

        public BspContentManager(GraphicsDevice graphicsDevice, LoadedBspLevel bspLevel)
        {
            m_GraphicsDevice = graphicsDevice;
            m_BspLevel = bspLevel;
            m_LightMapLoader = new LightMapLoaderComp(this);
            m_LightMaps = new Texture2D[bspLevel.Header.Loader.GetLightmapCount(bspLevel.Header)];
        }

        public RenderResourceManager.IManagedTextureLoader GetLightMapLoader()
        {
            return m_LightMapLoader;
        }

        LoadedBspLevel m_BspLevel;
        GraphicsDevice m_GraphicsDevice;
        LightMapLoaderComp m_LightMapLoader;
        Texture2D[] m_LightMaps;
    }
}