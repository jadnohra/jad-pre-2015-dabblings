using System;
using System.IO;
using System.Diagnostics;
using System.Runtime.InteropServices;

namespace BlackRice.Framework.Quake3.Load
{
    //using BspFile;

    public class BspLoader
    {
        string m_Path;
        Stream m_Stream;
        BinaryReader m_Reader;
        byte[] m_Buffer;

        public BspLoader(string path)
        {
            m_Path = path;
            try
            {
                m_Stream = File.OpenRead(m_Path);
                m_Reader = new BinaryReader(m_Stream);
                m_Buffer = new byte[128];
            }
            catch
            {
                m_Stream = null;
            }
        }

        public BspFile.Header Open()
        {
            if (m_Stream == null)
                return null;

            Type type = typeof(BspFile.Header.Binary);
            int size = Marshal.SizeOf(type);

            if (m_Buffer.Length < size)
            {
                m_Buffer = new byte[size];
            }

            m_Stream.Seek(0, SeekOrigin.Begin);
            if (m_Stream.Read(m_Buffer, 0, size) == size)
            {
                BspFile.Header.Binary binary = (BspFile.Header.Binary)BinarySerializer.RawDeserialize(m_Buffer, 0, type);

                BspFile.Header asset = new BspFile.Header();
                asset.Header = asset;
                asset.Loader = this;
                asset.StreamOffset = 0;

                if (asset.construct(binary))
                    return asset;
            }

            return null;
        }

        public int GetFaceCount(BspFile.Header header)
        {
            Type type = typeof(BspFile.Faces.Binary_face);
            int size = Marshal.SizeOf(type);
            int length = header.m_DirEntries[(int)(BspFile.Header.EntityType.Faces)].length;
            int count = (length / size);

            Trace.Assert(count * size == length);

            return count;
        }

        public BspFile.Faces GetFaces(BspFile.Header header, int startIndex, int count)
        {
            if (header == null)
                return null;

            Type type = typeof(BspFile.Faces.Binary_face);
            int size = Marshal.SizeOf(type);

            long baseOffset = header.StreamOffset + header.m_DirEntries[(int)(BspFile.Header.EntityType.Faces)].offset;
            long startOffset = baseOffset + startIndex * size;
            long endOffset = startOffset + count * size;
            int totalSize = (int)(endOffset - startOffset);

            if (m_Buffer.Length < totalSize)
            {
                m_Buffer = new byte[totalSize];
            }

            BspFile.Faces.Binary_face[] binaryFaces = new BspFile.Faces.Binary_face[count];

            m_Stream.Seek(startOffset, SeekOrigin.Begin);
            if (m_Stream.Read(m_Buffer, 0, totalSize) == totalSize)
            {
                int offset = 0;
                for (int i = 0; i < count; ++i, offset += size)
                {
                    binaryFaces[i] = (BspFile.Faces.Binary_face)BinarySerializer.RawDeserialize(m_Buffer, offset, type);
                }

                BspFile.Faces asset = new BspFile.Faces();
                asset.StreamOffset = startOffset;
                asset.Header = header;

                if (asset.construct(binaryFaces))
                    return asset;
            }

            return null;
        }

        public int GetVertexCount(BspFile.Header header)
        {
            Type type = typeof(BspFile.Vertices.Binary_vertex);
            int size = Marshal.SizeOf(type);
            int length = header.m_DirEntries[(int)(BspFile.Header.EntityType.Vertices)].length;
            int count = (length / size);

            Trace.Assert(count * size == length);

            return count;
        }

