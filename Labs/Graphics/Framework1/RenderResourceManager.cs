using System;
using System.IO;
using System.Diagnostics;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using Microsoft.Xna.Framework.Graphics;
using Microsoft.Xna.Framework.Content;

namespace Framework1
{
    public interface IVertex
    {
        IVertex Mul(float f);
        IVertex Add(IVertex b);
    }

    /*
     * The goal here is to enable the manager to per example add multiple vertex buffers 
     * into one vertex buffer
     * 
     * Source classes are kept in memory, Reader classes are temporary objects, so they should be written with
     * this in mind, many variations and compromizes on processing vs memory can be expressed easily this way.
     **/
    public class RenderResourceManager
    {
       
        public class VertexSemantics
        {
            public readonly VertexElement[] Layout;

            public VertexSemantics(VertexElement[] layout)
            {
                Layout = layout;
            }

            static public bool Equals(VertexElement[] layout1, VertexElement[] layout2)
            {
                if (layout1.Length == layout2.Length)
                {
                    for (int i = 0; i < layout1.Length; ++i)
                    {
                        if (layout1[i] != layout2[i])
                            return false;
                    }

                    return true;
                }

                return false;
            }
        }

        public class GPUVertexSemanticsProxy
        {
            public VertexDeclaration m_VertexDeclaration;

            public GPUVertexSemanticsProxy(VertexDeclaration vertexDeclaration)
            {
                m_VertexDeclaration = vertexDeclaration;
            }
        }

        public class RAMStream<T>
        {
            public T[] Data;
            public int Offset;
            public int Count;

            public int GetSizeBytes() { return Count * Marshal.SizeOf(typeof(T)); }
            public void Evict() { Data = null; }
        }

        public interface IRAMVertexStreamReader
        {
            int Count();
            void Read<T>(ref RAMStream<T> array, VertexSemantics semantics) where T : IVertex;
        }

        public interface IRAMIndexStreamReader
        {
            int Count();
            void Read<T>(ref RAMStream<T> array);
        }

        public abstract class RAMStreamSource
        {
            public abstract IRAMVertexStreamReader GetVertexReader(VertexSemantics vertexStructure, Type vertexType);
            public abstract IRAMIndexStreamReader GetIndexReader(Type indexType);
        }

        public abstract class RAMStreamProxyBase
        {
            public abstract int GetSizeBytes();
            public abstract void Evict();
        }

        public class RAMVertexStreamProxy<T> : RAMStreamProxyBase
        {
            RAMStream<T> m_RAMVertexStream;

            public RAMVertexStreamProxy(RAMStream<T> vertexStream)
            {
                m_RAMVertexStream = vertexStream;
            }

            public bool Check() { return true; }
            
            public void Prepare(RenderResourceManager renderResourceManager) 
            { 
                if (m_RAMVertexStream == null)
                {
                    renderResourceManager.Restore<T>(this);
                }
            } 
            
            public void Get(out RAMStream<T> array) { array = m_RAMVertexStream; }

            public override int GetSizeBytes() { return m_RAMVertexStream.GetSizeBytes(); }
            public override void Evict() { m_RAMVertexStream.Evict(); m_RAMVertexStream = null; }
        }

        public class RAMIndexStreamProxy<T> : RAMStreamProxyBase
        {
            RAMStream<T> m_RAMIndexStream;

            public RAMIndexStreamProxy(RAMStream<T> indexStream)
            {
                m_RAMIndexStream = indexStream;
            }

            public bool Check() { return true; }

            public void Prepare(RenderResourceManager renderResourceManager)
            {
                if (m_RAMIndexStream == null)
                {
                    renderResourceManager.Restore<T>(this);
                }
            }

            public void Get(out RAMStream<T> array) { array = m_RAMIndexStream; }

            public override int GetSizeBytes() { return m_RAMIndexStream.GetSizeBytes(); }
            public override void Evict() { m_RAMIndexStream.Evict(); m_RAMIndexStream = null; }
        }

        public interface IManagedTextureLoader
        {
            Texture2D LoadTexture(string assetName);
        }

        abstract public class ManagedTextureProxyBase
        {
            public abstract int GetSizeBytes();
            public abstract void Evict();
        }

        public class ManagedTexture2DProxy : ManagedTextureProxyBase
        {
            Texture2D m_Texture2D;
            static int trc = 0;

