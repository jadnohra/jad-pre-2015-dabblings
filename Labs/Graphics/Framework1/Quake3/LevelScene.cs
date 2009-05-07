using System;
using System.Collections.Generic;
using System.IO;
using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Graphics;
using Microsoft.Xna.Framework.Content;

namespace Framework1.Quake3
{
    public class LoadedBspLevel
    {
        public BspFile.Header Header;
        public BspFile.CoordSysConv CoordSysConv;
    }

    class LevelScene
    : Scene
    {
        LoadedBspLevel m_LoadedLevel;
        BspTree m_Tree;

        // All this and render is WIP, will be moved elsewhere
        BspContentManager m_BspContentManager;
        RenderResourceBlockCollector m_RenderResourceBlockCollector;
        RenderResourceManager m_RenderResourceManager;
        BasicRenderer m_Renderer;
        BasicRenderQueue m_RenderQueue;


        public bool IsLoaded()
        {
            return m_LoadedLevel != null;
        }

        public bool Load(string path, ContentManager content, GraphicsDevice graphicsDevice)
        {
            if (IsLoaded())
            {
                return false;
            }

            m_LoadedLevel = new LoadedBspLevel();
            BspLoader loader = new BspLoader(path);
            m_LoadedLevel.Header = loader.Open();

            if (m_LoadedLevel.Header != null)
            {
                m_LoadedLevel.CoordSysConv = new BspFile.CoordSysConv();

                m_Tree = new BspTree();
                m_Tree.Load(m_LoadedLevel);

                m_RenderResourceBlockCollector = new RenderResourceBlockCollector();
                string texRelPath = Path.Combine(Path.GetDirectoryName(path), "../");
                //string texRelPath = Path.Combine(content.RootDirectory, "jof3dm2.pk3/textures/");
                m_RenderResourceManager = new RenderResourceManager(graphicsDevice, content, texRelPath);
                m_Renderer = new BasicRenderer(content, graphicsDevice);
                m_RenderQueue = new BasicRenderQueue(m_Renderer);
                m_BspContentManager = new BspContentManager(graphicsDevice, m_LoadedLevel);
            }
            else
            {
                m_LoadedLevel = null;
            }

            return IsLoaded();
        }

        public void Unload()
        {
            m_LoadedLevel = null;
        }

        override public void Render(ManualCamera camera, Matrix projection)
        {
            BspTree.VisibleLeafs visibleLeafs = new BspTree.VisibleLeafs();
            m_Tree.FindVisibleLeafs(camera.GetWorldTransform().Translation, visibleLeafs, true);
            m_Tree.Evict(m_RenderResourceBlockCollector);

            int newVisibleLeafCount = 0;

            foreach (BspTree.Leaf leaf in visibleLeafs)
            {
                if (leaf.renderResourceBlock == null)
                {
                    {
                        
                        ++newVisibleLeafCount;
                        leaf.renderResourceBlock = new RAMLeafRenderResourceBlock(m_RenderResourceManager, m_BspContentManager, m_Tree, leaf);
                    }
                }
                else
                {
                    leaf.renderResourceBlock.Prepare(m_RenderResourceManager);
                }

                m_RenderQueue.Push((IBasicRenderable)leaf.renderResourceBlock);
            }

            m_Renderer.Push(m_RenderQueue);
            m_Renderer.Render(camera, projection);

            if (newVisibleLeafCount > 0)
            {
                Console.WriteLine(String.Format("{0:G} new visible leafs", newVisibleLeafCount));
            }
        }
    }
}