        public BspFile.Vertices GetVertices(BspFile.Header header, int startIndex, int count)
        {
            if (header == null)
                return null;

            Type type = typeof(BspFile.Vertices.Binary_vertex);
            int size = Marshal.SizeOf(type);

            long baseOffset = header.StreamOffset + header.m_DirEntries[(int)(BspFile.Header.EntityType.Vertices)].offset;
            long startOffset = baseOffset + startIndex * size;
            long endOffset = startOffset + count * size;
            int totalSize = (int)(endOffset - startOffset);

            if (m_Buffer.Length < totalSize)
            {
                m_Buffer = new byte[totalSize];
            }

            BspFile.Vertices.Binary_vertex[] binaryVertices = new BspFile.Vertices.Binary_vertex[count];

            m_Stream.Seek(startOffset, SeekOrigin.Begin);
            if (m_Stream.Read(m_Buffer, 0, totalSize) == totalSize)
            {
                int offset = 0;
                for (int i = 0; i < count; ++i, offset += size)
                {
                    binaryVertices[i] = (BspFile.Vertices.Binary_vertex)BinarySerializer.RawDeserialize(m_Buffer, offset, type);
                }

                BspFile.Vertices asset = new BspFile.Vertices();
                asset.StreamOffset = startOffset;
                asset.Header = header;

                if (asset.construct(binaryVertices))
                    return asset;
            }

            return null;
        }

        public int GetMeshVertexCount(BspFile.Header header)
        {
            Type type = typeof(BspFile.MeshVertices.Binary_meshvert);
            int size = Marshal.SizeOf(type);
            int length = header.m_DirEntries[(int)(BspFile.Header.EntityType.Meshverts)].length;
            int count = (length / size);

            Trace.Assert(count * size == length);

            return count;
        }

        public BspFile.MeshVertices GetMeshVertices(BspFile.Header header, int startIndex, int count)
        {
            if (header == null)
                return null;

            Type type = typeof(BspFile.MeshVertices.Binary_meshvert);
            int size = Marshal.SizeOf(type);

            long baseOffset = header.StreamOffset + header.m_DirEntries[(int)(BspFile.Header.EntityType.Meshverts)].offset;
            long startOffset = baseOffset + startIndex * size;
            long endOffset = startOffset + count * size;
            int totalSize = (int)(endOffset - startOffset);

            if (m_Buffer.Length < totalSize)
            {
                m_Buffer = new byte[totalSize];
            }

            BspFile.MeshVertices.Binary_meshvert[] binaryVertices = new BspFile.MeshVertices.Binary_meshvert[count];

            m_Stream.Seek(startOffset, SeekOrigin.Begin);
            if (m_Stream.Read(m_Buffer, 0, totalSize) == totalSize)
            {
                int offset = 0;
                for (int i = 0; i < count; ++i, offset += size)
                {
                    binaryVertices[i] = (BspFile.MeshVertices.Binary_meshvert)BinarySerializer.RawDeserialize(m_Buffer, offset, type);
                }

                BspFile.MeshVertices asset = new BspFile.MeshVertices();
                asset.StreamOffset = startOffset;
                asset.Header = header;

                if (asset.construct(binaryVertices))
                    return asset;
            }

            return null;
        }

        public int GetNodeCount(BspFile.Header header)
        {
            Type type = typeof(BspFile.Nodes.Binary_node);
            int size = Marshal.SizeOf(type);
            int length = header.m_DirEntries[(int)(BspFile.Header.EntityType.Nodes)].length;
            int count = (length / size);

            Trace.Assert(count * size == length);

            return count;
        }

        public BspFile.Nodes GetNodes(BspFile.Header header, int startIndex, int count)
        {
            if (header == null)
                return null;

            Type type = typeof(BspFile.Nodes.Binary_node);
            int size = Marshal.SizeOf(type);

            long baseOffset = header.StreamOffset + header.m_DirEntries[(int)(BspFile.Header.EntityType.Nodes)].offset;
            long startOffset = baseOffset + startIndex * size;
            long endOffset = startOffset + count * size;
            int totalSize = (int)(endOffset - startOffset);

            if (m_Buffer.Length < totalSize)
            {
                m_Buffer = new byte[totalSize];
            }

            BspFile.Nodes.Binary_node[] binaryNodes = new BspFile.Nodes.Binary_node[count];

            m_Stream.Seek(startOffset, SeekOrigin.Begin);
            if (m_Stream.Read(m_Buffer, 0, totalSize) == totalSize)
            {
                int offset = 0;
                for (int i = 0; i < count; ++i, offset += size)
                {
                    binaryNodes[i] = (BspFile.Nodes.Binary_node)BinarySerializer.RawDeserialize(m_Buffer, offset, type);
                }

                BspFile.Nodes asset = new BspFile.Nodes();
                asset.StreamOffset = startOffset;
                asset.Header = header;

                if (asset.construct(binaryNodes))
                    return asset;
            }

            return null;
        }