            public ManagedTexture2DProxy(Texture2D tex2D)
            {
                m_Texture2D = tex2D;
            }

            public void Get(out Texture2D tex2D) { tex2D = m_Texture2D; }

            public override int GetSizeBytes() 
            { 
                if (m_Texture2D == null)
                    return 0;

                if (trc++ == 0)
                    Trace.TraceWarning("ManagedTexture2DProxy.GetSizeBytes() not implemented");

                return 0;
            }
            public override void Evict() { Trace.Assert(false, "TODO"); }
        }

        public interface IManagedEffectLoader
        {
            Effect LoadEffect(string assetName);
        }

        public class ManagedEffectProxy
        {
            Effect m_Effect;
            static int trc = 0;

            public ManagedEffectProxy(Effect effect)
            {
                m_Effect = effect;
            }

            public void Get(out Effect effect) { effect = m_Effect; }

            public void Evict() { Trace.Assert(false, "TODO"); }
        }


        public RenderResourceManager(GraphicsDevice graphicsDevice, ContentManager contentManager, string textureRelativeRoot, string effectRelativeRoot)
        {
            m_GraphicsDevice = graphicsDevice;

            if (contentManager != null)
            {
                string texRootPath = Path.Combine(contentManager.RootDirectory, textureRelativeRoot);
                m_TextureContentManager = new ContentManager(contentManager.ServiceProvider, texRootPath);
            }

            if (contentManager != null)
            {
                string effectRootPath = Path.Combine(contentManager.RootDirectory, effectRelativeRoot);
                m_EffectContentManager = new ContentManager(contentManager.ServiceProvider, effectRootPath);
            }
        }

        public GPUVertexSemanticsProxy NewGPUVertexSemanticsProxy(Type type, VertexElement[] layout)
        {
            GPUVertexSemanticsProxy proxy;

            if (m_RenderVertexSemanticsProxies.TryGetValue(type, out proxy))
            {
                Trace.Assert(VertexSemantics.Equals(proxy.m_VertexDeclaration.GetVertexElements(), layout));

                return proxy;
            }

            proxy = new GPUVertexSemanticsProxy(new VertexDeclaration(m_GraphicsDevice, layout));
            m_RenderVertexSemanticsProxies.Add(type, proxy);

            return proxy;
        }

        public RAMVertexStreamProxy<T> NewRAMVertexStreamProxy<T>(VertexSemantics semantics, RAMStreamSource source, bool evictable)
        where T : IVertex
        {
            IRAMVertexStreamReader reader = source.GetVertexReader(semantics, typeof(T));

            if (reader == null)
                return null;

            RAMStream<T> vertexStream = new RAMStream<T>();
           
            // Note: implement sharing by DataStructure when using VRAM resources
            // for RAM this is probably good enough like this
            vertexStream.Offset = 0;
            vertexStream.Count = reader.Count();
            vertexStream.Data = new T[vertexStream.Count];

            reader.Read(ref vertexStream, semantics);

            RAMVertexStreamProxy<T> proxy = new RAMVertexStreamProxy<T>(vertexStream);
            m_RAMStreamProxies.Add(proxy);

            m_RAMStreamDataBytes += vertexStream.Count * Marshal.SizeOf(typeof(T));

            return proxy;
        }

        public void Restore<T>(RAMVertexStreamProxy<T> proxy)
        {
            Trace.Assert(false, "TODO");
        }

        public void Delete<T>(RAMVertexStreamProxy<T> proxy)
        {
            Trace.Assert(false, "TODO");
            //m_RAMStreamDataBytes -= 
        }

        public RAMIndexStreamProxy<T> NewRAMIndexStreamProxy<T>(RAMStreamSource source, bool evictable)
        {
            Trace.Assert(typeof(T).IsValueType);
            IRAMIndexStreamReader reader = source.GetIndexReader(typeof(T));

            if (reader == null)
                return null;

            RAMStream<T> indexStream = new RAMStream<T>();

            // Note: implement sharing by DataStructure when using VRAM resources
            // for RAM this is probably good enough like this
            indexStream.Offset = 0;
            indexStream.Count = reader.Count();
            indexStream.Data = new T[indexStream.Count];

            reader.Read(ref indexStream);

            RAMIndexStreamProxy<T> proxy = new RAMIndexStreamProxy<T>(indexStream);
            m_RAMStreamProxies.Add(proxy);

            m_RAMStreamDataBytes += indexStream.Count * Marshal.SizeOf(typeof(T));

            return proxy;
        }

