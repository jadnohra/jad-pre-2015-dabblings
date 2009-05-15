using System;
using Microsoft.Xna.Framework.Graphics;
using Microsoft.Xna.Framework.Content;
using BlackRice.Framework.Graphics;

namespace BlackRice.Framework.Quake3.Load
{
    public class LightmapLoader : RenderResourceManager.IManagedTextureLoader
    {
        LevelContentManager m_Parent;

        internal LightmapLoader(LevelContentManager parent)
        {
            m_Parent = parent;
        }

        public Texture2D LoadTexture(string assetName)
        {
            int lightmapIndex = Int32.Parse(assetName);

            if (lightmapIndex < 0 || lightmapIndex >= m_Parent.m_Lightmaps.Length)
            {
                return null;
            }

            if (m_Parent.m_Lightmaps[lightmapIndex] != null)
            {
                return m_Parent.m_Lightmaps[lightmapIndex];
            }

            m_Parent.m_Lightmaps[lightmapIndex] = LoadTexture(lightmapIndex);

            return m_Parent.m_Lightmaps[lightmapIndex];
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
}