        public int GetLeafCount(BspFile.Header header)
        {
            Type type = typeof(BspFile.Leafs.Binary_leaf);
            int size = Marshal.SizeOf(type);
            int length = header.m_DirEntries[(int)(BspFile.Header.EntityType.Leafs)].length;
            int count = (length / size);

            Trace.Assert(count * size == length);

            return count;
        }

        public BspFile.Leafs GetLeafs(BspFile.Header header, int startIndex, int count)
        {
            if (header == null)
                return null;

            Type type = typeof(BspFile.Leafs.Binary_leaf);
            int size = Marshal.SizeOf(type);

            long baseOffset = header.StreamOffset + header.m_DirEntries[(int)(BspFile.Header.EntityType.Leafs)].offset;
            long startOffset = baseOffset + startIndex * size;
            long endOffset = startOffset + count * size;
            int totalSize = (int)(endOffset - startOffset);

            if (m_Buffer.Length < totalSize)
            {
                m_Buffer = new byte[totalSize];
            }

            BspFile.Leafs.Binary_leaf[] binaryLeafs = new BspFile.Leafs.Binary_leaf[count];

            m_Stream.Seek(startOffset, SeekOrigin.Begin);
            if (m_Stream.Read(m_Buffer, 0, totalSize) == totalSize)
            {
                int offset = 0;
                for (int i = 0; i < count; ++i, offset += size)
                {
                    binaryLeafs[i] = (BspFile.Leafs.Binary_leaf)BinarySerializer.RawDeserialize(m_Buffer, offset, type);
                }

                BspFile.Leafs asset = new BspFile.Leafs();
                asset.StreamOffset = startOffset;
                asset.Header = header;

                if (asset.construct(binaryLeafs))
                    return asset;
            }

            return null;
        }

        public int GetLeafFaceCount(BspFile.Header header)
        {
            Type type = typeof(BspFile.LeafFaces.Binary_leafface);
            int size = Marshal.SizeOf(type);
            int length = header.m_DirEntries[(int)(BspFile.Header.EntityType.Leaffaces)].length;
            int count = (length / size);

            Trace.Assert(count * size == length);

            return count;
        }

        public BspFile.LeafFaces GetLeafFaces(BspFile.Header header, int startIndex, int count)
        {
            if (header == null)
                return null;

            Type type = typeof(BspFile.LeafFaces.Binary_leafface);
            int size = Marshal.SizeOf(type);

            long baseOffset = header.StreamOffset + header.m_DirEntries[(int)(BspFile.Header.EntityType.Leaffaces)].offset;
            long startOffset = baseOffset + startIndex * size;
            long endOffset = startOffset + count * size;
            int totalSize = (int)(endOffset - startOffset);

            if (m_Buffer.Length < totalSize)
            {
                m_Buffer = new byte[totalSize];
            }

            BspFile.LeafFaces.Binary_leafface[] binaryLeafFaces = new BspFile.LeafFaces.Binary_leafface[count];

            m_Stream.Seek(startOffset, SeekOrigin.Begin);
            if (m_Stream.Read(m_Buffer, 0, totalSize) == totalSize)
            {
                int offset = 0;
                for (int i = 0; i < count; ++i, offset += size)
                {
                    binaryLeafFaces[i] = (BspFile.LeafFaces.Binary_leafface)BinarySerializer.RawDeserialize(m_Buffer, offset, type);
                }

                BspFile.LeafFaces asset = new BspFile.LeafFaces();
                asset.StreamOffset = startOffset;
                asset.Header = header;

                if (asset.construct(binaryLeafFaces))
                    return asset;
            }

            return null;
        }