        public void Restore<T>(RAMIndexStreamProxy<T> proxy)
        {
            Trace.Assert(false, "TODO");
        }

        public void Delete<T>(RAMIndexStreamProxy<T> proxy)
        {
            Trace.Assert(false, "TODO");
            //m_RAMStreamDataBytes -= 
        }

        // Load will be used for shared textures
        // New will be used for not shared ones
        public ManagedTexture2DProxy LoadManagedTexture2D(string assetName, bool evictable)
        {
            Texture2D tex2D = null;

            try
            {
                tex2D = m_TextureContentManager.Load<Texture2D>(assetName);
            }
            catch (Exception e)
            {
                //Trace.TraceWarning(e.Message);
            }

            return LoadManagedTexture2D(tex2D, evictable);
        }

        public ManagedTexture2DProxy LoadManagedTexture2D(IManagedTextureLoader loader, string assetName, bool evictable)
        {
            Texture2D tex2D = null;

            try
            {
                tex2D = loader.LoadTexture(assetName);
            }
            catch (Exception e)
            {
                //Trace.TraceWarning(e.Message);
            }

            return LoadManagedTexture2D(tex2D, evictable);
        }

        ManagedTexture2DProxy LoadManagedTexture2D(Texture2D tex2D, bool evictable)
        {
            if (tex2D == null)
                return null;

            ManagedTexture2DProxy proxy = new ManagedTexture2DProxy(tex2D);
            // Wrong! texture might be shared using ContentManager
            // m_ManagedTextureBytes += proxy.GetSizeBytes();

            m_ManagedTextureProxies.Add(proxy);

            return proxy;
        }

        public void Restore(ManagedTexture2DProxy proxy)
        {
            Trace.Assert(false, "TODO");
        }

        public void Delete(ManagedTexture2DProxy proxy)
        {
            Trace.Assert(false, "TODO");
            //m_ManagedTextureBytes -= 
        }


        // Load will be used for shared textures
        // New will be used for not shared ones
        public ManagedEffectProxy LoadManagedEffect(string assetName, bool evictable)
        {
            Effect effect = null;

            try
            {
                effect = m_EffectContentManager.Load<Effect>(assetName);
            }
            catch (Exception e)
            {
                //Trace.TraceWarning(e.Message);
            }

            return LoadManagedEffect(effect, evictable);
        }

        public ManagedEffectProxy LoadManagedTexture2D(IManagedEffectLoader loader, string assetName, bool evictable)
        {
            Effect effect = null;

            try
            {
                effect = loader.LoadEffect(assetName);
            }
            catch (Exception e)
            {
                //Trace.TraceWarning(e.Message);
            }

            return LoadManagedEffect(effect, evictable);
        }

        ManagedEffectProxy LoadManagedEffect(Effect effect, bool evictable)
        {
            if (effect == null)
                return null;

            ManagedEffectProxy proxy = new ManagedEffectProxy(effect);
            // Wrong! texture might be shared using ContentManager
            // m_ManagedTextureBytes += proxy.GetSizeBytes();

            m_ManagedEffectProxies.Add(proxy);

            return proxy;
        }

        public void Restore(ManagedEffectProxy proxy)
        {
            Trace.Assert(false, "TODO");
        }

        public void Delete(ManagedEffectProxy proxy)
        {
            Trace.Assert(false, "TODO");
        }

        GraphicsDevice m_GraphicsDevice;
        ContentManager m_TextureContentManager;
        ContentManager m_EffectContentManager;
        List<RAMStreamProxyBase> m_RAMStreamProxies = new List<RAMStreamProxyBase>();
        List<ManagedTextureProxyBase> m_ManagedTextureProxies = new List<ManagedTextureProxyBase>();
        List<ManagedEffectProxy> m_ManagedEffectProxies = new List<ManagedEffectProxy>();
        Dictionary<Type, GPUVertexSemanticsProxy> m_RenderVertexSemanticsProxies = new Dictionary<Type, GPUVertexSemanticsProxy>();
        int m_RAMManagementBytes = 0;
        int m_RAMStreamDataBytes = 0;
        int m_ManagedTextureBytes = 0;
    }
}