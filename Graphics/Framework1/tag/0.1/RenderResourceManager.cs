using System;
using System.IO;
using System.Diagnostics;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using Microsoft.Xna.Framework.Graphics;
using Microsoft.Xna.Framework.Content;

namespace Framework1
{
    /*
     * The goal here is to enable the manager to per example add multiple vertex buffers 
     * into one vertex buffer
     * 
     **/ 
    public class RenderResourceManager
    {
        public class DataStructure
        {
            public readonly VertexElement[] Layout;

            public DataStructure(VertexElement[] layout)
            {
                Layout = layout;
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
        
        public abstract class RAMStreamReader
        {
            public abstract int Count();
            public abstract void Read<T>(ref RAMStream<T> array);
        }

        public abstract class RAMStreamSource
        {
            public abstract RAMStreamReader GetVertexReader(DataStructure vertexStructure, Type vertexType);
            public abstract RAMStreamReader GetIndexReader(Type indexType);
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

        abstract public class ManagedTextureProxyBase
        {
            public abstract int GetSizeBytes();
            public abstract void Evict();
        }

        public class ManagedTexture2DProxy : ManagedTextureProxyBase
        {
            Texture2D m_Texture2D;

            public ManagedTexture2DProxy(Texture2D tex2D)
            {
                m_Texture2D = tex2D;
            }

            public void Get(out Texture2D tex2D) { tex2D = m_Texture2D; }

            public override int GetSizeBytes() 
            { 
                if (m_Texture2D == null)
                    return 0;

                Trace.TraceWarning("ManagedTexture2DProxy.GetSizeBytes() not implemented");
                return 0;
            }
            public override void Evict() { Trace.Assert(false, "TODO"); }
        }

        public RenderResourceManager(ContentManager contentManager, string textureRelativeRoot)
        {
            if (contentManager != null)
            {
                string texRootPath = Path.Combine(contentManager.RootDirectory, textureRelativeRoot);
                m_TextureContentManager = new ContentManager(contentManager.ServiceProvider, texRootPath);
            }
        }

        public RAMVertexStreamProxy<T> NewRAMVertexStreamProxy<T>(DataStructure structure, RAMStreamSource source, bool evictable)
        {
            RAMStreamReader reader = source.GetVertexReader(structure, typeof(T));

            if (reader == null)
                return null;

            RAMStream<T> vertexStream = new RAMStream<T>();
           
            // Note: implement sharing by DataStructure when using VRAM resources
            // for RAM this is probably good enough like this
            vertexStream.Offset = 0;
            vertexStream.Count = reader.Count();
            vertexStream.Data = new T[vertexStream.Count];

            reader.Read(ref vertexStream);

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
            RAMStreamReader reader = source.GetIndexReader(typeof(T));

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

        public ManagedTexture2DProxy NewManagedTexture2D(string assetName, bool evictable)
        {
            Texture2D tex2D = m_TextureContentManager.Load<Texture2D>(assetName);

            if (tex2D == null)
                return null;

            ManagedTexture2DProxy proxy = new ManagedTexture2DProxy(tex2D);
            m_ManagedTextureBytes += proxy.GetSizeBytes();

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

        ContentManager m_TextureContentManager;
        List<RAMStreamProxyBase> m_RAMStreamProxies = new List<RAMStreamProxyBase>();
        List<ManagedTextureProxyBase> m_ManagedTextureProxies = new List<ManagedTextureProxyBase>();
        int m_RAMManagementBytes = 0;
        int m_RAMStreamDataBytes = 0;
        int m_ManagedTextureBytes = 0;
    }
}