        public int GetPlaneCount(BspFile.Header header)
        {
            Type type = typeof(BspFile.Planes.Binary_plane);
            int size = Marshal.SizeOf(type);
            int length = header.m_DirEntries[(int)(BspFile.Header.EntityType.Planes)].length;
            int count = (length / size);

            Trace.Assert(count * size == length);

            return count;
        }

        public BspFile.Planes GetPlanes(BspFile.Header header, int startIndex, int count)
        {
            if (header == null)
                return null;

            Type type = typeof(BspFile.Planes.Binary_plane);
            int size = Marshal.SizeOf(type);

            long baseOffset = header.StreamOffset + header.m_DirEntries[(int)(BspFile.Header.EntityType.Planes)].offset;
            long startOffset = baseOffset + startIndex * size;
            long endOffset = startOffset + count * size;
            int totalSize = (int)(endOffset - startOffset);

            if (m_Buffer.Length < totalSize)
            {
                m_Buffer = new byte[totalSize];
            }

            BspFile.Planes.Binary_plane[] binaryPlanes = new BspFile.Planes.Binary_plane[count];

            m_Stream.Seek(startOffset, SeekOrigin.Begin);
            if (m_Stream.Read(m_Buffer, 0, totalSize) == totalSize)
            {
                int offset = 0;
                for (int i = 0; i < count; ++i, offset += size)
                {
                    binaryPlanes[i] = (BspFile.Planes.Binary_plane)BinarySerializer.RawDeserialize(m_Buffer, offset, type);
                }

                BspFile.Planes asset = new BspFile.Planes();
                asset.StreamOffset = startOffset;
                asset.Header = header;

                if (asset.construct(binaryPlanes))
                    return asset;
            }

            return null;
        }

        public bool HasVisdata(BspFile.Header header)
        {
            if (header == null)
                return false;

            if (header.m_DirEntries[(int)(BspFile.Header.EntityType.Visdata)].length == 0)
                return false;

            Type infoType = typeof(BspFile.Visdata.Binary_info);
            int infoSize = Marshal.SizeOf(infoType);

            Type type = typeof(BspFile.Visdata.Binary_data);
            int size = Marshal.SizeOf(type);

            long baseOffset = header.StreamOffset + header.m_DirEntries[(int)(BspFile.Header.EntityType.Visdata)].offset;
            long dataSize = header.StreamOffset + header.m_DirEntries[(int)(BspFile.Header.EntityType.Visdata)].length;
            long startOffset = baseOffset + infoSize;
            long endOffset = startOffset + (dataSize - infoSize);
            int totalSize = (int)(endOffset - startOffset);
            int count = totalSize / size;

            return count > 0;
        }

        public BspFile.Visdata GetVisdata(BspFile.Header header)
        {
            if (header == null)
                return null;

            if (header.m_DirEntries[(int)(BspFile.Header.EntityType.Visdata)].length == 0)
                return null;

            Type infoType = typeof(BspFile.Visdata.Binary_info);
            int infoSize = Marshal.SizeOf(infoType);

            Type type = typeof(BspFile.Visdata.Binary_data);
            int size = Marshal.SizeOf(type);

            long baseOffset = header.StreamOffset + header.m_DirEntries[(int)(BspFile.Header.EntityType.Visdata)].offset;
            long dataSize = header.StreamOffset + header.m_DirEntries[(int)(BspFile.Header.EntityType.Visdata)].length;
            long startOffset = baseOffset + infoSize;
            long endOffset = startOffset + (dataSize - infoSize);
            int totalSize = (int)(endOffset - startOffset);
            int count = totalSize / size;

            if (count == 0)
                return null;

            Trace.Assert(count * size == totalSize);

            BspFile.Visdata.Binary_info binaryInfo = new BspFile.Visdata.Binary_info();

            {
                if (m_Buffer.Length < infoSize)
                {
                    m_Buffer = new byte[infoSize];
                }

                m_Stream.Seek(baseOffset, SeekOrigin.Begin);
                if (m_Stream.Read(m_Buffer, 0, infoSize) == infoSize)
                {
                    binaryInfo = (BspFile.Visdata.Binary_info)BinarySerializer.RawDeserialize(m_Buffer, 0, infoType);

                    Trace.Assert(binaryInfo.n_vecs * binaryInfo.sz_vecs == count);
                }
                else
                {
                    return null;
                }
            }

            if (m_Buffer.Length < totalSize)
            {
                m_Buffer = new byte[totalSize];
            }

            BspFile.Visdata.Binary_data[] binaryVisdata = new BspFile.Visdata.Binary_data[count];

            m_Stream.Seek(startOffset, SeekOrigin.Begin);
            if (m_Stream.Read(m_Buffer, 0, totalSize) == totalSize)
            {
                int offset = 0;
                for (int i = 0; i < count; ++i, offset += size)
                {
                    binaryVisdata[i] = (BspFile.Visdata.Binary_data)BinarySerializer.RawDeserialize(m_Buffer, offset, type);
                }

                BspFile.Visdata asset = new BspFile.Visdata();
                asset.StreamOffset = startOffset;
                asset.Header = header;

                if (asset.construct(binaryInfo, binaryVisdata))
                    return asset;
            }

            return null;
        }

        public int GetTextureCount(BspFile.Header header)
        {
            Type type = typeof(BspFile.Textures.Binary_texture);
            int size = Marshal.SizeOf(type);
            int length = header.m_DirEntries[(int)(BspFile.Header.EntityType.Textures)].length;
            int count = (length / size);

            Trace.Assert(count * size == length);

            return count;
        }

        public BspFile.Textures GetTextures(BspFile.Header header, int startIndex, int count)
        {
            if (header == null)
                return null;

            Type type = typeof(BspFile.Textures.Binary_texture);
            int size = Marshal.SizeOf(type);

            long baseOffset = header.StreamOffset + header.m_DirEntries[(int)(BspFile.Header.EntityType.Textures)].offset;
            long startOffset = baseOffset + startIndex * size;
            long endOffset = startOffset + count * size;
            int totalSize = (int)(endOffset - startOffset);

            if (m_Buffer.Length < totalSize)
            {
                m_Buffer = new byte[totalSize];
            }

            BspFile.Textures.Binary_texture[] binaryTextures = new BspFile.Textures.Binary_texture[count];

            m_Stream.Seek(startOffset, SeekOrigin.Begin);
            if (m_Stream.Read(m_Buffer, 0, totalSize) == totalSize)
            {
                int offset = 0;
                for (int i = 0; i < count; ++i, offset += size)
                {
                    binaryTextures[i] = (BspFile.Textures.Binary_texture)BinarySerializer.RawDeserialize(m_Buffer, offset, type);
                }

                BspFile.Textures asset = new BspFile.Textures();
                asset.StreamOffset = startOffset;
                asset.Header = header;

                if (asset.construct(binaryTextures))
                    return asset;
            }

            return null;
        }

        static int BinLightmapSizeBytes = 128 * 128 * 3;

        public int GetLightmapCount(BspFile.Header header)
        {
            int size = BinLightmapSizeBytes;
            int length = header.m_DirEntries[(int)(BspFile.Header.EntityType.Lightmaps)].length;
            int count = (length / size);

            Trace.Assert(count * size == length);

            return count;
        }

        public BspFile.Lightmaps GetLightmaps(BspFile.Header header, int startIndex, int count)
        {
            if (header == null)
                return null;

            int size = BinLightmapSizeBytes;

            long baseOffset = header.StreamOffset + header.m_DirEntries[(int)(BspFile.Header.EntityType.Lightmaps)].offset;
            long startOffset = baseOffset + startIndex * size;
            long endOffset = startOffset + count * size;
            int totalSize = (int)(endOffset - startOffset);

            if (m_Buffer.Length < totalSize)
            {
                m_Buffer = new byte[totalSize];
            }

            BspFile.Lightmaps.Binary_lightmap[] binaryLightmaps = new BspFile.Lightmaps.Binary_lightmap[count];

            m_Stream.Seek(startOffset, SeekOrigin.Begin);
            if (m_Stream.Read(m_Buffer, 0, totalSize) == totalSize)
            {
                int offset = 0;
                for (int i = 0; i < count; ++i, offset += size)
                {
                    binaryLightmaps[i] = new BspFile.Lightmaps.Binary_lightmap();
                    binaryLightmaps[i].pixels = new Microsoft.Xna.Framework.Graphics.Color[128 * 128];
                    
                    int bufferIndex = offset;
                    int pixelIndex = 0;
                    for (int x = 0; x < 128; ++x)
                    {
                        for (int y = 0; y < 128; ++y)
                        {
                            binaryLightmaps[i].pixels[pixelIndex].B = m_Buffer[bufferIndex++];
                            binaryLightmaps[i].pixels[pixelIndex].G = m_Buffer[bufferIndex++];
                            binaryLightmaps[i].pixels[pixelIndex].R = m_Buffer[bufferIndex++];
                            binaryLightmaps[i].pixels[pixelIndex].A = 255;
                            ++pixelIndex;
                        }
                    }
                }

                BspFile.Lightmaps asset = new BspFile.Lightmaps();
                asset.StreamOffset = startOffset;
                asset.Header = header;

                if (asset.construct(binaryLightmaps))
                    return asset;
            }

            return null;
        }


        public int GetEffectCount(BspFile.Header header)
        {
            Type type = typeof(BspFile.Effects.Binary_effect);
            int size = Marshal.SizeOf(type);
            int length = header.m_DirEntries[(int)(BspFile.Header.EntityType.Effects)].length;
            int count = (length / size);

            Trace.Assert(count * size == length);

            return count;
        }

        public BspFile.Effects GetEffects(BspFile.Header header, int startIndex, int count)
        {
            if (header == null)
                return null;

            Type type = typeof(BspFile.Effects.Binary_effect);
            int size = Marshal.SizeOf(type);

            long baseOffset = header.StreamOffset + header.m_DirEntries[(int)(BspFile.Header.EntityType.Effects)].offset;
            long startOffset = baseOffset + startIndex * size;
            long endOffset = startOffset + count * size;
            int totalSize = (int)(endOffset - startOffset);

            if (m_Buffer.Length < totalSize)
            {
                m_Buffer = new byte[totalSize];
            }

            BspFile.Effects.Binary_effect[] binaryEffects = new BspFile.Effects.Binary_effect[count];

            m_Stream.Seek(startOffset, SeekOrigin.Begin);
            if (m_Stream.Read(m_Buffer, 0, totalSize) == totalSize)
            {
                int offset = 0;
                for (int i = 0; i < count; ++i, offset += size)
                {
                    binaryEffects[i] = (BspFile.Effects.Binary_effect)BinarySerializer.RawDeserialize(m_Buffer, offset, type);
                }

                BspFile.Effects asset = new BspFile.Effects();
                asset.StreamOffset = startOffset;
                asset.Header = header;

                if (asset.construct(binaryEffects))
                    return asset;
            }

            return null;
        }
    }

    class BinarySerializer
    {
        public static byte[] RawSerialize(object anything)
        {
            int rawsize = Marshal.SizeOf(anything);
            IntPtr buffer = Marshal.AllocHGlobal(rawsize);
            Marshal.StructureToPtr(anything, buffer, false);
            byte[] rawdatas = new byte[rawsize];
            Marshal.Copy(buffer, rawdatas, 0, rawsize);
            Marshal.FreeHGlobal(buffer);
            return rawdatas;

        }


        public static object RawDeserialize(byte[] rawdatas, int offset, Type anytype)
        {
            int rawsize = Marshal.SizeOf(anytype);
            if (rawsize > rawdatas.Length)
                return null;
            IntPtr buffer = Marshal.AllocHGlobal(rawsize);
            Marshal.Copy(rawdatas, offset, buffer, rawsize);
            object retobj = Marshal.PtrToStructure(buffer, anytype);
            Marshal.FreeHGlobal(buffer);
            return retobj;

        }
    }
}